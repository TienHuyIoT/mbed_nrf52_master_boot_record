#include "mbed.h"
#include "SPIFBlockDevice.h"
#include "pinmap_ex.h"
#include <SPI.h>
#include "mem_layout.h"
#include "partition_manager.h"
#include "console_dbg.h"

/* Private define ------------------------------------------------------------*/
#define FSPI_MOSI_PIN p6
#define FSPI_MISO_PIN p5
#define FSPI_CLK_PIN p7
#define FSPI_CS_PIN p8

#define KX022_CS_PIN p11

/* Private macro -------------------------------------------------------------*/
#define MAIN_TAG_CONSOLE(...) CONSOLE_TAG_LOGI("[MAIN]", __VA_ARGS__)
#define MAIN_CONSOLE(...) CONSOLE_LOGI(__VA_ARGS__)

/* PinMap SPI0 */
const PinMapSPI PinMap_SPI[1] = {
    {FSPI_MOSI_PIN, FSPI_MISO_PIN, FSPI_CLK_PIN, 0}};
/* Init SPI Block Device */
SPIFBlockDevice spiFlashDevice(FSPI_MOSI_PIN, FSPI_MISO_PIN, FSPI_CLK_PIN, FSPI_CS_PIN);
partition_manager partition_mng(&spiFlashDevice);

DigitalOut kx022_cs(KX022_CS_PIN);

static uint32_t startup_application(void);

int main()
{
    MAIN_CONSOLE("\r\n\r\n");
    MAIN_TAG_CONSOLE("===================================================");
    kx022_cs = 1; /* unselect spi bus kx022 */

    // partition_mng.begin();
    // partition_mng.verifyMain();
    // partition_mng.backupMain();
    // partition_mng.verifyMainRollback();
    // partition_mng.restoreMain();
    // partition_mng.end();
    // while(1) {};

    uint32_t jump_address = startup_application();
    if (jump_address != 0)
    {
        MAIN_TAG_CONSOLE("Starting application 0x%0X", jump_address);
        mbed_start_application(jump_address);
    }
    else
    {
        while(1)
        {
            MAIN_TAG_CONSOLE("None application");
            ThisThread::sleep_for(10000ms);
        }
    }
}

static uint32_t startup_application(void)
{
    MasterBootRecord::startup_mode_t startupMode;
    uint32_t jump_address;

    partition_mng.begin();

    startupMode = partition_mng.getStartUpModeFromMBR();
    MAIN_TAG_CONSOLE("Startup Mode %u", startupMode);
    switch (startupMode)
    {
    case MasterBootRecord::UPGRADE_MODE:
        MAIN_TAG_CONSOLE("UPGRADE_MODE");
        if (partition_mng.verifyImageDownload())
        {
            MasterBootRecord::header_application_t typeApp;
            typeApp = (MasterBootRecord::header_application_t)partition_mng.appUpgrade();
            jump_address = partition_mng.appAddress(); /* Default jump address */
            if (MasterBootRecord::MAIN_APPLICATION == typeApp)
            {
                /* Backup main partition to external flash */
                MAIN_TAG_CONSOLE("BACKUP_MAIN");
                partition_mng.backupMain();
                MAIN_TAG_CONSOLE("UPGRADE_MAIN");
                if (partition_mng.upgradeMain())
                {
                    if (partition_mng.setStartUpModeToMBR(MasterBootRecord::MAIN_RUN_MODE))
                    {
                        jump_address = partition_mng.appAddress();
                    }
                }
                else
                {
                    MAIN_TAG_CONSOLE("UPGRADE_MAIN ERROR");
                }
            }
            else if (MasterBootRecord::BOOT_APPLICATION == typeApp)
            {
                /* Backup boot partition to external flash */
                MAIN_TAG_CONSOLE("BACKUP_BOOT");
                partition_mng.backupBoot();
                MAIN_TAG_CONSOLE("UPGRADE_BOOT");
                if (partition_mng.upgradeBoot())
                {
                    if (partition_mng.setStartUpModeToMBR(MasterBootRecord::BOOT_RUN_MODE))
                    {
                        jump_address = partition_mng.bootAddress();
                    }
                }
                else
                {
                    MAIN_TAG_CONSOLE("UPGRADE_BOOT ERROR");
                }
            }
            else
            {
                /* todo */
            }
            break;
        }
        else
        {
            MAIN_TAG_CONSOLE("UPGRADE_MODE ERROR");
        }
        // break;
        /* if the upgrade failure, then main run */
    case MasterBootRecord::MAIN_RUN_MODE:
        MAIN_TAG_CONSOLE("MAIN_RUN_MODE");
        if (partition_mng.verifyMain())
        {
            jump_address = partition_mng.appAddress();
            break;
        }
        else
        {
            MAIN_TAG_CONSOLE("MAIN_RUN_MODE ERROR");
        }
        // break;
        /* if the main application failure, then rollback main application */
    case MasterBootRecord::MAIN_ROLLBACK_MODE:
        MAIN_TAG_CONSOLE("MAIN_ROLLBACK_MODE");
        if (partition_mng.restoreMain())
        {
            jump_address = partition_mng.appAddress();
            break;
        }
        else
        {
            MAIN_TAG_CONSOLE("MAIN_ROLLBACK_MODE ERROR");
        }
        // break;
        /* if the main rollback failure, then bootloader run */
    case MasterBootRecord::BOOT_RUN_MODE:
        MAIN_TAG_CONSOLE("BOOT_RUN_MODE");
        if (partition_mng.verifyBoot())
        {
            jump_address = partition_mng.bootAddress();
            break;
        }
        else
        {
            MAIN_TAG_CONSOLE("BOOT_RUN_MODE ERROR");
        }
        // break;
        /* if the boot application failure, then rollback boot application */
    case MasterBootRecord::BOOT_ROLLBACK_MODE:
        MAIN_TAG_CONSOLE("BOOT_ROLLBACK_MODE");
        if (partition_mng.restoreBoot())
        {
            jump_address = partition_mng.bootAddress();
            break;
        }
        else
        {
            MAIN_TAG_CONSOLE("BOOT_ROLLBACK_MODE ERROR");
        }
        // break;
    
    default:
        jump_address = 0;
        break;
    }

    if (partition_mng.appAddress() == jump_address)
    {
        if (MasterBootRecord::MAIN_RUN_MODE != startupMode)
        {
            partition_mng.setStartUpModeToMBR(MasterBootRecord::MAIN_RUN_MODE);
        }
        MAIN_TAG_CONSOLE("main application 0x%0X", jump_address);
    }

    if (partition_mng.bootAddress() == jump_address)
    {
        if (MasterBootRecord::BOOT_RUN_MODE != startupMode)
        {
            partition_mng.setStartUpModeToMBR(MasterBootRecord::BOOT_RUN_MODE);
        }
        MAIN_TAG_CONSOLE("Bootloader application 0x%0X", jump_address);
    }

    partition_mng.end();

    return jump_address;
}