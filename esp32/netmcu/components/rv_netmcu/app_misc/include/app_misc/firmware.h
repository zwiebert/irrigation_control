#pragma once
#include <app_config/proj_app_cfg.h>
#ifdef USE_OTA
#include "firmware_update/ota.h"

#ifdef CONFIG_APP_USE_LAN
#define OTA_FWURL_MASTER "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/master/firmware/esp32/tronferno-mcu-lan.bin"
#define OTA_FWURL_BETA "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/beta/firmware/esp32/tronferno-mcu-lan.bin"
#else
#define OTA_FWURL_MASTER "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/master/firmware/esp32/tronferno-mcu.bin"
#define OTA_FWURL_BETA "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/beta/firmware/esp32/tronferno-mcu.bin"
#endif

#define OTA_FWURL_TAG_HEAD "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/"
#define OTA_FWURL_TAG_TAIL "/firmware/esp32/tronferno-mcu.bin"
#define OTA_FWURL_TAG_COOKIE "tag:"

inline bool app_doFirmwareUpdate(const char *firmwareURL) { return ota_doUpdate(firmwareURL); }

#endif

void mcu_delay_us(u16 us);
void mcu_restart(void);



