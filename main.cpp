#include "mbed.h"
#include "mbr.h"
#include "console_dbg.h"
#define MAIN_TAG_CONSOLE(...) CONSOLE_TAG_LOGI("[MAIN]", __VA_ARGS__)

MasterBootRecord mbr;

int main()
{
    mbr.begin();

    while(1) {};

    MAIN_TAG_CONSOLE("Starting application 0x%0X", POST_APPLICATION_ADDR);
    mbed_start_application(0x10000);
}
