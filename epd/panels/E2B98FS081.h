#pragma once

#include "panel.h"

#define PANEL_WIDTH 960UL     // Panel width in px
#define PANEL_HEIGHT 768UL    // Panel height in px
#define PANEL_WIDTH_BYTES ((PANEL_WIDTH / 8))
#define IMAGE_SIZE_BYTES (PANEL_HEIGHT * PANEL_WIDTH_BYTES)

// For graphics layer
#define PANEL_FORMAT_BR_SEQUENTIAL
#define PIXEL_FORMAT_1BPP_MONOCHROME

enum pixel_color {
    BLACK = 0,
    RED,
};

enum spi_target {
    ALL = 0,
    MASTER,
    SLAVE
};

// Panel API
void panel_init();
void panel_reset();
void panel_write(uint8_t* image_data, uint32_t len);
void panel_refresh();
void panel_shutdown();

extern const struct epd_panel panel;

// Internal
static void _spi_transaction_start(enum spi_target target);
static void _spi_transaction_end();
static void _panel_write(enum spi_target target, struct epd_cmd* cmd, size_t num);
static void _cog_init();
static void _dcdc_softstart();
static void _panel_refresh();
static void _send_image(enum spi_target target, enum pixel_color color, uint8_t* image_data, uint32_t len);
static void _dcdc_shutdown();
