/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/hal/system_timer/system_timer.h"

#include "led.h"
#include "pcf8523t.h"
#include "tmp1075n.h"
#include "rmii.h"

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	const uint8_t length = ((TFPMessageHeader*)message)->length;
	switch(tfp_get_fid_from_message(message)) {
		case FID_SET_LED:            return length != sizeof(SetLED)           ? HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER : set_led(message);
		case FID_GET_LED:            return length != sizeof(GetLED)           ? HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER : get_led(message, response);
		case FID_GET_TEMPERATURE:    return length != sizeof(GetTemperature)   ? HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER : get_temperature(message, response);
		case FID_SET_DATE_TIME:      return length != sizeof(SetDateTime)      ? HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER : set_date_time(message);
		case FID_GET_DATE_TIME:      return length != sizeof(GetDateTime)      ? HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER : get_date_time(message, response);
		case FID_GET_SD_INFORMATION: return length != sizeof(GetSDInformation) ? HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER : get_sd_information(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}


BootloaderHandleMessageResponse set_led(const SetLED *data) {
	if(data->state > WARP_ESP32_ETHERNET_V2_CO_LED_STATE_AUTO) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	led.state = data->state;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_led(const GetLED *data, GetLED_Response *response) {
	response->header.length = sizeof(GetLED_Response);
	response->state         = led.state;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_temperature(const GetTemperature *data, GetTemperature_Response *response) {
	response->header.length = sizeof(GetTemperature_Response);
	response->temperature   = tmp1075n.temperature;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_date_time(const SetDateTime *data) {
	pcf8523t.set              = true;
	pcf8523t.set_seconds      = data->seconds;
	pcf8523t.set_minutes      = data->minutes;
	pcf8523t.set_hours        = data->hours;
	pcf8523t.set_days         = data->days;
	pcf8523t.set_days_of_week = data->days_of_week;
	pcf8523t.set_month        = data->month;
	pcf8523t.set_year         = data->year;

	pcf8523t.seconds          = data->seconds;
	pcf8523t.minutes          = data->minutes;
	pcf8523t.hours            = data->hours;
	pcf8523t.days             = data->days;
	pcf8523t.days_of_week     = data->days_of_week;
	pcf8523t.month            = data->month;
	pcf8523t.year             = data->year;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_date_time(const GetDateTime *data, GetDateTime_Response *response) {
	response->header.length = sizeof(GetDateTime_Response);
	response->seconds       = pcf8523t.seconds;
	response->minutes       = pcf8523t.minutes;
	response->hours         = pcf8523t.hours;
	response->days          = pcf8523t.days;
	response->days_of_week  = pcf8523t.days_of_week;
	response->month         = pcf8523t.month;
	response->year          = pcf8523t.year;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_sd_information(const GetSDInformation *data, GetSDInformation_Response *response) {
	response->header.length = sizeof(GetSDInformation_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}




bool handle_rmmi_interrupt_callback(void) {
	static bool is_buffered = false;
	static RMMIInterrupt_Callback cb;

	if(!is_buffered) {
		if(!rmii.is_high) {
			// No RMMI interrupt, nothing to do
			return false;
		}

		if((rmii.last_low_high_edge == 0) || (system_timer_is_time_elapsed_ms(rmii.last_low_high_edge, 250))) {
			tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(RMMIInterrupt_Callback), FID_CALLBACK_RMMI_INTERRUPT);
			rmii.last_low_high_edge = system_timer_get_ms();
		} else {
			return false;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(RMMIInterrupt_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	communication_callback_init();
}
