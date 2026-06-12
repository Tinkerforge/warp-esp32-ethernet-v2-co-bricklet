/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * i2c.c: Shared I2C bus (TMP1075N + PCF85263) with simple owner arbitration
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

#include "i2c.h"

#include "configs/config_i2c.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"

I2C i2c;

void i2c_init(void) {
	// Note: We intentionally do not memset the I2C struct here. i2c_init() is
	// also used as an error/timeout recovery, in which case i2c_fifo_init()
	// relies on i2c_fifo.i2c_status (e.g. I2C_FIFO_STATUS_TIMEOUT) to decide
	// whether it has to clear a stuck bus.
	i2c.owner = I2C_OWNER_NONE;

	i2c.i2c_fifo.baudrate         = I2C_BAUDRATE;
	i2c.i2c_fifo.address          = 0;
	i2c.i2c_fifo.i2c              = I2C_I2C;

	i2c.i2c_fifo.scl_port         = I2C_SCL_PORT;
	i2c.i2c_fifo.scl_pin          = I2C_SCL_PIN;
	i2c.i2c_fifo.scl_mode         = I2C_SCL_PIN_MODE;
	i2c.i2c_fifo.scl_input        = I2C_SCL_INPUT;
	i2c.i2c_fifo.scl_source       = I2C_SCL_SOURCE;
	i2c.i2c_fifo.scl_fifo_size    = I2C_SCL_FIFO_SIZE;
	i2c.i2c_fifo.scl_fifo_pointer = I2C_SCL_FIFO_POINTER;

	i2c.i2c_fifo.sda_port         = I2C_SDA_PORT;
	i2c.i2c_fifo.sda_pin          = I2C_SDA_PIN;
	i2c.i2c_fifo.sda_mode         = I2C_SDA_PIN_MODE;
	i2c.i2c_fifo.sda_input        = I2C_SDA_INPUT;
	i2c.i2c_fifo.sda_source       = I2C_SDA_SOURCE;
	i2c.i2c_fifo.sda_fifo_size    = I2C_SDA_FIFO_SIZE;
	i2c.i2c_fifo.sda_fifo_pointer = I2C_SDA_FIFO_POINTER;

	i2c_fifo_init(&i2c.i2c_fifo);
}

bool i2c_claim(const I2COwner owner, const uint8_t address) {
	// Bus is already taken by somebody else.
	if(i2c.owner != I2C_OWNER_NONE) {
		return false;
	}

	// Only hand out the bus if no transaction is in flight (i.e. the fifo is
	// idle or in a ready state). This mirrors the static i2c_fifo_ready_or_idle()
	// check inside i2c_fifo.c which we can not call from here.
	if(!((i2c.i2c_fifo.state == I2C_FIFO_STATE_IDLE) || (i2c.i2c_fifo.state & I2C_FIFO_STATE_READY))) {
		return false;
	}

	i2c.owner            = owner;
	i2c.i2c_fifo.address = address;

	return true;
}

void i2c_release(const I2COwner owner) {
	// Only the current owner is allowed to release the bus.
	if(i2c.owner == owner) {
		i2c.owner = I2C_OWNER_NONE;
	}
}
