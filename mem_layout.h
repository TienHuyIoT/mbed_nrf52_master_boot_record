/*
Internal memory layout

|-------------------|   (0x100000) DEVICE_END_ADDR
|                   |
|    Application    |
| (is main program) |   MAIN_APPLICATION_REGION_SIZE
|       (652K)      |
|                   |
+-------------------+   MAIN_APPLICATION_ADDR
|                   |
|    Application    |
| (is a bootloader) |   BOOTLOADER_FACTORY_REGION_SIZE
|       (300K)      |
|                   |
+-------------------+   BOOTLOADER_FACTORY_ADDR
|      MBR params   |
|        (8K)       |   MASTER_BOOT_PARAMS_REGION_SIZE
+-------------------+   MASTER_BOOT_PARAMS_ADDR
|                   |
|        MBR        |
|       (64K)       |   MASTER_BOOT_RECORD_REGION_SIZE
|                   |
+-------------------+   (0x000000) MASTER_BOOT_RECORD_ADDR


External memory layout

|-------------------|   (0x800000) EX_MEM_END_ADDR
|                   |
|        Data       |
|       (6216K)     |   CHASING_DATA_REGION_SIZE
|                   |
+-------------------+   CHASING_DATA_ADDR
|                   |
|   Image download  |
|        (1M)       |   IMAGE_DOWNLOAD_REGION_SIZE
|                   |
+-------------------+   IMAGE_DOWNLOAD_ADDR
|                   |
|    App rollback   |
| (is main program) |   MAIN_APPLICATION_ROLLBACK_REGION_SIZE
|       (652K)      |
|                   |
+-------------------+   MAIN_APPLICATION_ROLLBACK_ADDR
|                   |
|    App rollback   |
| (is a bootloader) |   BOOTLOADER_ROLLBACK_REGION_SIZE
|       (300K)      |
|                   |
+-------------------+   (0x000000) BOOTLOADER_ROLLBACK_ADDR
*/

#ifndef __MEM_LAYOUT_H
#define __MEM_LAYOUT_H

// <<< Use Configuration Wizard in Context Menu >>>\n

// <h> device memory information

//==========================================================
// <o> DEVICE_PAGE_ERASE_SIZE

#ifndef DEVICE_PAGE_ERASE_SIZE
#define DEVICE_PAGE_ERASE_SIZE 4096U
#endif

// <o> DEVICE_MEMORY_SIZE (1M)

#ifndef DEVICE_MEMORY_SIZE
#define DEVICE_MEMORY_SIZE 0x100000
#endif

// <o> DEVICE_BASE_ADDR

#ifndef DEVICE_BASE_ADDR
#define DEVICE_BASE_ADDR 0U
#endif

// <o> DEVICE_END_ADDR

#ifndef DEVICE_END_ADDR
#define DEVICE_END_ADDR (DEVICE_BASE_ADDR + DEVICE_MEMORY_SIZE)
#endif

// </h>

// <h> Master boot record region

//==========================================================
// <o> MASTER_BOOT_RECORD_REGION_SIZE (64k)

#ifndef MASTER_BOOT_RECORD_REGION_SIZE
#define MASTER_BOOT_RECORD_REGION_SIZE (DEVICE_PAGE_ERASE_SIZE * 16)
#endif

// <o> MASTER_BOOT_RECORD_ADDR

#ifndef MASTER_BOOT_RECORD_ADDR
#define MASTER_BOOT_RECORD_ADDR DEVICE_BASE_ADDR
#endif

// </h>

// <h> Master boot params region

//==========================================================
// <o> MASTER_BOOT_PARAMS_REGION_SIZE (8k)

#ifndef MASTER_BOOT_PARAMS_REGION_SIZE
#define MASTER_BOOT_PARAMS_REGION_SIZE (DEVICE_PAGE_ERASE_SIZE * 2)
#endif

// <o> MASTER_BOOT_PARAMS_ADDR

#ifndef MASTER_BOOT_PARAMS_ADDR
#define MASTER_BOOT_PARAMS_ADDR (MASTER_BOOT_RECORD_ADDR + MASTER_BOOT_RECORD_REGION_SIZE)
#endif

// </h>

// <h> Bootloader factory region

//==========================================================
// <o> BOOTLOADER_FACTORY_REGION_SIZE (300k)

#ifndef BOOTLOADER_FACTORY_REGION_SIZE
#define BOOTLOADER_FACTORY_REGION_SIZE (DEVICE_PAGE_ERASE_SIZE * 75)
#endif

// <o> BOOTLOADER_FACTORY_ADDR

#ifndef BOOTLOADER_FACTORY_ADDR
#define BOOTLOADER_FACTORY_ADDR (MASTER_BOOT_PARAMS_ADDR +  MASTER_BOOT_PARAMS_REGION_SIZE)
#endif

// </h>

// <h> Application region

//==========================================================
// <o> MAIN_APPLICATION_ADDR

#ifndef MAIN_APPLICATION_ADDR
#define MAIN_APPLICATION_ADDR (BOOTLOADER_FACTORY_ADDR + BOOTLOADER_FACTORY_REGION_SIZE)
#endif

// <o> MAIN_APPLICATION_REGION_SIZE (1M - 48K - 8K - 300K = 644K)

#ifndef MAIN_APPLICATION_REGION_SIZE
#define MAIN_APPLICATION_REGION_SIZE (DEVICE_END_ADDR - MAIN_APPLICATION_ADDR)
#endif

// </h>

// <h> External memory information

//==========================================================
// <o> EX_FLASH_PAGE_ERASE_SIZE

#ifndef EX_FLASH_PAGE_ERASE_SIZE
#define EX_FLASH_PAGE_ERASE_SIZE 4096U
#endif

// <o> EX_FLASH_MEMORY_SIZE (8M)

#ifndef EX_FLASH_MEMORY_SIZE
#define EX_FLASH_MEMORY_SIZE 0x800000
#endif

// <o> EX_FLASH_BASE_ADDR

#ifndef EX_FLASH_BASE_ADDR
#define EX_FLASH_BASE_ADDR 0U
#endif

// <o> EX_FLASH_END_ADDR

#ifndef EX_FLASH_END_ADDR
#define EX_FLASH_END_ADDR (EX_FLASH_BASE_ADDR + EX_FLASH_MEMORY_SIZE)
#endif

// </h>

// <h> Bootloader rollback region

//==========================================================
// <o> BOOTLOADER_ROLLBACK_REGION_SIZE (300k)

#ifndef BOOTLOADER_ROLLBACK_REGION_SIZE
#define BOOTLOADER_ROLLBACK_REGION_SIZE BOOTLOADER_FACTORY_REGION_SIZE
#endif

// <o> BOOTLOADER_ROLLBACK_ADDR

#ifndef BOOTLOADER_ROLLBACK_ADDR
#define BOOTLOADER_ROLLBACK_ADDR EX_FLASH_BASE_ADDR
#endif

// </h>

// <h> Application region

//==========================================================
// <o> MAIN_APPLICATION_ROLLBACK_ADDR

#ifndef MAIN_APPLICATION_ROLLBACK_ADDR
#define MAIN_APPLICATION_ROLLBACK_ADDR (BOOTLOADER_ROLLBACK_ADDR + BOOTLOADER_ROLLBACK_REGION_SIZE)
#endif

// <o> MAIN_APPLICATION_ROLLBACK_REGION_SIZE (648K)

#ifndef MAIN_APPLICATION_ROLLBACK_REGION_SIZE
#define MAIN_APPLICATION_ROLLBACK_REGION_SIZE MAIN_APPLICATION_REGION_SIZE
#endif

// </h>

// <h> Image download region

//==========================================================
// <o> IMAGE_DOWNLOAD_ADDR

#ifndef IMAGE_DOWNLOAD_ADDR
#define IMAGE_DOWNLOAD_ADDR (MAIN_APPLICATION_ROLLBACK_ADDR + MAIN_APPLICATION_ROLLBACK_REGION_SIZE)
#endif

// <o> IMAGE_DOWNLOAD_REGION_SIZE (1M)

#ifndef IMAGE_DOWNLOAD_REGION_SIZE
#define IMAGE_DOWNLOAD_REGION_SIZE 0x100000
#endif

// </h>

// <h> Chasing data region

//==========================================================
// <o> CHASING_DATA_ADDR

#ifndef CHASING_DATA_ADDR
#define CHASING_DATA_ADDR (IMAGE_DOWNLOAD_ADDR + IMAGE_DOWNLOAD_REGION_SIZE)
#endif

// <o> CHASING_DATA_REGION_SIZE (8M - 300K - 648K - 1M = 6052K)

#ifndef CHASING_DATA_REGION_SIZE
#define CHASING_DATA_REGION_SIZE (EX_FLASH_END_ADDR - CHASING_DATA_ADDR)
#endif

// </h>

// <<< end of configuration section >>>

#endif /* __MEM_LAYOUT_H */
