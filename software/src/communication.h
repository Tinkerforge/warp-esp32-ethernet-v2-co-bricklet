/* warp-esp32-ethernet-v2-co-bricklet
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants

#define WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_OK 0
#define WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_SD_ERROR 1
#define WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_LFS_ERROR 2
#define WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_QUEUE_FULL 3
#define WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_DATE_OUT_OF_RANGE 4

#define WARP_ESP32_ETHERNET_V2_CO_FORMAT_STATUS_OK 0
#define WARP_ESP32_ETHERNET_V2_CO_FORMAT_STATUS_PASSWORD_ERROR 1
#define WARP_ESP32_ETHERNET_V2_CO_FORMAT_STATUS_FORMAT_ERROR 2

#define WARP_ESP32_ETHERNET_V2_CO_LED_STATE_OFF 0
#define WARP_ESP32_ETHERNET_V2_CO_LED_STATE_ON 1
#define WARP_ESP32_ETHERNET_V2_CO_LED_STATE_AUTO 2

#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_BOOTLOADER 0
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_FIRMWARE 1
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_OK 0
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_INVALID_MODE 1
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_NO_CHANGE 2
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_OFF 0
#define WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_ON 1
#define WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_SET_LED 1
#define FID_GET_LED 2
#define FID_GET_TEMPERATURE 3
#define FID_SET_DATE_TIME 4
#define FID_GET_DATE_TIME 5
#define FID_GET_UPTIME 6
#define FID_FORMAT_SD 7
#define FID_GET_SD_INFORMATION 8

#define FID_CALLBACK_RMMI_INTERRUPT 9

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
} __attribute__((__packed__)) SetLED;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetLED;

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
} __attribute__((__packed__)) GetLED_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetTemperature;

typedef struct {
	TFPMessageHeader header;
	int16_t temperature;
} __attribute__((__packed__)) GetTemperature_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t days;
	uint8_t days_of_week;
	uint8_t month;
	uint16_t year;
} __attribute__((__packed__)) SetDateTime;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetDateTime;

typedef struct {
	TFPMessageHeader header;
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t days;
	uint8_t days_of_week;
	uint8_t month;
	uint16_t year;
} __attribute__((__packed__)) GetDateTime_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetUptime;

typedef struct {
	TFPMessageHeader header;
	uint32_t uptime;
} __attribute__((__packed__)) GetUptime_Response;

typedef struct {
	TFPMessageHeader header;
	uint32_t password;
} __attribute__((__packed__)) FormatSD;

typedef struct {
	TFPMessageHeader header;
	uint8_t format_status;
} __attribute__((__packed__)) FormatSD_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetSDInformation;

typedef struct {
	TFPMessageHeader header;
	uint32_t sd_status;
	uint32_t lfs_status;
	uint16_t sector_size;
	uint32_t sector_count;
	uint32_t card_type;
	uint8_t product_rev;
	char product_name[5];
	uint8_t manufacturer_id;
} __attribute__((__packed__)) GetSDInformation_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) RMMIInterrupt_Callback;


// Function prototypes
BootloaderHandleMessageResponse set_led(const SetLED *data);
BootloaderHandleMessageResponse get_led(const GetLED *data, GetLED_Response *response);
BootloaderHandleMessageResponse get_temperature(const GetTemperature *data, GetTemperature_Response *response);
BootloaderHandleMessageResponse set_date_time(const SetDateTime *data);
BootloaderHandleMessageResponse get_date_time(const GetDateTime *data, GetDateTime_Response *response);
BootloaderHandleMessageResponse get_uptime(const GetUptime *data, GetUptime_Response *response);
BootloaderHandleMessageResponse format_sd(const FormatSD *data, FormatSD_Response *response);
BootloaderHandleMessageResponse get_sd_information(const GetSDInformation *data, GetSDInformation_Response *response);

// Callbacks
bool handle_rmmi_interrupt_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 1
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_rmmi_interrupt_callback, \


#endif
