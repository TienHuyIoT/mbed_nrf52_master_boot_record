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
}

MasterBootRecord::~MasterBootRecord() {}

MasterBootRecord::mbr_status_t MasterBootRecord::begin(void)
{
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
    return MBR_OK;
}

MasterBootRecord::mbr_status_t MasterBootRecord::load(mbr_info_t *mbr)
{
    MBR_TAG_PRINTF("[_flash_wear_levelling] read");
    if (_flash_wear_levelling.read(mbr))
    {
        MBR_TAG_PRINTF("[load] hw_version: %s", mbr->hw_version);
    }
    else
    {
        mbr_info_t mbr_default = MBR_INFO_DEFAULT;
        MBR_TAG_PRINTF("[load] failure");
        MBR_TAG_PRINTF("[load] Set Default");
        if (!_flash_wear_levelling.write(&mbr_default))
        {
            MBR_TAG_PRINTF("[load] write failed!");
            return MBR_ERROR;
        }
        *mbr = mbr_default;
    }
    return MBR_OK;
}

MasterBootRecord::mbr_status_t MasterBootRecord::commit(mbr_info_t *mbr)
{
    if (!_flash_wear_levelling.write(mbr))
    {
        MBR_TAG_PRINTF("[commit] failed!");
        return MBR_ERROR;
    }
    return MBR_OK;
}
