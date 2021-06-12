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
#define PARTITION_MANAGER_WRITE_READ_CRC32 1

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
    MasterBootRecord::startup_mode_t getStartUpModeFromMBR(void);
    bool setStartUpModeFromMBR(MasterBootRecord::startup_mode_t mode);
    uint32_t appAddress(void);
    uint32_t bootAddress(void);

private:
    SPIFBlockDevice* _spiDevice;
    MasterBootRecord _mbr;
    AES aes128;
    std::string readableSize(float bytes);
    bool programApp(app_info_t* des, app_info_t* src);
    bool backupApp(app_info_t* des, app_info_t* src);
    bool verify(app_info_t* app);
    uint32_t CRC32(app_info_t* app);
    void aesEncrypt(void *data, size_t length);
    void aesDecrypt(void *data, size_t length);
};

#endif /* __PARTITON_MANAGER_H */
