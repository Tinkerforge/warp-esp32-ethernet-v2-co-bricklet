/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
 *
 * tmp1075n.c: Driver for TMP1075N temperature sensor
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

#include "tmp1075n.h"

#include "i2c.h"
#include "configs/config_tmp1075n.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/logging/logging.h"

TMP1075N tmp1075n;

void tmp1075n_init(void) {
	memset(&tmp1075n, 0, sizeof(TMP1075N));

	tmp1075n.last_read = system_timer_get_ms();
}

void tmp1075n_tick(void) {
	if((i2c.owner != I2C_OWNER_NONE) && (i2c.owner != I2C_OWNER_TMP1075N)) {
		return;
	}

	I2CFifoState state = i2c_fifo_next_state(&i2c.i2c_fifo);

	// A read did not finish within 60 seconds
	if(system_timer_is_time_elapsed_ms(tmp1075n.last_read, 60*1000)) {
		loge("TMP1075N I2C timeout: %d\n\r", state);
		i2c_init();
		tmp1075n.last_read = system_timer_get_ms();
		return;
	}

	if(state & I2C_FIFO_STATE_ERROR) {
		loge("TMP1075N I2C error: %d\n\r", state);
		i2c_init();
		tmp1075n.last_read = system_timer_get_ms();
		return;
	}

	switch(state) {
		case I2C_FIFO_STATE_READ_DIRECT_READY: {
			uint8_t buffer[16];
			uint8_t length = i2c_fifo_read_fifo(&i2c.i2c_fifo, buffer, 16);
			if(length != 2) {
				loge("TMP1075N I2C unexpected read length : %d\n\r", length);
				i2c_init();
				tmp1075n.last_read = system_timer_get_ms();
				break;
			}

			int16_t value = ((buffer[0] << 8) | buffer[1]) >> 4;
			// 12-bit to 16-bit two-complement
			if(value & (1 << 11)) {
				value = value | 0xF000;
			}
			tmp1075n.temperature = ((((int32_t)value) * 625) / 100);
			tmp1075n.last_read = system_timer_get_ms();

			// Temperature is read, set state back to idle and release the bus.
			// The next read will be started after the timeout that is handled below.
			i2c.i2c_fifo.state = I2C_FIFO_STATE_IDLE;
			i2c_release(I2C_OWNER_TMP1075N);
			break;
		}

		case I2C_FIFO_STATE_IDLE: {
			break; // Handled below
		}

		default: {
			// If we end up in a ready state that we don't handle, something went wrong
			if(state & I2C_FIFO_STATE_READY) {
				loge("TMP1075N I2C unrecognized ready state : %d\n\r", state);
				i2c_init();
				tmp1075n.last_read = system_timer_get_ms();
			}
			return;
		}
	}

	if((state == I2C_FIFO_STATE_IDLE) || (state & I2C_FIFO_STATE_READY)) {
		// Read temperature once per 500ms
		if(system_timer_is_time_elapsed_ms(tmp1075n.last_read, 500)) {
			if(i2c_claim(I2C_OWNER_TMP1075N, TMP1075N_I2C_ADDRESS)) {
				i2c_fifo_read_direct(&i2c.i2c_fifo, 2, false);
			}
		}
	}
}
