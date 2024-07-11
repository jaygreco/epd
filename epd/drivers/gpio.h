#pragma once

/* GPIO core driver */

enum gpio_mode {
    INPUT = 0,
    OUTPUT
};

enum gpio_level {
    LOW = 0,
    HIGH
};

// vtable for GPIO functions
struct gpio_device {
    void (*init)(uint8_t pin);
    void (*mode)(uint8_t pin, enum gpio_mode mode);
    void (*write)(uint8_t pin, enum gpio_level level);
    uint8_t (*read)(uint8_t pin);
};
