#pragma once

/* console core driver */

// vtable for console functions
struct console_device {
    void (*init)();
    void (*print)(const char *fmt, ...);
    void (*debug)(const char *fmt, ...);
};
