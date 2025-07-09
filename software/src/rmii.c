/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
 *
 * rmii.c: Driver for RMII
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

#include "rmii.h"

#include "configs/config_rmii.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/logging/logging.h"

#include "xmc_gpio.h"

RMII rmii;

void rmii_init(void) {
	memset(&rmii, 0, sizeof(RMII));
	const XMC_GPIO_CONFIG_t rmii_config_input = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_LARGE,
	};
	XMC_GPIO_Init(RMII_INT_PIN, &rmii_config_input);
}

void rmii_tick(void) {
	if(XMC_GPIO_GetInput(RMII_INT_PIN)) {
		if(!rmii.is_high) {
			rmii.last_low_high_edge = 0;
		}
		rmii.is_high = true;
	} else {
		rmii.last_low_high_edge = 0;
		rmii.is_high = false;
	}
}
