/* Includes ------------------------------------------------------------------*/
#include "partition_manager.h"
#include "util_crc32.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

static void printData(const void* data, size_t length)
{
    const char* dataBytes = (const char*)data;
    for (size_t i = 0; i < length; i++) {
        if ((i % 16) == 0)
            PARTITION_MNG_PRINTF("\t");
        PARTITION_MNG_PRINTF("%02X ", dataBytes[i]);
    }
    PARTITION_MNG_PRINTF("\n");
}

SPIFBlockDevice* partition_manager::_spiDevice = nullptr;

partition_manager::partition_manager(SPIFBlockDevice* spiDevice) :
_mbr(),
aes128()
{
    _spiDevice = spiDevice;
    _init_isOK = false;
}

partition_manager::~partition_manager()
{
    this->end();
}

void partition_manager::begin(void)
{
    app_info_t app;
    firmwareHeader_t fw_header;
    bool mbr_update = false;

    if(_init_isOK)
    {
        return;
    }

    _spiDevice->init();  
    _mbr.begin();
    
    app = _mbr.getMainParams();
    if (MBR_CRC_APP_FACTORY == app.fw_header.checksum)
    {
        memcpy(&fw_header, (uint8_t*)MAIN_APP_HEADER_GENERAL_LOCATION, sizeof(firmwareHeader_t));
        PARTITION_MNG_TAG_PRINTF("[begin] Main firmware header at 0x%08X", MAIN_APP_HEADER_GENERAL_LOCATION);
        PARTITION_MNG_TAG_PRINTF("\t checksum: 0x%08X", fw_header.checksum);
        PARTITION_MNG_TAG_PRINTF("\t size: %u(%s)", fw_header.size, readableSize(fw_header.size).c_str());
        PARTITION_MNG_TAG_PRINTF("\t type: 0x%08X", fw_header.type.u32);
        PARTITION_MNG_TAG_PRINTF("\t version: 0x%08X", fw_header.version.u32);
        if (MBR_CRC_APP_NONE != fw_header.checksum
        && FW_APP_MAIN_TYPE == fw_header.type.u32)
        {
            app.fw_header = fw_header;
            PARTITION_MNG_TAG_PRINTF("[begin]\t verify Main application");
            if (verify(&app))
            {
                PARTITION_MNG_TAG_PRINTF("[begin]\t Main application OK");
                PARTITION_MNG_TAG_PRINTF("[begin]\t Update main header MBR\n");
                _mbr.setMainParams(&app);
                mbr_update = true;
            }
            else
            {
                PARTITION_MNG_TAG_PRINTF("[begin]\t Main application ERROR\n");
            }
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[begin]\t Main application isn't exist\n");
        }
    }

    app = _mbr.getBootParams();
    if (MBR_CRC_APP_FACTORY == app.fw_header.checksum)
    {
        memcpy(&fw_header, (uint8_t*)BOOT_APP_HEADER_GENERAL_LOCATION, sizeof(firmwareHeader_t));
        PARTITION_MNG_TAG_PRINTF("[begin] Boot firmware header at 0x%08X", BOOT_APP_HEADER_GENERAL_LOCATION);
        PARTITION_MNG_TAG_PRINTF("\t checksum: 0x%08X", fw_header.checksum);
        PARTITION_MNG_TAG_PRINTF("\t size: %u(%s)", fw_header.size, readableSize(fw_header.size).c_str());
        PARTITION_MNG_TAG_PRINTF("\t type: 0x%08X", fw_header.type.u32);
        PARTITION_MNG_TAG_PRINTF("\t version: 0x%08X", fw_header.version.u32);
        if (MBR_CRC_APP_NONE != fw_header.checksum
        && FW_APP_MAIN_TYPE == fw_header.type.u32)
        {
            app.fw_header = fw_header;
            PARTITION_MNG_TAG_PRINTF("[begin]\t verify Boot application");
            if (verify(&app))
            {
                PARTITION_MNG_TAG_PRINTF("[begin]\t Boot application OK");
                PARTITION_MNG_TAG_PRINTF("[begin]\t Update boot header MBR");
                _mbr.setMainParams(&app);
                mbr_update = true;
            }
            else
            {
                PARTITION_MNG_TAG_PRINTF("[begin]\t Boot application ERROR\n");
            }
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[begin]\t Boot application isn't exist\n");
        }
    }

    if (mbr_update)
    {
        PARTITION_MNG_TAG_PRINTF("[begin]\t store MBR");
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[begin]\t store MBR succeed!\n");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[begin]\t store MBR failure!\n");
        }
    }

    _mbr.printMbrInfo();
    this->printPartition();
    _init_isOK = true;
}

void partition_manager::end(void)
{
    _mbr.end();
    _spiDevice->deinit();
}

MasterBootRecord::startup_mode_t partition_manager::getStartUpModeFromMBR(void)
{
    return _mbr.getStartUpMode();
}

/* return true if succeed */
bool partition_manager::setStartUpModeToMBR(MasterBootRecord::startup_mode_t mode)
{
    PARTITION_MNG_TAG_PRINTF("[setStartUpModeToMBR]");
    if (getStartUpModeFromMBR() == mode)
    {
        return true;
    }
    _mbr.setStartUpMode(mode);
    return (_mbr.commit() == MasterBootRecord::MBR_OK);
}

MasterBootRecord::app_status_t partition_manager::getMainStatusFromMBR(void)
{
    return _mbr.getMainStatus();
}

bool partition_manager::setMainStatusToMBR(MasterBootRecord::app_status_t status)
{
    PARTITION_MNG_TAG_PRINTF("[setMainStatusToMBR]");
    if (getMainStatusFromMBR() == status)
    {
        return true;
    }
    _mbr.setMainStatus(status);
    return (_mbr.commit() == MasterBootRecord::MBR_OK);
}

MasterBootRecord::app_status_t partition_manager::getBootStatusFromMBR(void)
{
    return _mbr.getBootStatus();
}

bool partition_manager::setBootStatusToMBR(MasterBootRecord::app_status_t status)
{
    PARTITION_MNG_TAG_PRINTF("[setBootStatusToMBR]");
    if (getBootStatusFromMBR() == status)
    {
        return true;
    }
    _mbr.setBootStatus(status);
    return (_mbr.commit() == MasterBootRecord::MBR_OK);
}

uint32_t partition_manager::mainAddress(void)
{
    app_info_t app;
    uint32_t addr;

    app = _mbr.getMainParams();
    addr = app.startup_addr;
    PARTITION_MNG_TAG_PRINTF("[mainAddress] 0x%08X (%u)", addr, addr);
    return addr;
}

uint32_t partition_manager::bootAddress(void)
{
    app_info_t app;
    uint32_t addr;
    
    app = _mbr.getBootParams();
    addr = app.startup_addr;
    PARTITION_MNG_TAG_PRINTF("[bootAddress] 0x%08X (%u)", addr, addr);
    return addr;
}

void partition_manager::printPartition(void)
{
#if (0)
    FlashIAPBlockDevice* iapFlash;
    FlashSPIBlockDevice* spiFlash;
    app_info_t app;
    uint32_t size;
    
    PARTITION_MNG_TAG_PRINTF("spiDevice");
    size =  _spiDevice->size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", size,
                                readableSize(size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", _spiDevice->get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", _spiDevice->get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", _spiDevice->get_erase_size());
    
    PARTITION_MNG_TAG_PRINTF("main");
    app = _mbr.getMainParams();
    iapFlash = new FlashIAPBlockDevice(app.startup_addr, app.max_size);
    iapFlash->init();
    size =  iapFlash->size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", size,
                                readableSize(size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", iapFlash->get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", iapFlash->get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", iapFlash->get_erase_size());
    delete iapFlash;

    PARTITION_MNG_TAG_PRINTF("boot");
    app = _mbr.getBootParams();
    iapFlash = new FlashIAPBlockDevice(app.startup_addr, app.max_size);
    iapFlash->init();
    size =  iapFlash->size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", size,
                                readableSize(size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", iapFlash->get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", iapFlash->get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", iapFlash->get_erase_size());
    delete iapFlash;

    PARTITION_MNG_TAG_PRINTF("main rollback");
    app = _mbr.getMainRollbackParams();
    spiFlash = new FlashSPIBlockDevice(_spiDevice, app.startup_addr, app.max_size);
    size =  spiFlash->size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", size,
                                readableSize(size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", spiFlash->get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", spiFlash->get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", spiFlash->get_erase_size());
    delete spiFlash;

    PARTITION_MNG_TAG_PRINTF("boot rollback");
    app = _mbr.getBootRollbackParams();
    spiFlash = new FlashSPIBlockDevice(_spiDevice, app.startup_addr, app.max_size);
    size =  spiFlash->size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", size,
                                readableSize(size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", spiFlash->get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", spiFlash->get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", spiFlash->get_erase_size());
    delete spiFlash;

    PARTITION_MNG_TAG_PRINTF("image download");
    app = _mbr.getImageDownloadParams();
    spiFlash = new FlashSPIBlockDevice(_spiDevice, app.startup_addr, app.max_size);
    size =  spiFlash->size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", size,
                                readableSize(size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", spiFlash->get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", spiFlash->get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", spiFlash->get_erase_size());
    delete spiFlash;
#endif
}

/** 
 *  @return    True if application is valid for partition region
 */
bool partition_manager::verifyMain(void)
{
    app_info_t app;
    bool status;
    PARTITION_MNG_TAG_PRINTF("[verifyMain]>> start");
    app = _mbr.getMainParams();
    status = this->verify(&app);
    PARTITION_MNG_TAG_PRINTF("[verifyMain]<< finish, status %s", status ? "OK":"Fail");
    return status;
}

bool partition_manager::verifyBoot(void)
{
    app_info_t app;
    bool status;
    PARTITION_MNG_TAG_PRINTF("[verifyBoot]>> start");
    app = _mbr.getBootParams();
    status = this->verify(&app);
    PARTITION_MNG_TAG_PRINTF("[verifyBoot]<< finish, status %s", status ? "OK":"Fail");
    return status;
}

bool partition_manager::verifyMainRollback(void)
{
    app_info_t app;
    bool status;
    PARTITION_MNG_TAG_PRINTF("[verifyMainRollback]>> start");
    app = _mbr.getMainRollbackParams();
    status = this->verify(&app);
    PARTITION_MNG_TAG_PRINTF("[verifyMainRollback]<< finish, status %s", status ? "OK":"Fail");
    return status;
}

bool partition_manager::verifyBootRollback(void)
{
    app_info_t app;
    bool status;
    PARTITION_MNG_TAG_PRINTF("[verifyBootRollback]>> start");
    app = _mbr.getBootRollbackParams();
    status = this->verify(&app);
    PARTITION_MNG_TAG_PRINTF("[verifyBootRollback]<< finish, status %s", status ? "OK":"Fail");
    return status;
}

bool partition_manager::verifyImageDownload(void)
{
    app_info_t app;
    bool status;
    PARTITION_MNG_TAG_PRINTF("[verifyImageDownload]>> start");
    app = _mbr.getImageDownloadParams();
    status = this->verify(&app);
    PARTITION_MNG_TAG_PRINTF("[verifyImageDownload]<< finish, status %s", status ? "OK":"Fail");
    return status;
}

uint8_t partition_manager::appUpgrade(void)
{
    app_info_t app;
    uint8_t type_app;
    app = _mbr.getImageDownloadParams();
    type_app = app.fw_header.type.app;
    PARTITION_MNG_TAG_PRINTF("[appUpgrade] %s", type_app ? "Main" : "Boot");
    return type_app;
}

bool partition_manager::upgradeMain(void)
{
    app_info_t des;
    app_info_t src;
    MasterBootRecord::dfu_mode_t dfu_mode;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[upgradeMain]>> start");
    des = _mbr.getMainParams();
    src = _mbr.getImageDownloadParams();
    dfu_mode = _mbr.getDfuMode();

    PARTITION_MNG_TAG_PRINTF("[upgradeMain]>\t Main version 0x%08X", des.fw_header.version.u32);
    PARTITION_MNG_TAG_PRINTF("[upgradeMain]>\t New version 0x%08X", src.fw_header.version.u32);
    if (MasterBootRecord::UPGRADE_MODE_UP == dfu_mode)
    {
        if (des.fw_header.version.u32 > src.fw_header.version.u32)
        {
            PARTITION_MNG_TAG_PRINTF("\t Prevent upgrade !");
            return false;
        }
    }

    if (programApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = MasterBootRecord::APP_STATUS_WAIT_CONFIRM;
        PARTITION_MNG_TAG_PRINTF("[upgradeMain] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[upgradeMain] update des into MBR");
        _mbr.setMainParams(&des);
        _mbr.setMainDfuNum(_mbr.getMainDfuNum() + 1);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[upgradeMain]\t succeed!");
            status_isOK = true;
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[upgradeMain]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[upgradeMain]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[upgradeMain]<< finish");
    return status_isOK;
}

bool partition_manager::upgradeBoot(void)
{
    app_info_t des;
    app_info_t src;
    MasterBootRecord::dfu_mode_t dfu_mode;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[upgradeBoot]>> start");
    des = _mbr.getBootParams();
    src = _mbr.getImageDownloadParams();
    dfu_mode = _mbr.getDfuMode();

    PARTITION_MNG_TAG_PRINTF("[upgradeBoot]>\t Main version 0x%08X", des.fw_header.version.u32);
    PARTITION_MNG_TAG_PRINTF("[upgradeBoot]>\t New version 0x%08X", src.fw_header.version.u32);
    if (MasterBootRecord::UPGRADE_MODE_UP == dfu_mode)
    {
        if (des.fw_header.version.u32 > src.fw_header.version.u32)
        {
            PARTITION_MNG_TAG_PRINTF("\t Prevent upgrade !");
            return false;
        }
    }

    if (programApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = MasterBootRecord::APP_STATUS_WAIT_CONFIRM;
        PARTITION_MNG_TAG_PRINTF("[upgradeBoot] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[upgradeBoot] update des into MBR");
        _mbr.setBootParams(&des);
        _mbr.setBootDfuNum(_mbr.getBootDfuNum() + 1);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[upgradeBoot]\t succeed!");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[upgradeBoot]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[upgradeBoot]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[upgradeBoot]<< finish");
    return status_isOK;
}

bool partition_manager::restoreMain(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[restoreMain]>> start");
    des = _mbr.getMainParams();
    src = _mbr.getMainRollbackParams();

    if (MasterBootRecord::APP_STATUS_OK != src.common.app_status)
    {
        PARTITION_MNG_TAG_PRINTF("[restoreMain]\t app status Failure!");
        return false;
    }

    if (programApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = src.common.app_status;
        PARTITION_MNG_TAG_PRINTF("[restoreMain] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[restoreMain] update des into MBR");
        _mbr.setMainParams(&des);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[restoreMain]\t succeed!");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[restoreMain]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[restoreMain]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[restoreMain]<< finish");
    return status_isOK;
}

bool partition_manager::restoreBoot(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[restoreBoot]>> start");
    des = _mbr.getBootParams();
    src = _mbr.getBootRollbackParams();

    if (MasterBootRecord::APP_STATUS_OK != src.common.app_status)
    {
        PARTITION_MNG_TAG_PRINTF("[restoreBoot]\t app status Failure!");
        return false;
    }

    if (programApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = src.common.app_status;
        PARTITION_MNG_TAG_PRINTF("[restoreBoot] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[restoreBoot] update des into MBR");
        _mbr.setBootParams(&des);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[restoreBoot]\t succeed!");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[restoreBoot]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[restoreBoot]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[restoreBoot]<< finish");
    return status_isOK;
}

bool partition_manager::backupMain(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[backupMain]>> start");
    des = _mbr.getMainRollbackParams();
    src = _mbr.getMainParams();

    if (MasterBootRecord::APP_STATUS_OK != src.common.app_status)
    {
        PARTITION_MNG_TAG_PRINTF("[backupMain]\t app status Failure!");
        return false;
    }

    if (backupApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = src.common.app_status;
        PARTITION_MNG_TAG_PRINTF("[backupMain] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[backupMain] update MBR");
        _mbr.setMainRollbackParams(&des);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[backupMain]\t succeed!");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[backupMain]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[backupMain]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[backupMain]<< finish");
    return status_isOK;
}

bool partition_manager::backupMain2ImageDownload(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[backupMain2ImageDownload]>> start");
    des = _mbr.getImageDownloadParams();
    src = _mbr.getMainParams();
    if (backupApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = src.common.app_status;
        PARTITION_MNG_TAG_PRINTF("[backupMain2ImageDownload] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[backupMain2ImageDownload] update MBR");
        _mbr.setImageDownloadParams(&des);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[backupMain2ImageDownload]\t succeed!");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[backupMain2ImageDownload]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[backupMain2ImageDownload]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[backupMain2ImageDownload]<< finish");
    return status_isOK;
}

bool partition_manager::cloneMain2ImageDownload(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[cloneMain2ImageDownload]>> start");
    des = _mbr.getImageDownloadParams();
    src = _mbr.getMainParams();
    if (cloneApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = src.common.app_status;
        PARTITION_MNG_TAG_PRINTF("[cloneMain2ImageDownload] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[cloneMain2ImageDownload] update MBR");
        _mbr.setImageDownloadParams(&des);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[cloneMain2ImageDownload]\t succeed!");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[cloneMain2ImageDownload]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[cloneMain2ImageDownload]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[cloneMain2ImageDownload]<< finish");
    return status_isOK;
}

bool partition_manager::backupBoot(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[backupBoot]>> start");
    des = _mbr.getBootRollbackParams();
    src = _mbr.getBootParams();

    if (MasterBootRecord::APP_STATUS_OK != src.common.app_status)
    {
        PARTITION_MNG_TAG_PRINTF("[backupBoot]\t app status Failure!");
        return false;
    }

    if (backupApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
        des.common.app_status = src.common.app_status;
        PARTITION_MNG_TAG_PRINTF("[backupBoot] update des crc32");
        des.fw_header.checksum = CRC32(&des);
        PARTITION_MNG_TAG_PRINTF("[backupBoot] update MBR");
        _mbr.setBootRollbackParams(&des);
        if(_mbr.commit() == MasterBootRecord::MBR_OK)
        {
            PARTITION_MNG_TAG_PRINTF("[backupBoot]\t succeed!");
        }
        else
        {
            PARTITION_MNG_TAG_PRINTF("[backupBoot]\t failure!");
            status_isOK = false;
        }
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[backupBoot]\t failure!");
        status_isOK = false;
    }
    PARTITION_MNG_TAG_PRINTF("[backupBoot]<< finish");
    return status_isOK;
}

bool partition_manager::programApp(app_info_t* des, app_info_t* src)
{
    FlashHandler* desFlash;
    FlashHandler* srcFlash;
    uint32_t addr;
    uint32_t remain_size;
    uint32_t read_size;
    uint32_t block_size;
    uint32_t crc;
    uint8_t *ptr_data;
    bool status_isOK = true;
    bool decrypt_image = true;

    PARTITION_MNG_TAG_PRINTF("[programApp]>> start");
    PARTITION_MNG_TAG_PRINTF("[programApp]\t Src external: addr=0x%08X; size=%u",
                            src->startup_addr,
                            src->fw_header.size);
    PARTITION_MNG_TAG_PRINTF("[programApp]\t Des internal: addr=0x%08X; max_size=%u",
                            des->startup_addr,
                            des->max_size);

    if (src->fw_header.type.mem != MasterBootRecord::MEMORY_EXTERNAL
    || des->fw_header.type.mem != MasterBootRecord::MEMORY_INTERNAL)
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t type memory des/src ERROR");
        return false;
    }

    PARTITION_MNG_TAG_PRINTF("[programApp] verify source");
    if (!verify(src))
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t application source ERROR");
        return false;
    }

    if (src->fw_header.size > des->max_size)
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t Des partition size isn't enough to store source image");
        return false;
    }

    desFlash = new FlashHandler(des);
    srcFlash = new FlashHandler(src);

    block_size = desFlash->get_erase_size();
    /* Allocate dynamic memory */
    ptr_data = new (std::nothrow) uint8_t[block_size];
    if (ptr_data == nullptr)
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t allocate %u memory failed!", block_size);
        delete desFlash;
        delete srcFlash;
        return false;
    }

    if (MasterBootRecord::DATA_ENC == src->fw_header.type.enc)
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t processing decrypt image");
        decrypt_image = true;
        AES128_crypto_t mbr_aes = _mbr.getAes128Params();
        aes128.setup((const char*)mbr_aes.key, AES::KEY_128, AES::MODE_CBC, (const char*)mbr_aes.iv);
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t processing image");
        decrypt_image = false;
    }

    remain_size = src->fw_header.size;
    addr = 0;
    while (remain_size)
    {
        if (remain_size > block_size)
        {
            read_size = block_size;
        }
        else
        {
            read_size = remain_size;
        }
        desFlash->erase(addr, block_size);
        srcFlash->read(ptr_data, addr, read_size);
        if (decrypt_image)
        {
            /* Decrypt data before write to des partition */
            aes128.decrypt(ptr_data, read_size);
        }
        desFlash->program(ptr_data, addr, read_size);
#if defined(PM_VERIFY_DATA_BY_CRC32) && (PM_VERIFY_DATA_BY_CRC32 == 1)
        crc = Crc32_CalculateBuffer(ptr_data, read_size);
        desFlash->read(ptr_data, addr, read_size);
        if (crc != Crc32_CalculateBuffer(ptr_data, read_size))
        {
            status_isOK = false;
            PARTITION_MNG_TAG_PRINTF("[programApp]\t crc32=0x%08X fail!", crc);
            break;
        }
#endif
        addr += read_size;
        remain_size -= read_size;
        PARTITION_MNG_TAG_PRINTF("[programApp]\t %u, %08X, %u%%", read_size, crc, addr * 100 / src->fw_header.size);
    }

    if (decrypt_image)
    {
        aes128.clear();
    }
    delete[] ptr_data;
    delete desFlash;
    delete srcFlash;
    aes128.clear();

    PARTITION_MNG_TAG_PRINTF("[programApp]<< finish");

    return status_isOK;
} // programApp

/** @brief copy application image from internal to external
 * @param des information des application
 * @param src information src application
*/
bool partition_manager::backupApp(app_info_t* des, app_info_t* src)
{
    FlashHandler* desFlash;
    FlashHandler* srcFlash;
    uint32_t addr;
    uint32_t remain_size;
    uint32_t read_size;
    uint32_t block_size;
    uint32_t crc;
    uint8_t *ptr_data;
    bool status_isOK = true;
    bool encrypt_image = true;

    PARTITION_MNG_TAG_PRINTF("[backupApp]>> start");
    PARTITION_MNG_TAG_PRINTF("[backupApp]\t Src internal: addr=0x%08X; size=%u",
                            src->startup_addr,
                            src->fw_header.size);
    PARTITION_MNG_TAG_PRINTF("[backupApp]\t Des external: addr=0x%08X; max_size=%u",
                            des->startup_addr,
                            des->max_size);

    if (des->fw_header.type.mem != MasterBootRecord::MEMORY_EXTERNAL
    || src->fw_header.type.mem != MasterBootRecord::MEMORY_INTERNAL)
    {
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t type memory des/src ERROR");
        return false;
    }

    PARTITION_MNG_TAG_PRINTF("[programApp] verify source");
    if (!verify(src))
    {
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t application source ERROR");
        return false;
    }

    if (src->fw_header.size > des->max_size)
    {
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t Des partition size isn't enough to store source image");
        return false;
    }

    desFlash = new FlashHandler(des);
    srcFlash = new FlashHandler(src);

    block_size = desFlash->get_erase_size();
    /* Allocate dynamic memory */
    ptr_data = new (std::nothrow) uint8_t[block_size];
    if (ptr_data == nullptr)
    {
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t allocate %u memory failed!", block_size);
        delete desFlash;
        delete srcFlash;
        return false;
    }

    if (MasterBootRecord::DATA_ENC == des->fw_header.type.enc)
    {
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t processing encrypt image");
        encrypt_image = true;
        AES128_crypto_t mbr_aes = _mbr.getAes128Params();
        aes128.setup((const char*)mbr_aes.key, AES::KEY_128, AES::MODE_CBC, (const char*)mbr_aes.iv);
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t processing image");
        encrypt_image = false;
    }

    remain_size = src->fw_header.size;
    addr = 0;
    while (remain_size)
    {
        if (remain_size > block_size)
        {
            read_size = block_size;
        }
        else
        {
            read_size = remain_size;
        }
        desFlash->erase(addr, block_size);
        srcFlash->read(ptr_data, addr, read_size);
        if (encrypt_image)
        {
            /* Encrypt data before write to des partition */
            aes128.encrypt(ptr_data, read_size);
        }
        desFlash->program(ptr_data, addr, read_size);
#if defined(PM_VERIFY_DATA_BY_CRC32) && (PM_VERIFY_DATA_BY_CRC32 == 1)
        crc = Crc32_CalculateBuffer(ptr_data, read_size);
        desFlash->read(ptr_data, addr, read_size);
        if (crc != Crc32_CalculateBuffer(ptr_data, read_size))
        {
            status_isOK = false;
            PARTITION_MNG_TAG_PRINTF("[backupApp]\t crc32=0x%08X fail!", crc);
            break;
        }
#endif
        addr += read_size;
        remain_size -= read_size;
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t %u, %08X, %u%%", read_size, crc, addr * 100 / src->fw_header.size);
    }

    if (encrypt_image)
    {
        aes128.clear();
    }
    delete[] ptr_data;
    delete desFlash;
    delete srcFlash;

    PARTITION_MNG_TAG_PRINTF("[backupApp]<< finish");

    return status_isOK;
} // backupApp

bool partition_manager::cloneApp(app_info_t* des, app_info_t* src)
{
    FlashHandler* desFlash;
    FlashHandler* srcFlash;
    uint32_t addr;
    uint32_t remain_size;
    uint32_t read_size;
    uint32_t block_size;
    uint32_t crc;
    uint8_t *ptr_data;
    bool status_isOK = true;

    PARTITION_MNG_TAG_PRINTF("[cloneApp]>> start");
    PARTITION_MNG_TAG_PRINTF("[cloneApp]\t Src internal: addr=0x%08X; size=%u",
                            src->startup_addr,
                            src->fw_header.size);
    PARTITION_MNG_TAG_PRINTF("[cloneApp]\t Des external: addr=0x%08X; max_size=%u",
                            des->startup_addr,
                            des->max_size);

    PARTITION_MNG_TAG_PRINTF("[programApp] verify source");
    if (!verify(src))
    {
        PARTITION_MNG_TAG_PRINTF("[cloneApp]\t application source ERROR");
        return false;
    }

    if (src->fw_header.size > des->max_size)
    {
        PARTITION_MNG_TAG_PRINTF("[cloneApp]\t Des partition size isn't enough to store source image");
        return false;
    }

    desFlash = new FlashHandler(des);
    srcFlash = new FlashHandler(src);

    block_size = desFlash->get_erase_size();
    /* Allocate dynamic memory */
    ptr_data = new (std::nothrow) uint8_t[block_size];
    if (ptr_data == nullptr)
    {
        PARTITION_MNG_TAG_PRINTF("[cloneApp]\t allocate %u memory failed!", block_size);
        delete desFlash;
        delete srcFlash;
        return false;
    }

    remain_size = src->fw_header.size;
    addr = 0;
    while (remain_size)
    {
        if (remain_size > block_size)
        {
            read_size = block_size;
        }
        else
        {
            read_size = remain_size;
        }
        desFlash->erase(addr, block_size);
        srcFlash->read(ptr_data, addr, read_size);
        desFlash->program(ptr_data, addr, read_size);
#if defined(PM_VERIFY_DATA_BY_CRC32) && (PM_VERIFY_DATA_BY_CRC32 == 1)
        crc = Crc32_CalculateBuffer(ptr_data, read_size);
        desFlash->read(ptr_data, addr, read_size);
        if (crc != Crc32_CalculateBuffer(ptr_data, read_size))
        {
            status_isOK = false;
            PARTITION_MNG_TAG_PRINTF("[cloneApp]\t crc32=0x%08X fail!", crc);
            break;
        }
#endif
        addr += read_size;
        remain_size -= read_size;
        PARTITION_MNG_TAG_PRINTF("[cloneApp]\t %u, %08X, %u%%", read_size, crc, addr * 100 / src->fw_header.size);
    }

    delete[] ptr_data;
    delete desFlash;
    delete srcFlash;

    PARTITION_MNG_TAG_PRINTF("[cloneApp]<< finish");

    return status_isOK;
} // cloneApp

/** Convenience function for checking partition region validity
 *
 *  @param app      app information
 *  @return         True if verify crc32 is valid for partition region
 */
bool partition_manager::verify(app_info_t* app)
{
  uint32_t crc;
  bool result = false;
  PARTITION_MNG_TAG_PRINTF("[verify]>> start");

  if (FIRMWARE_TYPE_SIGNAL != app->fw_header.type.signal)
  {
      PARTITION_MNG_TAG_PRINTF("[verify]\t app header signal error");
      return false;
  }

  PARTITION_MNG_TAG_PRINTF("[verify]\t check CRC32");
  crc = this->CRC32(app);
  if(crc == app->fw_header.checksum)
  {
    result = true;
    PARTITION_MNG_TAG_PRINTF("[verify]\t crc=0x%08X App OK", crc);
  }
  else
  {
    result = false;
    PARTITION_MNG_TAG_PRINTF("[verify]\t error crc=0x%08X, expected crc=0x%08X", crc, app->fw_header.checksum);
    PARTITION_MNG_TAG_PRINTF("[verify]\t App Fail");
  }
  PARTITION_MNG_TAG_PRINTF("[verify]<< finish");
  return result;
}

/**
 * @brief Calculator CRC32 partition.
 */
uint32_t partition_manager::CRC32(app_info_t* app)
{
    uint32_t crc;

    PARTITION_MNG_TAG_PRINTF("[CRC32]>> start");
    PARTITION_MNG_TAG_PRINTF("[CRC32]\t addr=0x%08X, size=%u", app->startup_addr, app->fw_header.size);
    CRC32_Start(0);
    /* Calculator CRC 12-byte of fw_header*/
    CRC32_Accumulate((uint8_t *) &(app->fw_header.size), 12U);
    if (app->fw_header.type.mem == MasterBootRecord::MEMORY_INTERNAL)
    {
        if (app->fw_header.size > app->max_size)
        {
            PARTITION_MNG_TAG_PRINTF("[CRC32]\t internal fw_header size error");
            return 0;
        }
        PARTITION_MNG_TAG_PRINTF("[CRC32]\t internal memory");
        CRC32_Accumulate((uint8_t *) app->startup_addr, app->fw_header.size);
    }
    else
    {
        FlashSPIBlockDevice* spiFlash;
        uint32_t addr;
        uint32_t remain_size;
        uint32_t read_size;
        uint32_t block_size;

        PARTITION_MNG_TAG_PRINTF("[CRC32]\t external memory");
        spiFlash = new FlashSPIBlockDevice(_spiDevice, app->startup_addr, app->max_size);
        block_size = spiFlash->get_erase_size();
        /* Allocate dynamic memory */
        uint8_t *ptr_data = new (std::nothrow) uint8_t[block_size];
        if (ptr_data == nullptr)
        {
            PARTITION_MNG_TAG_PRINTF("[CRC32]\t allocate %u memory failed!", block_size);
            delete spiFlash;
            return 0;
        }

        remain_size = app->fw_header.size;
        addr = 0;

        if (remain_size > app->max_size)
        {
            PARTITION_MNG_TAG_PRINTF("[CRC32]\t external fw_header size error");
            remain_size = app->max_size;
        }

        while (remain_size)
        {
            if (remain_size > block_size)
            {
                read_size = block_size;
            }
            else
            {
                read_size = remain_size;
            }

            spiFlash->read(ptr_data, addr, read_size);
            CRC32_Accumulate((uint8_t *) ptr_data, read_size);
            addr += read_size;
            remain_size -= read_size;
            PARTITION_MNG_TAG_PRINTF("[CRC32]\t %u%%", addr * 100 / app->fw_header.size);
        }

        delete[] ptr_data;
        delete spiFlash;
    }
    crc = CRC32_Get();
    
    PARTITION_MNG_TAG_PRINTF("[CRC32]\t 0x%08X", crc);
    PARTITION_MNG_TAG_PRINTF("[CRC32]<< finish");
    return crc;
} // fwl_header_crc32

void partition_manager::aesEncrypt(void *data, size_t length)
{
    AES128_crypto_t mbr_aes = _mbr.getAes128Params();
#if (0)
    //Encrypt the message in-place
    PARTITION_MNG_TAG_PRINTF("[aesEncrypt]>> start");
    PARTITION_MNG_TAG_PRINTF("[aesEncrypt]\t Key");
    printData(mbr_aes.key, AES128_LENGTH);
    PARTITION_MNG_TAG_PRINTF("[aesEncrypt]\t Iv");
    printData(mbr_aes.iv, AES128_LENGTH);
    PARTITION_MNG_TAG_PRINTF("[aesEncrypt]\t Data length %u", length);
#endif
    aes128.setup((const char*)mbr_aes.key, AES::KEY_128, AES::MODE_CBC, (const char*)mbr_aes.iv);
    aes128.encrypt(data, length);
    aes128.clear();
    // PARTITION_MNG_TAG_PRINTF("[aesEncrypt]>> finish");
}

void partition_manager::aesDecrypt(void *data, size_t length)
{
    AES128_crypto_t mbr_aes = _mbr.getAes128Params();
#if (0)
    PARTITION_MNG_TAG_PRINTF("[aesDecrypt]>> start");
    PARTITION_MNG_TAG_PRINTF("[aesDecrypt]\t Key");
    printData(mbr_aes.key, AES128_LENGTH);
    PARTITION_MNG_TAG_PRINTF("[aesDecrypt]\t Iv");
    printData(mbr_aes.iv, AES128_LENGTH);
    PARTITION_MNG_TAG_PRINTF("[aesDecrypt]\t Data length %u", length);
#endif
    aes128.setup((const char*)mbr_aes.key, AES::KEY_128, AES::MODE_CBC, (const char*)mbr_aes.iv);
    aes128.decrypt(data, length);
    aes128.clear();
    // PARTITION_MNG_TAG_PRINTF("[aesDecrypt]>> finish");
}

std::string partition_manager::readableSize(float bytes) {
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
} // readableSize