#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

struct epd_cmd {
    uint8_t reg;
    uint8_t data;
    uint16_t wait_ms : 15;
    bool chain_previous : 1;
};

#define NUM(c) (sizeof(c)/sizeof(*c))

struct epd_panel {
    void (*init)();
    void (*write)();
    void (*refresh)();
    void (*reset)();
    void (*shutdown)();
};
