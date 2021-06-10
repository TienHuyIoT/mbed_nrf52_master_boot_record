/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PARTITON_MANAGER_H
#define __PARTITON_MANAGER_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include "FlashIAPBlockDevice.h"
#include "FlashSPIBlockDevice.h"
#include "mem_layout.h"
#include "console_dbg.h"

/* Exported macro ------------------------------------------------------------*/
#define PARTITION_MNG_PRINTF(...) CONSOLE_LOGI(__VA_ARGS__)
#define PARTITION_MNG_TAG_PRINTF(...) CONSOLE_TAG_LOGI("[PARTITION_MNG]", __VA_ARGS__)

/* Private defines -----------------------------------------------------------*/

class partition_manager
{
public:
    partition_manager(SPIFBlockDevice* spiDevice);
    ~partition_manager();
    void begin(void);
    void end(void);

private:
    SPIFBlockDevice* _spiDevice;
    /* Main application region manage */
    FlashIAPBlockDevice _main;
    /* Boot application region manage */
    FlashIAPBlockDevice _boot;
    /* Image download region manage */
    FlashSPIBlockDevice _image_download;
    /* Main application rollback region manage */
    FlashSPIBlockDevice _main_rollback;
    /* Boot application rollback region manage */
    FlashSPIBlockDevice _boot_rollback;
};

#endif /* __PARTITON_MANAGER_H */
