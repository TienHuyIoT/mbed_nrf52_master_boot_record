/** @file mbr.h
 *  @brief Master boot record using FlashWearLevellingUtils lib to manager
 *          data header and application partition
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
#ifndef __MBR_H /* MASTER BOOT RECORD HEADER */
#define __MBR_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include <string>
#include "flash_if.h"
#include "FlashIAPBlockDevice.h"
#include "FlashWearLevellingUtils.h"
#include "mbr_config.h"
#include "mem_layout.h"
#include "console_dbg.h"

/* Exported macro ------------------------------------------------------------*/
#define MBR_PRINTF(...) CONSOLE_LOGI(__VA_ARGS__)
#define MBR_TAG_PRINTF(...) CONSOLE_TAG_LOGI("[MBR]", __VA_ARGS__)

/* Private defines -----------------------------------------------------------*/
#define MBR_STARTUP_MODE MAIN_RUN_MODE
#define MBR_DFU_MODE UPGRADE_MODE_ANY

#define HARDWARE_VERSION_LENGTH_MAX 16
#define AES128_LENGTH 16

#define MBR_CRC_APP_FACTORY 0x00000000 /* the application doesn't need to verify CRC */
#define MBR_CRC_APP_NONE 0xFFFFFFFF    /* the application isn't exist */
#define FIRMWARE_TYPE_SIGNAL 0x55

/* 55(signal)-01(app)-00(raw image)-00(internal) */
#define FW_APP_MAIN_TYPE 0x55010000
/* 55(signal)-01(app)-01(encrypt image)-01(external) */
#define FW_APP_ROLLBACK_TYPE 0x55010101
/* 55(signal)-00(boot)-00(raw image)-00(internal) */
#define FW_BOOT_TYPE 0x55000000
/* 55(signal)-00(boot)-01(encrypt image)-01(external) */
#define FW_BOOT_ROLLBACK_TYPE 0x55000101
/* 55(signal)-00/01(boot/app)-01(encrypt image)-01(external) */
#define FW_IMAGE_DOWNLOAD_TYPE 0x55010101

/** Address main header and boot header general using for partition manager
 * The first bootup, the MBR have checksum default for main header and boot header
 * Partition manager (PM) should be check app header and boot header at this specific address.
 * If main header or boot header true. PM will update new header for main or boot into MBR.
 */
#define MAIN_APP_HEADER_GENERAL_LOCATION 0x15FE0
#define BOOT_APP_HEADER_GENERAL_LOCATION 0x15FC0

#define MBR_INFO_DEFAULT                                                              \
    {                                                                                 \
        .main_app = {.startup_addr = MAIN_APPLICATION_ADDR,                           \
                     .max_size = MAIN_APPLICATION_REGION_SIZE,                        \
                     .fw_header = {.checksum = MBR_CRC_APP_FACTORY,                   \
                                   .size = MAIN_APPLICATION_REGION_SIZE,              \
                                   .type = {.u32 = FW_APP_MAIN_TYPE},                 \
                                   .version = {.u32 = 0x01010001 /*v1.1.1*/}}},       \
        .main_rollback = {.startup_addr = MAIN_APPLICATION_ROLLBACK_ADDR,             \
                          .max_size = MAIN_APPLICATION_ROLLBACK_REGION_SIZE,          \
                          .fw_header = {.checksum = MBR_CRC_APP_NONE,                 \
                                        .size = 0,                                    \
                                        .type = {.u32 = FW_APP_ROLLBACK_TYPE},        \
                                        .version = {.u32 = 0x00000000 /*v0.0.0*/}}},  \
        .boot_app = {.startup_addr = BOOTLOADER_FACTORY_ADDR,                         \
                     .max_size = BOOTLOADER_FACTORY_REGION_SIZE,                      \
                     .fw_header = {.checksum = MBR_CRC_APP_FACTORY,                   \
                                   .size = BOOTLOADER_FACTORY_REGION_SIZE,            \
                                   .type = {.u32 = FW_BOOT_TYPE},                     \
                                   .version = {.u32 = 0x01010001 /*v1.1.1*/}}},       \
        .boot_rollback = {.startup_addr = BOOTLOADER_ROLLBACK_ADDR,                   \
                          .max_size = BOOTLOADER_ROLLBACK_REGION_SIZE,                \
                          .fw_header = {.checksum = MBR_CRC_APP_NONE,                 \
                                        .size = 0,                                    \
                                        .type = {.u32 = FW_BOOT_ROLLBACK_TYPE},       \
                                        .version = {.u32 = 0x00000000 /*v0.0.0*/}}},  \
        .image_download = {.startup_addr = IMAGE_DOWNLOAD_ADDR,                       \
                           .max_size = IMAGE_DOWNLOAD_REGION_SIZE,                    \
                           .fw_header = {.checksum = MBR_CRC_APP_NONE,                \
                                         .size = 0,                                   \
                                         .type = {.u32 = FW_IMAGE_DOWNLOAD_TYPE},     \
                                         .version = {.u32 = 0x00000000 /*v0.0.0*/}}}, \
        .dfu_num = {.main = 0, .boot = 0},                                            \
        .hw_version_str = HW_VERSION_STRING,                                          \
        .aes = {.key = {0x9a, 0x95, 0x0f, 0x6c, 0x4f, 0xa1, 0xf9, 0x19,               \
                        0xcb, 0x1e, 0x15, 0x39, 0x56, 0x47, 0x23, 0xe2},              \
                .iv = {0x45, 0xc4, 0x25, 0x0f, 0x8d, 0x79, 0x85, 0xa1,                \
                       0xe7, 0x46, 0x92, 0xc7, 0xdd, 0x24, 0x79, 0x83}},              \
    }

typedef struct __attribute__((packed, aligned(4)))
{
    uint32_t checksum; /* Firmware CRC32 checksum verify */
    uint32_t size;     /* Firmware size */
    union
    {
        uint32_t u32;
        struct
        {
            uint8_t mem;    /* refer header_memory_t
                            0. internal
                            1. external;
                            */
            uint8_t enc;    /* refer header_encrypt_t
                            0. image raw; 
                            1. image encrypt;
                            2. Header + image raw (image download option);
                            3. (Header + image raw) encrypt (image download option);
                            */
            uint8_t app;    /* refer header_application_t 
                            0. Boot
                            1. App;
                            */
            uint8_t signal; /* alway 0x55 */
        };
    } type; /* Firmware type */
    union
    {
        uint32_t u32;
        struct
        {
            uint16_t build;
            uint8_t minor;
            uint8_t major;
        };
    } version; /* Firmware version */
} firmwareHeader_t;

typedef struct __attribute__((packed, aligned(4)))
{
    uint32_t startup_addr; /* App address startup */
    uint32_t max_size;     /* App size limit */
    firmwareHeader_t fw_header;
    union
    {
        uint32_t u32;
        struct
        {
            uint8_t app_status; /* ref app_status_t */
            uint8_t NI1;
            uint8_t NI2;
            uint8_t NI3;
        };
    } common;
} app_info_t;

typedef struct
{
    uint8_t key[AES128_LENGTH]; /* AES key encrypt */
    uint8_t iv[AES128_LENGTH];  /* AES iv encrypt */
} AES128_crypto_t;

/* Size of structure must be multiples write_size-byte for write command */
typedef struct __attribute__((packed, aligned(4)))
{
    app_info_t main_app;       /* main application */
    app_info_t main_rollback;  /* rollback application */
    app_info_t boot_app;       /* ble bootloader application */
    app_info_t boot_rollback;  /* ble bootloader application */
    app_info_t image_download; /* image download application */
    union
    {
        uint32_t u32;
        struct
        {
            uint16_t main;
            uint16_t boot;
        };
    } dfu_num;                                        /* Number counter upgrade*/
    char hw_version_str[HARDWARE_VERSION_LENGTH_MAX]; /* App Hardware version */
    AES128_crypto_t aes;
    union
    {
        uint32_t u32;
        struct
        {
            uint8_t startup_mode; /* ref startup_mode_t */
            uint8_t dfu_mode;     /* ref dfu_mode_t */
        };
    } common;
} mbr_info_t;

class MasterBootRecord
{
public:
    /* Error code */
    typedef enum mbr_status
    {
        MBR_OK = 0,
        MBR_ERROR
    } mbr_status_t;

    typedef enum
    {
        UPGRADE_MODE_ANY = 0,
        UPGRADE_MODE_UP /* prevent upgrade version lowest than current version */
    } dfu_mode_t;

    typedef enum
    {
        MEMORY_INTERNAL = 0,
        MEMORY_EXTERNAL
    } header_memory_t;

    typedef enum
    {
        BOOT_APPLICATION = 0,
        MAIN_APPLICATION
    } header_application_t;

    typedef enum
    {
        DATA_RAW = 0,
        DATA_ENC,
        DATA_HEADER_AND_RAW, /* reserve */
        DATA_HEADER_AND_ENC  /* reserve */
    } header_encrypt_t;

    typedef enum
    {
        APP_STATUS_NONE = 0,
        APP_STATUS_OK,
        APP_STATUS_WAIT_CONFIRM,
        APP_STATUS_ERROR
    } app_status_t;

    /**
     * @brief  Comm status structures definition
     */
    typedef enum
    {
        MAIN_RUN_MODE,      /* 0. App run */
        MAIN_ROLLBACK_MODE, /* 1. App rollback */
        BOOT_RUN_MODE,      /* 2. boot run */
        BOOT_ROLLBACK_MODE, /* 3. boot rollback */
        UPGRADE_MODE,       /* 4. New update */
        NO_APP_MODE         /* 5. App None */
    } startup_mode_t;

public:
    MasterBootRecord();
    ~MasterBootRecord();

    mbr_status_t begin(void);
    mbr_status_t load(void);
    mbr_status_t setDefault(void);
    mbr_status_t commit(void);
    void end(void);
    void printMbrInfo(void);
    void printAppInfo(app_info_t *pParams);
    void printFwHeaderInfo(firmwareHeader_t *fwHeader);

    app_info_t getMainParams(void);
    app_info_t getBootParams(void);
    app_info_t getMainRollbackParams(void);
    app_info_t getBootRollbackParams(void);
    app_info_t getImageDownloadParams(void);
    AES128_crypto_t getAes128Params(void);
    dfu_mode_t getDfuMode(void);
    startup_mode_t getStartUpMode(void);
    app_status_t getMainStatus(void);
    app_status_t getBootStatus(void);
    uint16_t getMainDfuNum(void);
    uint16_t getBootDfuNum(void);
    std::string getHardwareVersion(void);

    void setMainParams(app_info_t *pParams);
    void setBootParams(app_info_t *pParams);
    void setMainRollbackParams(app_info_t *pParams);
    void setBootRollbackParams(app_info_t *pParams);
    void setImageDownloadParams(app_info_t *pParams);
    void setAes128Params(AES128_crypto_t *pParams);
    void setDfuMode(dfu_mode_t mode);
    void setStartUpMode(startup_mode_t mode);
    void setMainStatus(app_status_t status);
    void setBootStatus(app_status_t status);
    void setHardwareVersion(std::string const &hwName);
    void setMainDfuNum(uint16_t num);
    void setBootDfuNum(uint16_t num);

private:
    /* Register callback handler flash memory */
    class flashIFCallback : public FlashWearLevellingCallbacks
    {
    public:
        flashIFCallback(flashInterface<FlashIAPBlockDevice> *flash_if) : _flash_if(flash_if) {}

        bool onRead(uint32_t addr, uint8_t *buff, uint16_t *length)
        {
            // MBR_TAG_PRINTF("[flashIFCallback][onRead] onRead [addr][length]: [%u(0x%x)][%u]",
            //                  addr, addr, *length);
            return (_flash_if->read(addr, buff, *length) == flash_status_t::FLASHIF_OK);
        }

        bool onWrite(uint32_t addr, uint8_t *buff, uint16_t *length)
        {
            // MBR_TAG_PRINTF("[flashIFCallback][onWrite] onWrite [addr][length]: [%u(0x%x)][%u]",
            //                  addr, addr, *length);
            return (_flash_if->write(addr, buff, *length) == flash_status_t::FLASHIF_OK);
        }

        bool onErase(uint32_t addr, uint16_t length)
        {
            MBR_TAG_PRINTF("[flashIFCallback][onErase] addr=%u(0x%x), length=%u", addr, addr,
                           length);
            return (_flash_if->erase(addr, length) == flash_status_t::FLASHIF_OK);
        }

        bool onReady()
        {
            MBR_TAG_PRINTF("[flashIFCallback][onReady]");
            return true;
        }

        void onStatus(FlashWearLevellingCallbacks::status_t err_code)
        {
            MBR_TAG_PRINTF("[flashIFCallback][onStatus] %s",
                           reportStr(err_code).c_str());
        }

    private:
        flashInterface<FlashIAPBlockDevice> *_flash_if;
    };

private:
    flashInterface<FlashIAPBlockDevice> _flash_internal_handler;
    FlashWearLevellingUtils _flash_wear_levelling;
    FlashIAPBlockDevice _flash_iap_block_device;
    flashIFCallback _fp_callback;
    mbr_info_t _mbr_info;
    bool _init_isOK;

    std::string readableSize(float bytes);
};

#endif /* __MBR_H */
