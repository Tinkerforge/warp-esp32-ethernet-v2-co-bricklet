/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * config_pcf85263.h: Config for PCF85263 driver
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

#ifndef CONFIG_PCF85263_H
#define CONFIG_PCF85263_H

// The PCF85263 shares the I2C bus with the TMP1075N.

#define PCF85263_I2C_ADDRESS 0b1010001 // 0x51

#endif
