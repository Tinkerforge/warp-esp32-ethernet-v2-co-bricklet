/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
 *
 * led.c: Driver for LED
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

#include "led.h"

#include "configs/config_led.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/logging/logging.h"

#include "communication.h"

LED led;

void led_init(void) {
	memset(&led, 0, sizeof(LED));
	led.state = WARP_ESP32_ETHERNET_V2_CO_LED_STATE_AUTO;

	const XMC_GPIO_CONFIG_t led_config_low = {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_LOW,
	};
	XMC_GPIO_Init(LED_PIN, &led_config_low);
}

void led_tick(void) {
	switch(led.state) {
		case WARP_ESP32_ETHERNET_V2_CO_LED_STATE_OFF: {
			XMC_GPIO_SetOutputLow(LED_PIN);
			break;
		}
		case WARP_ESP32_ETHERNET_V2_CO_LED_STATE_ON: {
			XMC_GPIO_SetOutputHigh(LED_PIN);
			break;
		}
		case WARP_ESP32_ETHERNET_V2_CO_LED_STATE_AUTO: {
			if((system_timer_get_ms() % 1000) < 500) {
				XMC_GPIO_SetOutputHigh(LED_PIN);
			} else {
				XMC_GPIO_SetOutputLow(LED_PIN);
			}
			break;
		}
	}
}
