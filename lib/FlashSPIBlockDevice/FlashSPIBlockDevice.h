#ifndef __FLASH_SPI_BLOCK_DEVICE_H
#define __FLASH_SPI_BLOCK_DEVICE_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include "SPIFBlockDevice.h"
#include "console_dbg.h"

/* Exported macro ------------------------------------------------------------*/
#define F_SPIBLOCK_PRINTF(...) CONSOLE_LOGI(__VA_ARGS__)
#define F_SPIBLOCK_TAG_PRINTF(...) CONSOLE_TAG_LOGI("[F_SPIBLOCK]", __VA_ARGS__)

class FlashSPIBlockDevice
{
public:
    FlashSPIBlockDevice(SPIFBlockDevice* spiDevice, uint32_t baseAddr, uint32_t size);
    ~FlashSPIBlockDevice();

    int read(void *buffer, uint32_t addr, uint32_t size);
    int program(const void *buffer, uint32_t addr, uint32_t size);
    int erase(uint32_t addr, uint32_t size);
    uint32_t get_read_size(void) const;
    uint32_t get_program_size(void) const;
    uint32_t get_erase_size(void) const;
    uint32_t size(void) const;

private:
    bool is_valid(uint32_t addr, uint32_t size);
    SPIFBlockDevice* _spiDevice;
    uint32_t _baseAddr;
    uint32_t _size;
};


#endif // __FLASH_SPI_BLOCK_DEVICE_H