/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * i2c.h: Shared I2C bus (TMP1075N + PCF85263) with simple owner arbitration
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

#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"

typedef enum {
	I2C_OWNER_NONE = 0,
	I2C_OWNER_TMP1075N,
	I2C_OWNER_PCF85263,
} I2COwner;

typedef struct {
	I2CFifo i2c_fifo;
	I2COwner owner;
} I2C;

extern I2C i2c;

void i2c_init(void);
bool i2c_claim(const I2COwner owner, const uint8_t address);
void i2c_release(const I2COwner owner);

#endif
