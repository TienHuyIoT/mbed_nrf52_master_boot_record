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

partition_manager::partition_manager(SPIFBlockDevice* spiDevice) :
_spiDevice(spiDevice),
_mbr(),
aes128()
{
}

partition_manager::~partition_manager()
{
    this->end();
}

void partition_manager::begin(void)
{
    _spiDevice->init();  
    _mbr.begin();
    _mbr.printMbrInfo();
    this->printPartition();
}

void partition_manager::end(void)
{
    _mbr.end();
    _spiDevice->deinit();
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
bool partition_manager::upgradeMain(void) {return true;}
bool partition_manager::upgradeBoot(void) {return true;}
bool partition_manager::restoreMain(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[restoreMain]>> start");
    des = _mbr.getMainParams();
    src = _mbr.getMainRollbackParams();
    if (programApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
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
bool partition_manager::restoreBoot(void) {return true;}
bool partition_manager::backupMain(void)
{
    app_info_t des;
    app_info_t src;
    bool status_isOK = true;
    PARTITION_MNG_TAG_PRINTF("[backupMain]>> start");
    des = _mbr.getMainRollbackParams();
    src = _mbr.getMainParams();
    if (backupApp(&des, &src))
    {
        des.fw_header.size = src.fw_header.size;
        des.fw_header.version.u32 = src.fw_header.version.u32;
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
bool partition_manager::backupBoot(void) {return true;}

bool partition_manager::programApp(app_info_t* des, app_info_t* src)
{
    FlashSPIBlockDevice* srcFlash;
    FlashIAPBlockDevice* desFlash;
    uint32_t addr, offset;
    uint32_t remain_size;
    uint32_t read_size;
    uint32_t block_size;
    uint32_t crc;
    uint8_t *ptr_data;
    bool status_isOK = true;
    bool decrypt_image = true;

    PARTITION_MNG_TAG_PRINTF("[programApp]>> start");
    PARTITION_MNG_TAG_PRINTF("[programApp]\t Src external: addr=0x%x; size=%u",
                            src->startup_addr,
                            src->fw_header.size);
    PARTITION_MNG_TAG_PRINTF("[programApp]\t Des internal: addr=0x%x; max_size=%u",
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

    desFlash = new FlashIAPBlockDevice(des->startup_addr, des->max_size);
    srcFlash = new FlashSPIBlockDevice(_spiDevice, src->startup_addr, src->max_size);
    desFlash->init();

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
        offset = 0;
    }
    else if (MasterBootRecord::DATA_HEADER_AND_ENC == src->fw_header.type.enc)
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t processing decrypt image");
        decrypt_image = true;
        offset = sizeof(firmwareHeader_t);
    }
    else if (MasterBootRecord::DATA_HEADER_AND_RAW == src->fw_header.type.enc)
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t processing decrypt image");
        decrypt_image = false;
        offset = sizeof(firmwareHeader_t);
    }
    else
    {
        PARTITION_MNG_TAG_PRINTF("[programApp]\t processing image");
        decrypt_image = false;
        offset = 0;
    }

    remain_size = src->fw_header.size - offset;
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
        srcFlash->read(ptr_data, addr + offset, read_size);
        if (decrypt_image)
        {
            /* Decrypt data before write to des partition */
            aesDecrypt(ptr_data, read_size);
        }
        crc = Crc32_CalculateBuffer(ptr_data, read_size);
        desFlash->program(ptr_data, addr, read_size);
        desFlash->read(ptr_data, addr, read_size);
        if (crc != Crc32_CalculateBuffer(ptr_data, read_size))
        {
            status_isOK = false;
            PARTITION_MNG_TAG_PRINTF("[programApp]\t crc32(%08X) fail!", crc);
            break;
        }
        addr += read_size;
        remain_size -= read_size;
        PARTITION_MNG_TAG_PRINTF("[programApp]\t %u, %08X, %u%%", read_size, crc, addr * 100 / src->fw_header.size);
    }

    delete[] ptr_data;
    delete desFlash;
    delete srcFlash;

    PARTITION_MNG_TAG_PRINTF("[programApp]<< finish");

    return status_isOK;
}


/** @brief copy application image from internal to external
 * @param des information des application
 * @param src information src application
*/
bool partition_manager::backupApp(app_info_t* des, app_info_t* src)
{
    FlashSPIBlockDevice* desFlash;
    FlashIAPBlockDevice* srcFlash;
    uint32_t addr;
    uint32_t remain_size;
    uint32_t read_size;
    uint32_t block_size;
    uint32_t crc;
    uint8_t *ptr_data;
    bool status_isOK = true;
    bool encrypt_image = true;

    PARTITION_MNG_TAG_PRINTF("[backupApp]>> start");
    PARTITION_MNG_TAG_PRINTF("[backupApp]\t Src internal: addr=0x%x; size=%u",
                            src->startup_addr,
                            src->fw_header.size);
    PARTITION_MNG_TAG_PRINTF("[backupApp]\t Des external: addr=0x%x; max_size=%u",
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

    desFlash = new FlashSPIBlockDevice(_spiDevice, des->startup_addr, des->max_size);
    srcFlash = new FlashIAPBlockDevice(src->startup_addr, src->max_size);
    srcFlash->init();

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
            aesEncrypt(ptr_data, read_size);
        }
        crc = Crc32_CalculateBuffer(ptr_data, read_size);
        desFlash->program(ptr_data, addr, read_size);
        desFlash->read(ptr_data, addr, read_size);
        if (crc != Crc32_CalculateBuffer(ptr_data, read_size))
        {
            status_isOK = false;
            PARTITION_MNG_TAG_PRINTF("[backupApp]\t crc32(%08X) fail!", crc);
            break;
        }
        addr += read_size;
        remain_size -= read_size;
        PARTITION_MNG_TAG_PRINTF("[backupApp]\t %u, %08X, %u%%", read_size, crc, addr * 100 / src->fw_header.size);
    }

    delete[] ptr_data;
    delete desFlash;
    delete srcFlash;

    PARTITION_MNG_TAG_PRINTF("[backupApp]<< finish");

    return status_isOK;
} // backupApp

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
  PARTITION_MNG_TAG_PRINTF("[verify] verify CRC32");
  crc = this->CRC32(app);
  if(crc == app->fw_header.checksum)
  {
    result = true;
    PARTITION_MNG_TAG_PRINTF("[verify]\t crc(%X) App OK", crc);
  }
  else
  {
    result = false;
    PARTITION_MNG_TAG_PRINTF("[verify] error crc=%X, expected crc=%X", crc, app->fw_header.checksum);
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
    PARTITION_MNG_TAG_PRINTF("[CRC32]\t addr=0x%X, size=%u", app->startup_addr, app->fw_header.size);
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
        PARTITION_MNG_TAG_PRINTF("[CRC32] internal memory");
        CRC32_Accumulate((uint8_t *) app->startup_addr, app->fw_header.size);
    }
    else
    {
        FlashSPIBlockDevice* spiFlash;
        uint32_t addr;
        uint32_t remain_size;
        uint32_t read_size;
        uint32_t block_size;

        PARTITION_MNG_TAG_PRINTF("[CRC32] external memory");
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
            // aesDecrypt(ptr_data, read_size);
            CRC32_Accumulate((uint8_t *) ptr_data, read_size);
            addr += read_size;
            remain_size -= read_size;
            PARTITION_MNG_TAG_PRINTF("[CRC32]\t %u%%", addr * 100 / app->fw_header.size);
        }

        delete[] ptr_data;
        delete spiFlash;
    }
    crc = CRC32_Get();
    
    PARTITION_MNG_TAG_PRINTF("[CRC32]\t %08X", crc);
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