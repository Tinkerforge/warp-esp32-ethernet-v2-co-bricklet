/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pcf85263.h: Driver for PCF85263
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

#ifndef PCF85263_H
#define PCF85263_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	PCF85263_STATE_INIT_OSCILLATOR = 0, // Configure oscillator load capacitance / 24h mode
	PCF85263_STATE_INIT_FUNCTION,       // Configure CLK output / 100th second / RTC mode
	PCF85263_STATE_IDLE,                // Nothing to do, periodically read the time
	PCF85263_STATE_SET_STOP,            // Stop clock and clear prescaler before setting the time
	PCF85263_STATE_SET_TIME,            // Write the new date/time
	PCF85263_STATE_SET_START,           // Start clock again
	PCF85263_STATE_GET,                 // Read the current date/time
} PCF85263State;

typedef struct {
	PCF85263State state;
	uint32_t last_get;

	bool set;
	uint8_t set_seconds;
	uint8_t set_minutes;
	uint8_t set_hours;
	uint8_t set_days;
	uint8_t set_days_of_week;
	uint8_t set_month;
	uint16_t set_year;

	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t days;
	uint8_t days_of_week;
	uint8_t month;
	uint16_t year;
} PCF85263;

extern PCF85263 pcf85263;

void pcf85263_tick(void);
void pcf85263_init(void);

#endif
