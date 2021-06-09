/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MBR_H /* MASTER BOOT RECORD HEADER */
#define __MBR_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include "flash_if.h"
#include "FlashIAPBlockDevice.h"
#include "FlashWearLevellingUtils.h"
#include "mbr_config.h"
#include "console_dbg.h"

/* Exported macro ------------------------------------------------------------*/
#define MBR_PRINTF(...) CONSOLE_LOGI(__VA_ARGS__)
#define MBR_TAG_PRINTF(...) CONSOLE_TAG_LOGI("[MBR]", __VA_ARGS__)

/* Private defines -----------------------------------------------------------*/
#define MBR_KEY_AUTO_CFG_VALUE 0xAA

#define MBR_INFO_DEFAULT {\
  .app_main      = {.startup_addr = 0x8000, .max_size = (0x20000 - 0x8000 - 0x400), .size = (0x20000 - 0x8000 - 0x400), \
                    .type = FW_TYPE, .version = {.major = 0, .minor = 1, .build = 1}},\
  .app_rollback  = {.startup_addr = 0x8000, .max_size = (0x20000 - 0x8000 - 0x400), .size = (0x20000 - 0x8000 - 0x400)},\
  .app_factory   = {.startup_addr = 0x8000, .max_size = (0x20000 - 0x8000 - 0x400), .size = (0x20000 - 0x8000 - 0x400)},\
  .app_uart_boot = {.startup_addr = 0x0   , .max_size = 0x8000,                     .size = 0x8000                    ,\
                    .type = FW_TYPE, .version = {.major = FW_VERSION_MAJOR, .minor = FW_VERSION_MINOR, .build = FW_VERSION_BUILD}},\
  .app_upgrade_num = 0,\
  .hw_version      = HW_VERSION_STRING,\
  .aes_key         = {0x9a, 0x95, 0x0f, 0x6c, 0x4f, 0xa0, 0xf9, 0x19, 0xcb, 0x1e, 0x05, 0x39, 0x56, 0x47, 0x23, 0xe2},\
  .aes_iv          = {0x45, 0xc4, 0x25, 0x0f, 0x8d, 0x78, 0x85, 0xa1, 0xe7, 0x46, 0x94, 0xc7, 0xdd, 0x24, 0x79, 0x83},\
  .common          = {.app_status = 5, .upgrade_mode = 0, .reserve = 0, .key_auto_cfg = MBR_KEY_AUTO_CFG_VALUE}\
}

typedef struct __attribute__((packed, aligned(4)))
{
  uint32_t startup_addr; /* App address startup */
  uint32_t max_size;     /* App size limit */
  uint32_t build_time;   /* Epoch time (option)*/
  uint32_t checksum;     /* App CRC32 checksum verify */
  uint32_t size;         /* App size */
  uint32_t type;         /* App type */
  union {
    uint32_t   u32;
    struct
    {
      uint16_t build;
      uint8_t  minor;
      uint8_t  major;
    };
  } version;           /* App version */
} app_info_t;

/* Size of structure must be multiples write_size-byte for write command */
typedef struct __attribute__((packed, aligned(4)))
{
  app_info_t app_main;           /* main application */
  app_info_t app_rollback;       /* rollback application */
  app_info_t app_factory;        /* factory application */
  app_info_t app_uart_boot;      /* uart bootloader application */
  uint32_t   app_upgrade_num;    /* Number counter upgrade */
  uint8_t    hw_version[16];     /* App Hardware version */
  uint8_t    aes_key[16];        /* AES key encrypt */
  uint8_t    aes_iv[16];         /* AES iv encrypt */
  union {
    uint32_t u32_common;
    struct {
      uint8_t app_status; /* ref app_status_t */
      uint8_t upgrade_mode; /* ref upgrade_mode_t */
      uint8_t reserve;
      uint8_t key_auto_cfg;
    };
  } common;
} mbr_info_t;

class MasterBootRecord
{
  /* Error code */
  typedef enum mbr_status {
    MBR_OK = 0,
    MBR_ERROR
  } mbr_status_t;

  typedef enum
  {
    UPGRADE_MODE_ANY = 0,
    UPGRADE_MODE_UP /* prevent upgrade version lowest than current version */
  } upgrade_mode_t;

  /**
  * @brief  Comm status structures definition
  */
  typedef enum
  {
    APP_READY,     /* App ready */
    APP_CONFIRM,   /* App wating confirm after upgrade */
    APP_DFU_MODE,  /* App enter bootloader mode */
    APP_ROLLBACK,  /* App enter rollback mode */
    APP_FACTORY,   /* App enter factory mode */
    APP_NONE      /* App None */
  } app_status_t;

public:
  MasterBootRecord(/* args */);
  ~MasterBootRecord();
  mbr_status_t begin(void);
  mbr_status_t load(mbr_info_t *mbr);
  mbr_status_t commit(mbr_info_t *mbr);

private:
  /* Register callback handler flash memory */
  class flashIFCallback : public FlashWearLevellingCallbacks
  {
    public:
      flashIFCallback(flashInterface<FlashIAPBlockDevice> *flash_if) :
      _flash_if(flash_if) {}

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
};

#endif  /* __MBR_H */
