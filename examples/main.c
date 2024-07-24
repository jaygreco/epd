#include <stdio.h>
#include <string.h>
#include "epd/platforms/platform.h"
#include "epd/panels/E2B98FS081.h"
#include "image_data.h"

int main() {
    console.init();
    time.sleep_ms(2500);

    gpio.init(PIN_LED);
    gpio.mode(PIN_LED, OUTPUT);

    uint8_t image_buffer[2*IMAGE_SIZE_BYTES] = {0};
    memcpy(image_buffer, image_data, 2*IMAGE_SIZE_BYTES);

    panel.init();
    panel.write(image_buffer, 2*IMAGE_SIZE_BYTES);
    panel.refresh();
    panel.shutdown();

    while (true) {
        gpio.write(PIN_LED, 1);
        time.sleep_ms(1000);
        gpio.write(PIN_LED, 0);
        time.sleep_ms(1000);
    }
}
