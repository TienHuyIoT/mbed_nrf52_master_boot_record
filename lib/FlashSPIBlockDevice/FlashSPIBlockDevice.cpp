/* Includes ------------------------------------------------------------------*/
#include "FlashSPIBlockDevice.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

FlashSPIBlockDevice::FlashSPIBlockDevice(SPIFBlockDevice* spiDevice, uint32_t baseAddr, uint32_t size)
: _spiDevice(spiDevice),
_baseAddr(baseAddr),
_size(size) 
{
}

FlashSPIBlockDevice::~FlashSPIBlockDevice()
{
}

/** Read blocks from a block device
 *
 *  @param buffer   Buffer to write blocks to
 *  @param addr     Address of block to begin reading from
 *  @param size     Size to read in bytes, must be a multiple of read block size
 *  @return         SPIF_BD_ERROR_OK(0) - success
 *                  SPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
 */
int FlashSPIBlockDevice::read(void *buffer, uint32_t addr, uint32_t size)
{
    if (is_valid(addr, size))
    {
        addr += _baseAddr;
        return _spiDevice->read(buffer, addr, size);
    }
    return SPIF_BD_ERROR_DEVICE_ERROR;
}

/** Program blocks to a block device
 *
 *  @note The blocks must have been erased prior to being programmed
 *
 *  @param buffer   Buffer of data to write to blocks
 *  @param addr     Address of block to begin writing to
 *  @param size     Size to write in bytes, must be a multiple of program block size
 *  @return         SPIF_BD_ERROR_OK(0) - success
 *                  SPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
 *                  SPIF_BD_ERROR_READY_FAILED - Waiting for Memory ready failed or timed out
 *                  SPIF_BD_ERROR_WREN_FAILED - Write Enable failed
 */
int FlashSPIBlockDevice::program(const void *buffer, uint32_t addr, uint32_t size)
{
    if (is_valid(addr, size))
    {
        addr += _baseAddr;
        return _spiDevice->program(buffer, addr, size);
    }
    return SPIF_BD_ERROR_DEVICE_ERROR;
}

/** Erase blocks on a block device
 *
 *  @note The state of an erased block is undefined until it has been programmed
 *
 *  @param addr     Address of block to begin erasing
 *  @param size     Size to erase in bytes, must be a multiple of erase block size
 *  @return         SPIF_BD_ERROR_OK(0) - success
 *                  SPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
 *                  SPIF_BD_ERROR_READY_FAILED - Waiting for Memory ready failed or timed out
 *                  SPIF_BD_ERROR_INVALID_ERASE_PARAMS - Trying to erase unaligned address or size
 */
int FlashSPIBlockDevice::erase(uint32_t addr, uint32_t size)
{
    if (is_valid(addr, size))
    {
        addr += _baseAddr;
        return _spiDevice->erase(addr, size);
    }
    return SPIF_BD_ERROR_DEVICE_ERROR;
}

/** Get the size of a readable block
 *
 *  @return         Size of a readable block in bytes
 */
uint32_t FlashSPIBlockDevice::get_read_size() const
{
    return _spiDevice->get_read_size();
}

/** Get the size of a programable block
 *
 *  @return         Size of a programable block in bytes
 *  @note Must be a multiple of the read size
 */
uint32_t FlashSPIBlockDevice::get_program_size() const
{
    return _spiDevice->get_program_size();
}

/** Get the size of a eraseable block
 *
 *  @return         Size of a eraseable block in bytes
 *  @note Must be a multiple of the program size
 */
uint32_t FlashSPIBlockDevice::get_erase_size() const
{
    return _spiDevice->get_erase_size();
}

/** Get the total size of the underlying device
 *
 *  @return         Size of the underlying device in bytes
 */
uint32_t FlashSPIBlockDevice::size() const
{
    return _size;
}

/** Convenience function for checking block erase validity
 *
 *  @param addr     Address of block to begin erasing
 *  @param size     Size to erase in bytes
 *  @return         True if addr and size is valid for underlying block device
 */
bool FlashSPIBlockDevice::is_valid(uint32_t addr, uint32_t size)
{
    return ((addr + size) <= _size);
}