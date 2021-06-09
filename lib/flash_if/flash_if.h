/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"
#include "FlashIAPBlockDevice.h"
#include "SPIFBlockDevice.h"
#include "console_dbg.h"

/* Exported macro ------------------------------------------------------------*/
#define FLASHIF_PRINTF(...) CONSOLE_LOGI(__VA_ARGS__)
#define FLASHIF_TAG_PRINTF(...) CONSOLE_TAG_LOGI("[FLASHIF]", __VA_ARGS__)

/* Error code */
typedef enum flash_status {
  FLASHIF_OK,
  FLASHIF_INIT_ERROR,
  FLASHIF_ERASE_ERROR,
  FLASHIF_READ_ERROR,
  FLASHIF_WRITE_ERROR
} flash_status_t;

template <class flashType>
class flashInterface
{
  typedef struct flash_properties {
    uint32_t base_addr;
    size_t size;
    size_t read_size;
    size_t write_size;
    size_t erase_size;
  }flash_properties_t;

private:
  flashType *_flash_if;
  flash_properties_t _properties;

public:
  flashInterface(flashType *fType, uint32_t baseAddr = 0) : _flash_if(fType) {
    _properties.base_addr = baseAddr;
  }
  ~flashInterface() {}

  flash_status_t begin(void)
  {
    FLASHIF_TAG_PRINTF("begin");

    /* Initialize the flash IAP block device and print the memory layout */
    if(_flash_if->init())
    {
      FLASHIF_TAG_PRINTF("ERROR");
      return FLASHIF_INIT_ERROR;
    }

    _properties.size = _flash_if->size();
    _properties.read_size = _flash_if->get_read_size();
    _properties.write_size = _flash_if->get_program_size();
    _properties.erase_size = _flash_if->get_erase_size();

    FLASHIF_TAG_PRINTF("device base addr: %llu",  _properties.base_addr);
    FLASHIF_TAG_PRINTF("device size: %llu",  _properties.size);
    FLASHIF_TAG_PRINTF("read size: %llu",    _properties.read_size);
    FLASHIF_TAG_PRINTF("program size: %llu", _properties.write_size);
    FLASHIF_TAG_PRINTF("erase size: %llu",   _properties.erase_size);

    return FLASHIF_OK;
  }

  flash_properties_t properties(void)
  {
    return _properties;
  }

  flash_status_t erase(uint32_t addr_start, size_t lengthInBytes)
  {
    FLASHIF_TAG_PRINTF("Erase addr=%u, length=%u ", addr_start, lengthInBytes);
    if (_properties.read_size > 1)
    {
      if (addr_start % _properties.read_size || lengthInBytes % _properties.read_size)
      {
        FLASHIF_TAG_PRINTF("addr or length failure");
      }
    }

    addr_start -= _properties.base_addr;
    if(_flash_if->erase(addr_start, _properties.erase_size))
    {
      FLASHIF_TAG_PRINTF("ERROR");
      return FLASHIF_ERASE_ERROR;
    }
    FLASHIF_TAG_PRINTF("OK");
    return FLASHIF_OK;
  }

  flash_status_t write(uint32_t addr_start, uint8_t *src, size_t lengthInBytes)
  {
    FLASHIF_TAG_PRINTF("Write addr=%u, length=%u", addr_start, lengthInBytes);

    if (_properties.write_size > 1)
    {
      if (addr_start % _properties.write_size || lengthInBytes % _properties.write_size)
      {
        FLASHIF_TAG_PRINTF("addr or length failure");
      }
    }

    addr_start -= _properties.base_addr;
    if(_flash_if->program(src, addr_start, lengthInBytes))
    {
      FLASHIF_TAG_PRINTF("ERROR");
      return FLASHIF_WRITE_ERROR;
    }
    FLASHIF_TAG_PRINTF("OK");
    return FLASHIF_OK;
  }

  flash_status_t read(uint32_t addr_start, uint8_t *src, size_t lengthInBytes)
  {
    FLASHIF_TAG_PRINTF("Read addr=%u, length=%u", addr_start, lengthInBytes);
    addr_start -= _properties.base_addr;
    if(_flash_if->read(src, addr_start, lengthInBytes))
    {
      FLASHIF_TAG_PRINTF("ERROR");
      return FLASHIF_READ_ERROR;
    }
    FLASHIF_TAG_PRINTF("OK");
    return FLASHIF_OK;
  }
};

#endif  /* __FLASH_IF_H */
