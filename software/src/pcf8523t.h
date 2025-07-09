/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pcf8523t.h: Driver for PCF8523T
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

#ifndef PCF8523T_H
#define PCF8523T_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
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
} PCF8523T;

extern PCF8523T pcf8523t;

void pcf8523t_tick(void);
void pcf8523t_init(void);

#endif