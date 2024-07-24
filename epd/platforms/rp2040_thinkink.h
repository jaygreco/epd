#pragma once

#include "../drivers/console.h"
#include "../drivers/gpio.h"
#include "../drivers/spi.h"
#include "../drivers/time.h"
#include "pico/stdlib.h"

/* BSP for RP2040 Thinkink using RP2040 SDK */

// Pin definitions
#define PIN_LED PICO_DEFAULT_LED_PIN
#define PIN_PANEL_BUSY 16
#define PIN_PANEL_DC 18
#define PIN_PANEL_RESET 17
#define PIN_PANEL_CS_M 19
#define PIN_PANEL_CS_S 5
#define PIN_PANEL_SCK 22
#define PIN_PANEL_MOSI 23

// GPIO
void rp2040_gpio_init(uint8_t pin);
void rp2040_gpio_mode(uint8_t pin, enum gpio_mode);
void rp2040_gpio_write(uint8_t pin, enum gpio_level level);
uint8_t rp2040_gpio_read(uint8_t pin);

extern const struct gpio_device gpio;

// Console
void rp2040_console_init();
void rp2040_console_print(const char *fmt, ...);
void rp2040_console_debug(const char *fmt, ...);

extern const struct console_device console;

// Time
void rp2040_sleep_us(uint32_t delay);
void rp2040_sleep_ms(uint32_t delay);
uint32_t rp2040_millis();

extern const struct time_device time;

// SPI
void rp2040_spi_init(struct spi_config config);
void rp2040_spi_write(uint8_t* data, uint8_t len);

extern const struct spi_device spi;

// General init
void rp2040_init();
