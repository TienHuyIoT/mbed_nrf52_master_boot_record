## mBed-OS Nrf52840 Master Boot Project
### Features
- Master boot record
    - Params using FlashWearLevelling library to manage data.
    - Params is stored to internal memory.
- Partition startup manager
    - Verify application.
    - Upgrade application.
    - Backup application.
    - Restore application.
    - Clone application.
    - AES encrypt image stored external memory.
    - CRC32 image application internal and external memory.
### Library
- [AES](https://os.mbed.com/users/neilt6/code/AES/docs/tip/classAES.html) - C++
- [Segger RTT](https://os.mbed.com/users/GlimwormBeacons/code/SEGGER_RTT/) - Console Log using J-Link.
- FlashWearLevelling
### Production Tools
- [Tools generate dfu image and release image](https://github.com/TienHuyIoT/py_tool_for_master_boot_record)
#### Project configure
- mbed_app.json
```sh
{
    "target_overrides": {
        "NRF52840_DK": {
            "target.components_add": ["SPIF", "FLASHIAP"],
            "target.restrict_size": "0x14000",
            "platform.stdio-baud-rate": 115200,
            "target.console-uart": false
        }
    }
}
```
#### Memory layout
- mem_layout.h
```sh
Internal memory layout

|-------------------|   (0x100000) DEVICE_END_ADDR
|                   |
|    Application    |
| (is main program) |   MAIN_APPLICATION_REGION_SIZE
|       (636K)      |
|                   |
+-------------------+   (0x61000)MAIN_APPLICATION_ADDR
|                   |
|    Application    |
| (is a bootloader) |   BOOTLOADER_FACTORY_REGION_SIZE
|       (300K)      |
|                   |
+-------------------+   (0x16000)BOOTLOADER_FACTORY_ADDR
|                   |   (0x15FE0)MAIN_APP_HEADER_GENERAL_LOCATION
|                   |   (0x15FC0)BOOT_APP_HEADER_GENERAL_LOCATION
|      MBR params   |
|        (8K)       |   MASTER_BOOT_PARAMS_REGION_SIZE
|                   |
+-------------------+   (0x14000)MASTER_BOOT_PARAMS_ADDR
|                   |
|        MBR        |
|       (80K)       |   MASTER_BOOT_RECORD_REGION_SIZE
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
+-------------------+   (0xEA000)IMAGE_DOWNLOAD_ADDR
|                   |
|    App rollback   |
| (is main program) |   MAIN_APPLICATION_ROLLBACK_REGION_SIZE
|       (636K)      |
|                   |
+-------------------+   (0x4B000)MAIN_APPLICATION_ROLLBACK_ADDR
|                   |
|    App rollback   |
| (is a bootloader) |   BOOTLOADER_ROLLBACK_REGION_SIZE
|       (300K)      |
|                   |
+-------------------+   (0x000000) BOOTLOADER_ROLLBACK_ADDR
```