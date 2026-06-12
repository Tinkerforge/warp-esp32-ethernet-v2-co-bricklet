/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * config_i2c.h: Config for shared I2C bus (TMP1075N + PCF85263)
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

#ifndef CONFIG_I2C_H
#define CONFIG_I2C_H

#include "xmc_gpio.h"
#include "xmc_i2c.h"

// The TMP1075N temperature sensor and the PCF85263 RTC share this I2C bus.

#define I2C_BAUDRATE         100000

#define I2C_I2C              XMC_I2C0_CH1

#define I2C_SCL_PORT         XMC_GPIO_PORT2
#define I2C_SCL_PIN          11
#define I2C_SCL_PIN_MODE     XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT6
#define I2C_SCL_INPUT        XMC_USIC_CH_INPUT_DX1
#define I2C_SCL_SOURCE       4 // DX1E
#define I2C_SCL_FIFO_SIZE    XMC_USIC_CH_FIFO_SIZE_16WORDS
#define I2C_SCL_FIFO_POINTER 32

#define I2C_SDA_PORT         XMC_GPIO_PORT2
#define I2C_SDA_PIN          10
#define I2C_SDA_PIN_MODE     XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT7
#define I2C_SDA_INPUT        XMC_USIC_CH_INPUT_DX0
#define I2C_SDA_SOURCE       5 // DX0F
#define I2C_SDA_FIFO_SIZE    XMC_USIC_CH_FIFO_SIZE_16WORDS
#define I2C_SDA_FIFO_POINTER 48

#endif
