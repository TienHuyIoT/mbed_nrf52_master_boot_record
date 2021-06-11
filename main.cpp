#include "mbed.h"
#include "SPIFBlockDevice.h"
#include "pinmap_ex.h"
#include <SPI.h>
#include "mem_layout.h"
#include "partition_manager.h"
#include "console_dbg.h"

/* Private define ------------------------------------------------------------*/
#define FSPI_MOSI_PIN p6
#define FSPI_MISO_PIN p5
#define FSPI_CLK_PIN p7
#define FSPI_CS_PIN p8

#define KX022_CS_PIN p11

/* Private macro -------------------------------------------------------------*/
#define MAIN_TAG_CONSOLE(...) CONSOLE_TAG_LOGI("[MAIN]", __VA_ARGS__)
#define MAIN_CONSOLE(...) CONSOLE_LOGI(__VA_ARGS__)

/* PinMap SPI0 */
const PinMapSPI PinMap_SPI[1] = {
    {FSPI_MOSI_PIN, FSPI_MISO_PIN, FSPI_CLK_PIN, 0}};
/* Init SPI Block Device */
SPIFBlockDevice spiFlash(FSPI_MOSI_PIN, FSPI_MISO_PIN, FSPI_CLK_PIN, FSPI_CS_PIN);
partition_manager partition_mng(&spiFlash);

DigitalOut kx022_cs(KX022_CS_PIN);

int main()
{
    MAIN_CONSOLE("\r\n\r\n");
    MAIN_TAG_CONSOLE("===================================================");

    kx022_cs = 1; /* unselect spi bus kx022 */

    partition_mng.begin();
    // partition_mng.verifyMain();
    // partition_mng.backupMain();
    // partition_mng.verifyMainRollback();
    partition_mng.restoreMain();
    partition_mng.end();
    // while(1) {};

    MAIN_TAG_CONSOLE("Starting application 0x%0X", MAIN_APPLICATION_ADDR);
    mbed_start_application(MAIN_APPLICATION_ADDR);
}
