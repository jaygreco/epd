# pragma once

/* SPI core driver */

// SPI config for HW init
struct spi_config {
    uint8_t sck;
    uint8_t miso;
    uint8_t mosi;
    uint8_t cs;
    uint32_t clock_rate;
    uint8_t bit_order;
    uint8_t mode;
};

// vtable for SPI functions
struct spi_device {
    void (*init)(struct spi_config config);
    void (*deinit)(struct spi_config config);
    void (*write)(uint8_t* data, uint8_t len);
    uint8_t (*read)(uint8_t* buf, uint8_t len);
};
