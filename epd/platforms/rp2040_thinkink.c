#include <stdio.h>
#include <stdarg.h>
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "epd/platforms/rp2040_thinkink.h"

#define RP2040_SPI spi0

// GPIO
const struct gpio_device gpio = {
    .init = rp2040_gpio_init,
    .mode = rp2040_gpio_mode,
    .write = rp2040_gpio_write,
    .read = rp2040_gpio_read,
};

void rp2040_gpio_init(uint8_t pin) {
    gpio_init(pin);
}

void rp2040_gpio_mode(uint8_t pin, enum gpio_mode mode) {
    gpio_set_dir(pin, mode);
}

void rp2040_gpio_write(uint8_t pin, enum gpio_level level) {
    gpio_put(pin, level);
}

uint8_t rp2040_gpio_read(uint8_t pin) {
    return (uint8_t) gpio_get(pin);
}

// Console
const struct console_device console = {
    .init = rp2040_console_init,
    .print = rp2040_console_print,
    .debug = rp2040_console_debug,
};

void rp2040_console_init() {
    stdio_init_all();
}

void rp2040_console_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void rp2040_console_debug(const char *fmt, ...) {
    #ifdef DEBUG
    #pragma message "DEBUG BUILD"
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    #endif
}

// Time
const struct time_device time = {
    .sleep_us = rp2040_sleep_us,
    .sleep_ms = rp2040_sleep_ms,
    .millis = rp2040_millis,
};

void rp2040_sleep_us(uint32_t delay) {
    sleep_us(delay);
}

void rp2040_sleep_ms(uint32_t delay) {
    sleep_ms(delay);
}

uint32_t rp2040_millis() {
    return to_ms_since_boot(get_absolute_time());
}

// SPI
const struct spi_device spi = {
    .init = rp2040_spi_init,
    .write = rp2040_spi_write,
};

void rp2040_spi_init(struct spi_config config) {
    spi_init(RP2040_SPI, config.clock_rate);
    gpio_set_function(config.sck, GPIO_FUNC_SPI);
    gpio_set_function(config.mosi, GPIO_FUNC_SPI);

    spi_cpha_t cpha;
    spi_cpol_t cpol;
    
    switch (config.mode) {
    case 1:
        cpha = SPI_CPHA_1;
        cpol = SPI_CPOL_0;
        break;
    case 2:
        cpha = SPI_CPHA_0;
        cpol = SPI_CPOL_1;
        break;
    case 3:
        cpha = SPI_CPHA_1;
        cpol = SPI_CPOL_1;
        break;
    default: //MODE 0
        cpha = SPI_CPHA_0;
        cpol = SPI_CPOL_0;
        break;
    }

    spi_set_format(RP2040_SPI, 8, cpol, cpha, SPI_MSB_FIRST);
}

void rp2040_spi_write(uint8_t* data, uint8_t len) {
    spi_write_blocking(RP2040_SPI, data, len);
}
