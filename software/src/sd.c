/* warp-energy-manager-bricklet
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
 *
 * sd.c: SD card handling
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

#include "sd.h"

#include <stdlib.h>

#include "bricklib2/logging/logging.h"
#include "bricklib2/utility/util_definitions.h"
#include "bricklib2/os/coop_task.h"

#include "configs/config_sdmmc.h"
#include "configs/config.h"
#include "sdmmc.h"

#include "xmc_rtc.h"
#include "xmc_wdt.h"
#include "lfs.h"
#include "sd_new_file_objects.h"
#include "data_storage.h"

SD sd;
CoopTask sd_task;

bool sd_lfs_format = false;

void sd_init_task(void) {
	memset(&sd, 0, sizeof(SD));

	// Status 0xFFFFFFFF = not yet initialized
	sd.sd_status  = 0xFFFFFFFF;
	sd.lfs_status = 0xFFFFFFFF;

	// lfs functions
	sd.lfs_config.read  = sd_lfs_read;
	sd.lfs_config.prog  = sd_lfs_prog;
	sd.lfs_config.erase = sd_lfs_erase;
	sd.lfs_config.sync  = sd_lfs_sync;

	// lfs block device configuration
	sd.lfs_config.read_size      = 512;
	sd.lfs_config.prog_size      = 512;
	sd.lfs_config.block_size     = 512;
	sd.lfs_config.block_count    = 15333376; // sector count for 8gb sd card
	sd.lfs_config.cache_size     = 512;
	sd.lfs_config.lookahead_size = 512;
	sd.lfs_config.block_cycles   = -1; // no wear-leveling (done by sd card itself)

	// lfs buffer
	sd.lfs_config.read_buffer      = sd.lfs_read_buffer;
	sd.lfs_config.prog_buffer      = sd.lfs_prog_buffer;
	sd.lfs_config.lookahead_buffer = sd.lfs_lookahead_buffer;

	// lfs disk version
#ifdef IS_ENERGY_MANAGER_V1
	sd.lfs_config.disk_version = 0x00020000;
#endif

	// lfs file config
	sd.lfs_file_config.buffer     = sd.lfs_file_buffer;
	sd.lfs_file_config.attr_count = 0;

	SDMMCError sdmmc_error = sdmmc_init();
	if(sdmmc_error != SDMMC_ERROR_OK) {
		sd.sd_status = sdmmc_error;
		logd("sdmmc_init: %d\n\r", sdmmc_error);
		sd.sdmmc_init_last = system_timer_get_ms();
		return;
	}
	sd.sdmmc_init_last = 0;

	sd.sector_size     = 512;
	sd.sector_count    = (sdmmc.csd_v2.dev_size_high << 16) | ((sdmmc.csd_v2.dev_size_low + 1) << 10);
	sd.product_rev     = sdmmc.cid.product_rev;
	sd.manufacturer_id = sdmmc.cid.manufacturer_id;
	sd.card_type       = sdmmc.type;
	memcpy(sd.product_name, sdmmc.cid.product_name, 5);

	// Overwrite block count
	sd.lfs_config.block_count = sd.sector_count;

	int err = 0;
	if(sd_lfs_format) {
		logd("Starting lfs format...\n\r");
		coop_task_yield();
		err = lfs_format(&sd.lfs, &sd.lfs_config);
		sd_lfs_format = false;
		logd("... done\n\r", err);
		if(err != LFS_ERR_OK) {
			logw("lfs_format %d\n\r", err);
			sd.lfs_status = ABS(err);
			sd.sd_status = sdmmc_error;
			return;
		}
	}

	coop_task_yield();
	err = lfs_mount(&sd.lfs, &sd.lfs_config);
	sd.lfs_status = ABS(err);
	coop_task_yield();

	if(err != LFS_ERR_OK) {
		logw("lfs_mount %d\n\r", err);
		err = lfs_format(&sd.lfs, &sd.lfs_config);
		if(err != LFS_ERR_OK) {
			logw("lfs_format %d\n\r", err);
			sd.lfs_status = ABS(err);
			sd.sd_status = sdmmc_error;
			return;
		}
		err = lfs_mount(&sd.lfs, &sd.lfs_config);
		if(err != LFS_ERR_OK) {
			logw("lfs_mount 2nd try %d\n\r", err);
			sd.lfs_status = ABS(err);
			sd.sd_status = sdmmc_error;
			return;
		}
	}

	// Open/create boot_count file, increment boot count and write it back
	// This is a good initial sd card sanity check
	lfs_file_t file;

	// read boot count
	uint32_t boot_count = 0;
	err = lfs_file_opencfg(&sd.lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT, &sd.lfs_file_config);
	if(err != LFS_ERR_OK) {
		logd("boot_count lfs_file_opencfg %d\n\r", err);
	}

	lfs_ssize_t size = lfs_file_read(&sd.lfs, &file, &boot_count, sizeof(boot_count));
	if(size != sizeof(boot_count)) {
		logd("boot_count lfs_file_read size %d vs %d\n\r", size, sizeof(boot_count));
	}

 	// update boot count
	boot_count += 1;
	err = lfs_file_rewind(&sd.lfs, &file);
	if(err != LFS_ERR_OK) {
		logd("boot_count lfs_file_rewind %d\n\r", err);
	}

	size = lfs_file_write(&sd.lfs, &file, &boot_count, sizeof(boot_count));
	if(size != sizeof(boot_count)) {
		logd("boot_count lfs_file_write size %s vs %d\n\r", size, sizeof(boot_count));
	}

	err = lfs_file_close(&sd.lfs, &file);
	if(err != LFS_ERR_OK) {
		logd("boot_count lfs_file_close %d\n\r", err);
	}

	logd("boot_count: %d\n\r", boot_count);

	// Set sd status at the end, to make sure that everything is completely initialized
	// before any other code tries to access the sd card
	sd.sd_status = sdmmc_error;
}

void sd_tick_task(void) {
#ifdef IS_ENERGY_MANAGER_V2
	static bool was_detected = false;
#endif

	// Pre-initialize sd and lfs status.
	// If no sd card is inserted, the sd_init code is never called and the status would in this case never be set.
	sd.sd_status  = 0xFFFFFFFF;
	sd.lfs_status = 0xFFFFFFFF;

	sd.sdmmc_init_last = system_timer_get_ms();
	const XMC_GPIO_CONFIG_t input_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	XMC_GPIO_Init(SDMMC_CDS_PIN, &input_pin_config);

#ifdef IS_ENERGY_MANAGER_V2
	// In v2 start with SD card disabled and spi deinitialized (i.e. all pins input floating)
	sdmmc_spi_deinit();
	const XMC_GPIO_CONFIG_t config_high = {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};
	// Enable pin is active low
	XMC_GPIO_Init(SDMMC_ENABLE_PIN, &config_high);
#endif

	while(true) {
		// If the sd and lfs status are OK and no sd is detected,
		// we assume that the sd card was hot-removed

		// Always assume that a sd is inserted, the SDMMC_CDS_PIN can be unreliable and we don't
		// really need hotplug functionality for the WEM...
		const bool sd_detected = true; //!XMC_GPIO_GetInput(SDMMC_CDS_PIN);
		if((sd.sd_status == SDMMC_ERROR_OK) && (sd.lfs_status == LFS_ERR_OK) && !sd_detected) {
			sd.sd_rw_error_count = 1000;
			logd("SD card hot-removed? Force error_count=1000 to re-initialize\n\r");
		}

		if(sd_lfs_format) {
			sd.sd_rw_error_count = 1000;
			logd("SD format requested. Force error_count=1000 to re-initialize\n\r");
		}

		if(!sd_detected) {
			sd.sd_status = SDMMC_ERROR_NO_CARD;
#ifdef IS_ENERGY_MANAGER_V2
			if(was_detected) {
				was_detected = false;
				logd("SD card removed\n\r");
				sdmmc_spi_deinit();
				XMC_GPIO_SetOutputHigh(SDMMC_ENABLE_PIN);
			}
#endif
		} else {
#ifdef IS_ENERGY_MANAGER_V2
			if(!was_detected) {
				was_detected = true;
				logd("SD card inserted, wait for 3 seconds\n\r");
				XMC_GPIO_SetOutputLow(SDMMC_ENABLE_PIN);
				sd.sdmmc_init_last = system_timer_get_ms();
				coop_task_sleep_ms(3000);
			}
#endif
		}

		if(sd.sd_rw_error_count > 10) {
			logw("sd.sd_rw_error_count: %d, sd detected: %d, sd format request: %d\n\r", sd.sd_rw_error_count, sd_detected, sd_lfs_format);
			int err = lfs_unmount(&sd.lfs);
			if(err != LFS_ERR_OK) {
				logw("lfs_unmount failed: %d\n\r", err);
			}

			sdmmc_spi_deinit();

			sd.sd_rw_error_count = 0;
			sd.sdmmc_init_last   = system_timer_get_ms() - 1001;
			sd.sd_status         = SDMMC_ERROR_COUNT_TO_HIGH;
			sd.lfs_status        = SDMMC_ERROR_COUNT_TO_HIGH;
		}


		// We retry to initialize SD card once per second
		if((sd.sdmmc_init_last != 0) && system_timer_is_time_elapsed_ms(sd.sdmmc_init_last, 1000)) {
			if(sd_detected) {
				sd_init_task();
			} else {
				logd("No SD card detected\n\r");
				sd.sdmmc_init_last = system_timer_get_ms();
			}
		}

		coop_task_yield();
	}
}

void sd_init(void) {
	coop_task_init(&sd_task, sd_tick_task);

}

void sd_tick(void) {
	coop_task_tick(&sd_task);
}

int sd_lfs_erase(const struct lfs_config *c, lfs_block_t block) {
	return LFS_ERR_OK;
}

int sd_lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
	// Yield once per block read
	coop_task_yield();

	SDMMCError sdmmc_error = sdmmc_read_block(block, buffer);
	if(sdmmc_error != SDMMC_ERROR_OK) {
		logw("sdmmc_read_block error %d, block %d, off %d, size %d\n\r", sdmmc_error, block, off, size);
		return LFS_ERR_IO;
	}
	return LFS_ERR_OK;
}

int sd_lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
	// Yield once per block write
	coop_task_yield();

	SDMMCError sdmmc_error = sdmmc_write_block(block, buffer);
	if(sdmmc_error != SDMMC_ERROR_OK) {
		logw("sdmmc_write_block error %d, block %d, off %d, size %d\n\r", sdmmc_error, block, off, size);
		return LFS_ERR_IO;
	}
	return LFS_ERR_OK;
}

int sd_lfs_sync(const struct lfs_config *c) {
	return LFS_ERR_OK;
}
