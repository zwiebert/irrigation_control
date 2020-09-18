#pragma once
#include <app/config/proj_app_cfg.h>
#ifdef USE_OTA
#include "firmware_update/ota.h"

#ifdef USE_LAN
#define OTA_FWURL_MASTER "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/master/firmware/esp32/tronferno-mcu-lan.bin"
#define OTA_FWURL_BETA "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/beta/firmware/esp32/tronferno-mcu-lan.bin"
#else
#define OTA_FWURL_MASTER "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/master/firmware/esp32/tronferno-mcu.bin"
#define OTA_FWURL_BETA "https://raw.githubusercontent.com/zwiebert/tronferno-mcu-bin/beta/firmware/esp32/tronferno-mcu.bin"
#endif

extern const char ca_cert_pem[];
inline bool app_doFirmwareUpdate(const char *firmwareURL, const char *cert = ca_cert_pem) { return ota_doUpdate(firmwareURL, cert); }

#endif
