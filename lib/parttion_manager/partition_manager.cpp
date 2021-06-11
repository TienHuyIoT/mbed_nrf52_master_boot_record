/* Includes ------------------------------------------------------------------*/
#include "partition_manager.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

partition_manager::partition_manager(SPIFBlockDevice* spiDevice) :
_spiDevice(spiDevice),
_mbr(),
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
    _mbr.end();
}

void partition_manager::begin(void)
{
    _spiDevice->init();  
    _main.init();
    _boot.init();
    _mbr.begin();
    this->printPartition();
    _mbr.printMbrInfo();
}

void partition_manager::end(void)
{
    _main.deinit();
    _boot.deinit();
    _mbr.end();
}

void partition_manager::printPartition(void)
{
    PARTITION_MNG_TAG_PRINTF("_spiDevice");
    uint32_t spiDevice_size =  _spiDevice->size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", spiDevice_size,
                                readableSize(spiDevice_size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", _spiDevice->get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", _spiDevice->get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", _spiDevice->get_erase_size());
    
    PARTITION_MNG_TAG_PRINTF("_main");
    uint32_t main_size =  _main.size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", main_size,
                                readableSize(main_size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", _main.get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", _main.get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", _main.get_erase_size());

    PARTITION_MNG_TAG_PRINTF("_boot");
    uint32_t boot_size =  _boot.size();
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", boot_size,
                                readableSize(boot_size).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", _boot.get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", _boot.get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", _boot.get_erase_size());

    PARTITION_MNG_TAG_PRINTF("_main_rollback");
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", _main_rollback.size(),
                                readableSize(_main_rollback.size()).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", _main_rollback.get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", _main_rollback.get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", _main_rollback.get_erase_size());

    PARTITION_MNG_TAG_PRINTF("_boot_rollback");
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", _boot_rollback.size(),
                                readableSize(_boot_rollback.size()).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", _boot_rollback.get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", _boot_rollback.get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", _boot_rollback.get_erase_size());

    PARTITION_MNG_TAG_PRINTF("_image_download");
    PARTITION_MNG_TAG_PRINTF("\t size: %llu(%s)", _image_download.size(),
                                readableSize(_image_download.size()).c_str());
    PARTITION_MNG_TAG_PRINTF("\t read size: %llu", _image_download.get_read_size());
    PARTITION_MNG_TAG_PRINTF("\t program size: %llu", _image_download.get_program_size());
    PARTITION_MNG_TAG_PRINTF("\t erase size: %llu\n", _image_download.get_erase_size());

}

bool partition_manager::verifyMain(void) {return true;}
bool partition_manager::verifyBoot(void) {return true;}
bool partition_manager::verifyMainRollback(void) {return true;}
bool partition_manager::verifyBootRollback(void) {return true;}
bool partition_manager::verifyImageDownload(void) {return true;}
bool partition_manager::upgradeMain(void) {return true;}
bool partition_manager::upgradeBoot(void) {return true;}
bool partition_manager::restoreMain(void) {return true;}
bool partition_manager::restoreBoot(void) {return true;}
bool partition_manager::backupMain(void) {return true;}
bool partition_manager::backupBoot(void) {return true;}

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
}