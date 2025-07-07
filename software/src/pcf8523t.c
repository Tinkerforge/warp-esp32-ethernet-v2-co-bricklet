/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pcf8523t.c: Driver for PCF8523T
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

#include "pcf8523t.h"

#include "configs/config_pcf8523t.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/logging/logging.h"

PCF8523T pcf8523t;

void pcf8523t_init(void) {
	memset(&pcf8523t, 0, sizeof(PCF8523T));
}

void pcf8523t_tick(void) {

}
