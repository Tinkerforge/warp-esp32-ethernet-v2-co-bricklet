/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pcf85263.c: Driver for PCF85263
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "pcf85263.h"

#include "i2c.h"
#include "configs/config_pcf85263.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/logging/logging.h"

// RTC time and date registers (read/written as one block of 8 bytes)
#define PCF85263_REG_RTC_TIME_100TH_SECOND      0x00
// Control and function registers
#define PCF85263_REG_OSCILLATOR                 0x25
#define PCF85263_REG_FUNCTION                   0x28
#define PCF85263_REG_STOP_ENABLE                0x2E
#define PCF85263_REG_RESET                      0x2F

// Oscillator register: 12.5pF load capacitance, 24h mode (1224 = 0)
#define PCF85263_REG_OSCILLATOR_CL_12PF         0b00000010
// Function register: CLK pin static low, 100th second disabled, RTC mode
#define PCF85263_REG_FUNCTION_COF_STATIC_LOW    0b00000111
// Stop enable register
#define PCF85263_REG_STOP_ENABLE_STOP_DISABLED  0b00000000
#define PCF85263_REG_STOP_ENABLE_STOP_ENABLED   0b00000001
// Reset register
#define PCF85263_REG_RESET_CLEAR_PRESCALER      0b10100100 // CPR

// Read the current date/time from the RTC at most this often (in ms)
#define PCF85263_GET_INTERVAL                   250

PCF85263 pcf85263;

static uint8_t bcd2bin(const uint8_t bcd) {
	return (bcd >> 4) * 10 + (bcd & 0x0F);
}

static uint8_t bin2bcd(const uint8_t bin) {
	return ((bin / 10) << 4) + (bin % 10);
}

// Reset the I2C bus and start over with the oscillator initialization.
static void pcf85263_reset(void) {
	i2c_init();
	pcf85263.state    = PCF85263_STATE_INIT_OSCILLATOR;
	pcf85263.last_get = system_timer_get_ms();
}

static void pcf85263_fail(void) {
	pcf85263.error_count++;
	if(pcf85263.error_count >= PCF85263_MAX_ERRORS) {
		loge("PCF85263 disabled after %d consecutive I2C errors\n\r", pcf85263.error_count);
		pcf85263.disabled  = true;
		// Hand the shared bus back so the TMP1075N can keep using it.
		i2c.i2c_fifo.state = I2C_FIFO_STATE_IDLE;
		i2c_release(I2C_OWNER_PCF85263);
	} else {
		pcf85263_reset();
	}
}

void pcf85263_init(void) {
	memset(&pcf85263, 0, sizeof(PCF85263));

	pcf85263.state    = PCF85263_STATE_INIT_OSCILLATOR;
	pcf85263.last_get = system_timer_get_ms();
}

void pcf85263_tick(void) {
	if(pcf85263.disabled) {
		return;
	}

	if((i2c.owner != I2C_OWNER_NONE) && (i2c.owner != I2C_OWNER_PCF85263)) {
		return;
	}

	I2CFifoState state = i2c_fifo_next_state(&i2c.i2c_fifo);

	// An operation did not finish within 60 seconds
	if(system_timer_is_time_elapsed_ms(pcf85263.last_get, 60*1000)) {
		loge("PCF85263 I2C timeout: %d\n\r", state);
		pcf85263_fail();
		return;
	}

	if(state & I2C_FIFO_STATE_ERROR) {
		loge("PCF85263 I2C error: %d\n\r", state);
		pcf85263_fail();
		return;
	}

	switch(state) {
		case I2C_FIFO_STATE_READ_REGISTER_READY: {
			uint8_t data[16];
			uint8_t length = i2c_fifo_read_fifo(&i2c.i2c_fifo, data, 16);
			if(length != 8) {
				loge("PCF85263 unexpected I2C read length: %d\n\r", length);
				pcf85263_fail();
				return;
			}

			pcf85263.error_count = 0;

			// Don't overwrite the visible date/time while a set is pending,
			// otherwise we would replace the just-set value with a stale read.
			if(!pcf85263.set) {
				pcf85263.seconds      = bcd2bin(data[1] & 0x7F);  // mask OS bit
				pcf85263.minutes      = bcd2bin(data[2] & 0x7F);  // mask EMON bit
				pcf85263.hours        = bcd2bin(data[3] & 0x3F);  // 24h mode
				pcf85263.days         = bcd2bin(data[4] & 0x3F) - 1;
				pcf85263.days_of_week = bcd2bin(data[5] & 0x07);
				pcf85263.month        = bcd2bin(data[6] & 0x1F) - 1;
				pcf85263.year         = bcd2bin(data[7]) + 100;
			}

			pcf85263.last_get  = system_timer_get_ms();
			pcf85263.state     = PCF85263_STATE_IDLE;
			i2c.i2c_fifo.state = I2C_FIFO_STATE_IDLE;
			i2c_release(I2C_OWNER_PCF85263);
			break;
		}

		case I2C_FIFO_STATE_WRITE_REGISTER_READY: {
			pcf85263.error_count = 0;

			switch(pcf85263.state) {
				case PCF85263_STATE_INIT_OSCILLATOR: {
					pcf85263.state = PCF85263_STATE_INIT_FUNCTION;
					break;
				}

				case PCF85263_STATE_INIT_FUNCTION: {
					pcf85263.state     = PCF85263_STATE_IDLE;
					i2c.i2c_fifo.state = I2C_FIFO_STATE_IDLE;
					i2c_release(I2C_OWNER_PCF85263);
					break;
				}

				case PCF85263_STATE_SET_STOP: {
					pcf85263.state = PCF85263_STATE_SET_TIME;
					break;
				}

				case PCF85263_STATE_SET_TIME: {
					pcf85263.state = PCF85263_STATE_SET_START;
					break;
				}

				case PCF85263_STATE_SET_START: {
					pcf85263.state     = PCF85263_STATE_IDLE;
					i2c.i2c_fifo.state = I2C_FIFO_STATE_IDLE;
					i2c_release(I2C_OWNER_PCF85263);
					break;
				}

				default: {
					loge("PCF85263 unrecognized write state: %d\n\r", pcf85263.state);
					pcf85263_fail();
					return;
				}
			}

			break;
		}

		case I2C_FIFO_STATE_IDLE: {
			break; // Handled below
		}

		default: {
			// If we end up in a ready state that we don't handle, something went wrong
			if(state & I2C_FIFO_STATE_READY) {
				loge("PCF85263 unrecognized I2C ready state: %d\n\r", state);
				pcf85263_fail();
			}
			return;
		}
	}

	if((state == I2C_FIFO_STATE_IDLE) || (state & I2C_FIFO_STATE_READY)) {
		if(pcf85263.state == PCF85263_STATE_INIT_OSCILLATOR) {
			// Select 12.5pF oscillator load capacitance and 24h mode
			if(i2c_claim(I2C_OWNER_PCF85263, PCF85263_I2C_ADDRESS)) {
				uint8_t data = PCF85263_REG_OSCILLATOR_CL_12PF;
				i2c_fifo_write_register(&i2c.i2c_fifo, PCF85263_REG_OSCILLATOR, 1, &data, true);
			}
		} else if(pcf85263.state == PCF85263_STATE_INIT_FUNCTION) {
			// Disable CLK pin and 100th second, keep RTC mode (bus already owned)
			uint8_t data = PCF85263_REG_FUNCTION_COF_STATIC_LOW;
			i2c_fifo_write_register(&i2c.i2c_fifo, PCF85263_REG_FUNCTION, 1, &data, true);
		} else if((pcf85263.state == PCF85263_STATE_IDLE) && pcf85263.set) {
			// Stop clock and clear prescaler before writing the new time
			if(i2c_claim(I2C_OWNER_PCF85263, PCF85263_I2C_ADDRESS)) {
				// Consume the request now. If a new set_date_time arrives while
				// the sequence is running, set is raised again and we redo it.
				pcf85263.set = false;

				uint8_t data[2] = {
					PCF85263_REG_STOP_ENABLE_STOP_ENABLED,
					PCF85263_REG_RESET_CLEAR_PRESCALER
				};
				pcf85263.state = PCF85263_STATE_SET_STOP;
				i2c_fifo_write_register(&i2c.i2c_fifo, PCF85263_REG_STOP_ENABLE, 2, data, true);
			}
		} else if(pcf85263.state == PCF85263_STATE_SET_TIME) {
			// bus already owned
			uint8_t data[8] = {
				0,                                       // 100th second (disabled)
				bin2bcd(pcf85263.set_seconds),           // tm_sec  [0..59]
				bin2bcd(pcf85263.set_minutes),           // tm_min  [0..59]
				bin2bcd(pcf85263.set_hours),             // tm_hour [0..23], 24h mode
				bin2bcd(pcf85263.set_days + 1),          // tm_mday-1 -> [1..31]
				bin2bcd(pcf85263.set_days_of_week),      // tm_wday [Sun..Sat] == [0..6]
				bin2bcd(pcf85263.set_month + 1),         // tm_mon -> [1..12]
				bin2bcd(pcf85263.set_year - 100)         // tm_year (since 1900) -> [0..99]
			};
			i2c_fifo_write_register(&i2c.i2c_fifo, PCF85263_REG_RTC_TIME_100TH_SECOND, 8, data, true);
		} else if(pcf85263.state == PCF85263_STATE_SET_START) {
			// Start clock again (bus already owned)
			uint8_t data = PCF85263_REG_STOP_ENABLE_STOP_DISABLED;
			i2c_fifo_write_register(&i2c.i2c_fifo, PCF85263_REG_STOP_ENABLE, 1, &data, true);
		} else if(pcf85263.state == PCF85263_STATE_IDLE) {
			// Periodically read the current date/time
			if(system_timer_is_time_elapsed_ms(pcf85263.last_get, PCF85263_GET_INTERVAL)) {
				if(i2c_claim(I2C_OWNER_PCF85263, PCF85263_I2C_ADDRESS)) {
					pcf85263.state = PCF85263_STATE_GET;
					i2c_fifo_read_register(&i2c.i2c_fifo, PCF85263_REG_RTC_TIME_100TH_SECOND, 8);
				}
			}
		}
	}
}
