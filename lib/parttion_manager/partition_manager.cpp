/* Includes ------------------------------------------------------------------*/
#include "partition_manager.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

partition_manager::partition_manager(SPIFBlockDevice* spiDevice) :
_spiDevice(spiDevice),
_main(MAIN_APPLICATION_ADDR, MAIN_APPLICATION_REGION_SIZE),
_boot(BOOTLOADER_FACTORY_ADDR, BOOTLOADER_FACTORY_REGION_SIZE),
_image_download(spiDevice, IMAGE_DOWNLOAD_ADDR, IMAGE_DOWNLOAD_REGION_SIZE),
_main_rollback(spiDevice, MAIN_APPLICATION_ROLLBACK_ADDR, MAIN_APPLICATION_ROLLBACK_REGION_SIZE),
_boot_rollback(spiDevice, BOOTLOADER_ROLLBACK_ADDR, BOOTLOADER_ROLLBACK_REGION_SIZE)
{
}

partition_manager::~partition_manager()
{
    _main.deinit();
    _boot.deinit();
}

void partition_manager::begin(void)
{
    _spiDevice->init();
    PARTITION_MNG_TAG_PRINTF("_spiDevice size: %llu\n", _spiDevice->size());
    PARTITION_MNG_TAG_PRINTF("_spiDevice read size: %llu\n", _spiDevice->get_read_size());
    PARTITION_MNG_TAG_PRINTF("_spiDevice program size: %llu\n", _spiDevice->get_program_size());
    PARTITION_MNG_TAG_PRINTF("_spiDevice erase size: %llu\n", _spiDevice->get_erase_size());
    
    _main.init();
    PARTITION_MNG_TAG_PRINTF("_main size: %llu\n", _main.size());
    PARTITION_MNG_TAG_PRINTF("_main read size: %llu\n", _main.get_read_size());
    PARTITION_MNG_TAG_PRINTF("_main program size: %llu\n", _main.get_program_size());
    PARTITION_MNG_TAG_PRINTF("_main erase size: %llu\n", _main.get_erase_size());

    _boot.init();
    PARTITION_MNG_TAG_PRINTF("_boot size: %llu\n", _boot.size());
    PARTITION_MNG_TAG_PRINTF("_boot read size: %llu\n", _boot.get_read_size());
    PARTITION_MNG_TAG_PRINTF("_boot program size: %llu\n", _boot.get_program_size());
    PARTITION_MNG_TAG_PRINTF("_boot erase size: %llu\n", _boot.get_erase_size());
}

void partition_manager::end(void)
{
    _main.deinit();
    _boot.deinit();
}
