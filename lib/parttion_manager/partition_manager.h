/** @file partition_manager.h
 *  @brief Partition management using for master boot record
 *
 *  @author tienhuyiot
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver    Who                      Date             Changes
 * -----  --------------------     ----------       --------------------
 * 1.0    tienhuyiot@gmail.com     Jun 14, 2021     Initialize
 *
 *
 *</pre>
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PARTITON_MANAGER_H
#define __PARTITON_MANAGER_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include "AES.h"
#include <string>
#include "FlashIAPBlockDevice.h"
#include "FlashSPIBlockDevice.h"
#include "mbr.h"
#include "console_dbg.h"

/* Exported macro ------------------------------------------------------------*/
#define PARTITION_MNG_PRINTF(...) CONSOLE_LOGI(__VA_ARGS__)
#define PARTITION_MNG_TAG_PRINTF(...) CONSOLE_TAG_LOGI("[PARTITION_MNG]", __VA_ARGS__)

/* Private defines -----------------------------------------------------------*/
#define PM_VERIFY_DATA_BY_CRC32 1
#define PM_VERIFY_DATA_BY_MBED_CRC32 0

class partition_manager
{
public:
    partition_manager(SPIFBlockDevice* spiDevice);
    ~partition_manager();
    void begin(void);
    void end(void);
    void printPartition(void);
    bool verifyMain(void);
    bool verifyBoot(void);
    bool verifyMainRollback(void);
    bool verifyBootRollback(void);
    bool verifyImageDownload(void);
    uint8_t appUpgrade(void);
    bool upgradeMain(void);
    bool upgradeBoot(void);
    bool restoreMain(void);
    bool restoreBoot(void);
    bool backupMain(void);
    bool backupBoot(void);
    bool backupMain2ImageDownload(void);
    bool cloneMain2ImageDownload(void);
    MasterBootRecord::startup_mode_t getStartUpModeFromMBR(void);
    bool setStartUpModeToMBR(MasterBootRecord::startup_mode_t mode);
    MasterBootRecord::app_status_t getMainStatusFromMBR(void);
    MasterBootRecord::app_status_t getBootStatusFromMBR(void);
    bool setMainStatusToMBR(MasterBootRecord::app_status_t status);
    bool setBootStatusToMBR(MasterBootRecord::app_status_t status);
    uint32_t mainAddress(void);
    uint32_t bootAddress(void);

private:
    static SPIFBlockDevice* _spiDevice;
    MasterBootRecord _mbr;
#if defined(PM_VERIFY_DATA_BY_MBED_CRC32) && (PM_VERIFY_DATA_BY_MBED_CRC32 == 1)
    MbedCRC<POLY_32BIT_ANSI, 32, CrcMode::BITWISE> _mbedCrc;
#endif
    AES aes128;
    bool _init_isOK;
    std::string readableSize(float bytes);
    bool programApp(app_info_t* des, app_info_t* src);
    bool backupApp(app_info_t* des, app_info_t* src);
    bool cloneApp(app_info_t* des, app_info_t* src);
    bool verify(app_info_t* app);
    uint32_t CRC32(app_info_t* app);
    void aesEncrypt(void *data, size_t length);
    void aesDecrypt(void *data, size_t length);

    class FlashHandler
    {
    private:
        FlashSPIBlockDevice* spiFlash;
        FlashIAPBlockDevice* iapFlash;
        bool _external;
    public:
        FlashHandler(app_info_t* app)
        {
            if (app->fw_header.type.mem == MasterBootRecord::MEMORY_EXTERNAL)
            {
                _external = true;
                spiFlash = new FlashSPIBlockDevice(_spiDevice, app->startup_addr, app->max_size);
                spiFlash->init();
            }
            else
            {
                _external = false;
                iapFlash = new FlashIAPBlockDevice(app->startup_addr, app->max_size);
                iapFlash->init();
            }
        }

        ~FlashHandler(){
            if (_external)
            {
                delete spiFlash;
            }
            else
            {
                delete iapFlash;
            }
        }

        int read(void *buffer, uint32_t addr, uint32_t size) {
            if (_external)
            {
                return spiFlash->read(buffer, addr, size);
            }
            else
            {
                return iapFlash->read(buffer, addr, size);
            }
        }

        int program(const void *buffer, uint32_t addr, uint32_t size) {
            if (_external)
            {
                return spiFlash->program(buffer, addr, size);
            }
            else
            {
                return iapFlash->program(buffer, addr, size);
            }
        }

        int erase(uint32_t addr, uint32_t size) {
            if (_external)
            {
                return spiFlash->erase(addr, size);
            }
            else
            {
                return iapFlash->erase(addr, size);
            }
        }

        uint32_t get_erase_size(void) const {
            if (_external)
            {
                return spiFlash->get_erase_size();
            }
            else
            {
                return iapFlash->get_erase_size();
            }
        }
    };
};

#endif /* __PARTITON_MANAGER_H */
