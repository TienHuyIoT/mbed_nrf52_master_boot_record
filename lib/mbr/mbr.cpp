/* Includes ------------------------------------------------------------------*/
#include "mbr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

MasterBootRecord::MasterBootRecord() : /* Initialization FlashWearLevellingUtils object */
                                       _flash_wear_levelling(MASTER_BOOT_PARAMS_ADDR, MASTER_BOOT_PARAMS_REGION_SIZE, DEVICE_PAGE_ERASE_SIZE, sizeof(mbr_info_t)),
                                       /* Initialization FlashIAPBlockDevice object */
                                       _flash_iap_block_device(MASTER_BOOT_PARAMS_ADDR, MASTER_BOOT_PARAMS_REGION_SIZE),
                                       /* Initialization flashInterface object with FlashIAPBlockDevice handler*/
                                       _flash_internal_handler(&_flash_iap_block_device, MASTER_BOOT_PARAMS_ADDR),
                                       /* Initialization flashIFCallback object with flashInterface handler */
                                       _fp_callback(&_flash_internal_handler)
{
    _init_isOK = false;
}

MasterBootRecord::~MasterBootRecord() {
    this->end();
}

void MasterBootRecord::end(void)
{
    if (_init_isOK)
    {
        _flash_internal_handler.end();
        _init_isOK = false;
    }
}

MasterBootRecord::mbr_status_t MasterBootRecord::begin(void)
{
    if (_init_isOK)
    {
        return MBR_OK;
    }
    
    /* Init flashIFCallback object with flashInterface handler */
    if (_flash_internal_handler.begin() != flash_status_t::FLASHIF_OK)
    {
        MBR_TAG_PRINTF("[_flash_internal_handler] begin failed!");
        return MBR_ERROR;
    }

    /* register flash handler callback */
    _flash_wear_levelling.setCallbacks(&_fp_callback);
    if (!_flash_wear_levelling.begin(true))
    {
        MBR_TAG_PRINTF("[_flash_wear_levelling] begin failed!");
        return MBR_ERROR;
    }

    this->load();

    _init_isOK = true;
    return MBR_OK;
}

MasterBootRecord::mbr_status_t MasterBootRecord::load(void)
{
    MBR_TAG_PRINTF("[_flash_wear_levelling] read");
    if (!_flash_wear_levelling.read(&_mbr_info))
    {
        return setDefault();
    }
    return MBR_OK;
}

MasterBootRecord::mbr_status_t MasterBootRecord::commit(void)
{
    if (!_flash_wear_levelling.write(&_mbr_info))
    {
        MBR_TAG_PRINTF("[commit] failed!");
        return MBR_ERROR;
    }
    return MBR_OK;
}

MasterBootRecord::mbr_status_t MasterBootRecord::setDefault(void)
{
    mbr_info_t mbr_default = MBR_INFO_DEFAULT;

    mbr_default.main_app.common.app_status = APP_STATUS_OK;
    mbr_default.main_rollback.common.app_status = APP_STATUS_NONE;

    mbr_default.boot_app.common.app_status = APP_STATUS_OK;
    mbr_default.boot_rollback.common.app_status = APP_STATUS_NONE;

    mbr_default.image_download.common.app_status = APP_STATUS_NONE;
    
    mbr_default.common.startup_mode = MBR_STARTUP_MODE;
    mbr_default.common.dfu_mode = MBR_DFU_MODE;
    
    MBR_TAG_PRINTF("[setDefault] Set");
    if (!_flash_wear_levelling.write(&mbr_default))
    {
        MBR_TAG_PRINTF("[setDefault] write failed!");
        return MBR_ERROR;
    }
    _mbr_info = mbr_default;
    return MBR_OK;
}

app_info_t MasterBootRecord::getMainParams(void)
{
    return _mbr_info.main_app;
}

app_info_t MasterBootRecord::getBootParams(void)
{
    return _mbr_info.boot_app;
}

app_info_t MasterBootRecord::getMainRollbackParams(void)
{
    return _mbr_info.main_rollback;
}

app_info_t MasterBootRecord::getBootRollbackParams(void)
{
    return _mbr_info.boot_rollback;
}

app_info_t MasterBootRecord::getImageDownloadParams(void)
{
    return _mbr_info.image_download;
}

AES128_crypto_t MasterBootRecord::getAes128Params(void)
{
    return _mbr_info.aes;
}

MasterBootRecord::dfu_mode_t MasterBootRecord::getDfuMode(void)
{
    return (MasterBootRecord::dfu_mode_t)_mbr_info.common.dfu_mode;
}

MasterBootRecord::startup_mode_t MasterBootRecord::getStartUpMode(void)
{
    return (MasterBootRecord::startup_mode_t)_mbr_info.common.startup_mode;
}

MasterBootRecord::app_status_t MasterBootRecord::getMainStatus(void)
{
    return (MasterBootRecord::app_status_t)_mbr_info.main_app.common.app_status;
}

MasterBootRecord::app_status_t MasterBootRecord::getBootStatus(void)
{
    return (MasterBootRecord::app_status_t)_mbr_info.boot_app.common.app_status;
}

uint16_t MasterBootRecord::getMainDfuNum(void)
{
    return _mbr_info.dfu_num.main;
}

uint16_t MasterBootRecord::getBootDfuNum(void)
{
    return _mbr_info.dfu_num.boot;
}

std::string MasterBootRecord::getHardwareVersion(void)
{
    _mbr_info.hw_version_str[HARDWARE_VERSION_LENGTH_MAX - 1] = 0;
    return std::string((char*)_mbr_info.hw_version_str, HARDWARE_VERSION_LENGTH_MAX);
}

void MasterBootRecord::setMainParams(app_info_t* pParams)
{
    _mbr_info.main_app = *pParams;
}

void MasterBootRecord::setBootParams(app_info_t* pParams)
{
    _mbr_info.boot_app = *pParams;
}

void MasterBootRecord::setMainRollbackParams(app_info_t* pParams)
{
    _mbr_info.main_rollback = *pParams;
}

void MasterBootRecord::setBootRollbackParams(app_info_t* pParams)
{
    _mbr_info.boot_rollback = *pParams;
}

void MasterBootRecord::setImageDownloadParams(app_info_t* pParams)
{
    _mbr_info.image_download = *pParams;
}

void MasterBootRecord::setAes128Params(AES128_crypto_t* pParams)
{
    _mbr_info.aes = *pParams;
}

void MasterBootRecord::setDfuMode(dfu_mode_t mode)
{
    _mbr_info.common.dfu_mode = mode;
}

void MasterBootRecord::setStartUpMode(startup_mode_t mode)
{
    _mbr_info.common.startup_mode = mode;
}

void MasterBootRecord::setMainStatus(app_status_t status)
{
    _mbr_info.main_app.common.app_status = status;
}

void MasterBootRecord::setBootStatus(app_status_t status)
{
    _mbr_info.boot_app.common.app_status = status;
}

void MasterBootRecord::setHardwareVersion(std::string const &hwName)
{
    memcpy(_mbr_info.hw_version_str, hwName.data(), HARDWARE_VERSION_LENGTH_MAX);
    _mbr_info.hw_version_str[HARDWARE_VERSION_LENGTH_MAX - 1] = 0;
}

void MasterBootRecord::setMainDfuNum(uint16_t num)
{
    _mbr_info.dfu_num.main = num;
}

void MasterBootRecord::setBootDfuNum(uint16_t num)
{
    _mbr_info.dfu_num.boot = num;
}

void MasterBootRecord::printMbrInfo(void)
{
#if (1)
    if(load() == MBR_OK)
    {
        MBR_TAG_PRINTF("MBR information");
        MBR_TAG_PRINTF("main_app:");
        MBR_TAG_PRINTF("\t startup_addr: 0x%08x", _mbr_info.main_app.startup_addr);
        MBR_TAG_PRINTF("\t max_size: %s", readableSize(_mbr_info.main_app.max_size).c_str());
        MBR_TAG_PRINTF("\t checksum: 0x%08x", _mbr_info.main_app.fw_header.checksum);
        MBR_TAG_PRINTF("\t size: %u(%s)", _mbr_info.main_app.fw_header.size, 
                        readableSize(_mbr_info.main_app.fw_header.size).c_str());
        MBR_TAG_PRINTF("\t version: 0x%08X", _mbr_info.main_app.fw_header.version.u32);
        MBR_TAG_PRINTF("\t type: %s, %s, %s\n", 
                        _mbr_info.main_app.fw_header.type.mem ? "external" : "internal",
                        _mbr_info.main_app.fw_header.type.enc ? "encrypt" : "raw",
                        _mbr_info.main_app.fw_header.type.app ? "Main" : "Boot");

        MBR_TAG_PRINTF("boot_app:");
        MBR_TAG_PRINTF("\t startup_addr: 0x%08x", _mbr_info.boot_app.startup_addr);
        MBR_TAG_PRINTF("\t max_size: %s", readableSize(_mbr_info.boot_app.max_size).c_str());
        MBR_TAG_PRINTF("\t checksum: 0x%08x", _mbr_info.boot_app.fw_header.checksum);
        MBR_TAG_PRINTF("\t size: %u(%s)", _mbr_info.boot_app.fw_header.size, 
                        readableSize(_mbr_info.boot_app.fw_header.size).c_str());
        MBR_TAG_PRINTF("\t version: 0x%08X", _mbr_info.boot_app.fw_header.version.u32);
        MBR_TAG_PRINTF("\t type: %s, %s, %s\n", 
                        _mbr_info.boot_app.fw_header.type.mem ? "external" : "internal",
                        _mbr_info.boot_app.fw_header.type.enc ? "encrypt" : "raw",
                        _mbr_info.boot_app.fw_header.type.app ? "Main" : "Boot");

        MBR_TAG_PRINTF("main_rollback:");
        MBR_TAG_PRINTF("\t startup_addr: 0x%08x", _mbr_info.main_rollback.startup_addr);
        MBR_TAG_PRINTF("\t max_size: %s", readableSize(_mbr_info.main_rollback.max_size).c_str());
        MBR_TAG_PRINTF("\t checksum: 0x%08x", _mbr_info.main_rollback.fw_header.checksum);
        MBR_TAG_PRINTF("\t size: %u(%s)", _mbr_info.main_rollback.fw_header.size, 
                        readableSize(_mbr_info.main_rollback.fw_header.size).c_str());
        MBR_TAG_PRINTF("\t version: 0x%08X", _mbr_info.main_rollback.fw_header.version.u32);
        MBR_TAG_PRINTF("\t type: %s, %s, %s\n", 
                        _mbr_info.main_rollback.fw_header.type.mem ? "external" : "internal",
                        _mbr_info.main_rollback.fw_header.type.enc ? "encrypt" : "raw",
                        _mbr_info.main_rollback.fw_header.type.app ? "Main" : "Boot");

        MBR_TAG_PRINTF("boot_rollback:");
        MBR_TAG_PRINTF("\t startup_addr: 0x%08x", _mbr_info.boot_rollback.startup_addr);
        MBR_TAG_PRINTF("\t max_size: %s", readableSize(_mbr_info.boot_rollback.max_size).c_str());
        MBR_TAG_PRINTF("\t checksum: 0x%08x", _mbr_info.boot_rollback.fw_header.checksum);
        MBR_TAG_PRINTF("\t size: %u(%s)", _mbr_info.boot_rollback.fw_header.size, 
                        readableSize(_mbr_info.boot_rollback.fw_header.size).c_str());
        MBR_TAG_PRINTF("\t version: 0x%X08", _mbr_info.boot_rollback.fw_header.version.u32);
        MBR_TAG_PRINTF("\t type: %s, %s, %s\n", 
                        _mbr_info.boot_rollback.fw_header.type.mem ? "external" : "internal",
                        _mbr_info.boot_rollback.fw_header.type.enc ? "encrypt" : "raw",
                        _mbr_info.boot_rollback.fw_header.type.app ? "Main" : "Boot");

        MBR_TAG_PRINTF("image_download:");
        MBR_TAG_PRINTF("\t startup_addr: 0x%08x", _mbr_info.image_download.startup_addr);
        MBR_TAG_PRINTF("\t max_size: %s", readableSize(_mbr_info.image_download.max_size).c_str());
        MBR_TAG_PRINTF("\t checksum: 0x%08x", _mbr_info.image_download.fw_header.checksum);
        MBR_TAG_PRINTF("\t size: %u(%s)", _mbr_info.image_download.fw_header.size, 
                        readableSize(_mbr_info.image_download.fw_header.size).c_str());
        MBR_TAG_PRINTF("\t version: 0x%08X", _mbr_info.image_download.fw_header.version.u32);
        MBR_TAG_PRINTF("\t type: %s, %s, %s\n",
                        _mbr_info.image_download.fw_header.type.mem ? "external" : "internal",
                        _mbr_info.image_download.fw_header.type.enc ? "encrypt" : "raw",
                        _mbr_info.image_download.fw_header.type.app ? "Main" : "Boot");

        MBR_TAG_PRINTF("main dfu_num: %u", _mbr_info.dfu_num.main);
        MBR_TAG_PRINTF("boot dfu_num: %u", _mbr_info.dfu_num.boot);
        MBR_TAG_PRINTF("hw_version_str: %16s", _mbr_info.hw_version_str);
        MBR_TAG_PRINTF("startup_mode: %u", _mbr_info.common.startup_mode);
        MBR_TAG_PRINTF("dfu_mode: %u\n", _mbr_info.common.dfu_mode);
    }
    else
    {
        MBR_TAG_PRINTF("MBR information load ERROR");
    }
#endif
}

void MasterBootRecord::printAppInfo(app_info_t *pParams)
{
    MBR_TAG_PRINTF("[printAppInfo] Information");
    MBR_TAG_PRINTF("\t startup_addr: 0x%08x", pParams->startup_addr);
    MBR_TAG_PRINTF("\t max_size: %s", readableSize(pParams->max_size).c_str());
    MBR_TAG_PRINTF("\t checksum: 0x%08x", pParams->fw_header.checksum);
    MBR_TAG_PRINTF("\t size: %u(%s)", pParams->fw_header.size, 
                    readableSize(pParams->fw_header.size).c_str());
    MBR_TAG_PRINTF("\t version: 0x%08X", pParams->fw_header.version.u32);
    MBR_TAG_PRINTF("\t type(0x%X): %s, %s, %s\n",
                    pParams->fw_header.type.u32,
                    pParams->fw_header.type.mem ? "external" : "internal",
                    pParams->fw_header.type.enc ? "encrypt" : "raw",
                    pParams->fw_header.type.app ? "Main" : "Boot");
}

void MasterBootRecord::printFwHeaderInfo(firmwareHeader_t *fwHeader)
{
    MBR_TAG_PRINTF("[printFwHeaderInfo] Information");
    MBR_TAG_PRINTF("\t checksum: 0x%08x", fwHeader->checksum);
    MBR_TAG_PRINTF("\t size: %u(%s)", fwHeader->size, 
                    readableSize(fwHeader->size).c_str());
    MBR_TAG_PRINTF("\t version: 0x%08X", fwHeader->version.u32);
    MBR_TAG_PRINTF("\t type(0x%X): %s, %s, %s\n",
                    fwHeader->type.u32,
                    fwHeader->type.mem ? "external" : "internal",
                    fwHeader->type.enc ? "encrypt" : "raw",
                    fwHeader->type.app ? "Main" : "Boot");
}

std::string MasterBootRecord::readableSize(float bytes) {
    char buff[10];
    std::string var;
    if (bytes < 1024)
    {
        snprintf(buff, 10, "%u", (uint32_t)bytes);
        var = std::string(buff) + " B";
        return var;
    }
    const char* const units[] = {" KiB", " MiB", " GiB", " TiB", "PiB"};
    int i = -1;
    do {
        bytes = bytes / 1024;
        i++;
    } while (bytes >= 1024);
    snprintf(buff, 10, "%u.%02u", (uint32_t)bytes, (uint32_t)(bytes*100)%100);
    var = std::string(buff) + units[i];
    return var;
}