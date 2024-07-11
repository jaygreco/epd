#pragma once

/* time core driver */

// vtable for time functions
struct time_device {
    void (*sleep_us)(uint32_t delay);
    void (*sleep_ms)(uint32_t delay);
    uint32_t (*millis)();
};
