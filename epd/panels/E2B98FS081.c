#include "E2B98FS081.h"
#include "epd/platforms/platform.h"


// Panel API
const struct epd_panel panel = {
    .init = panel_init,
    .write = panel_write,
    .refresh = panel_refresh,
    .reset = panel_reset,
    .shutdown = panel_shutdown,
};

// External
// Initialize the panel after a reset
void panel_init() {
    console.debug("panel init: %lu\n", time.millis());
    // Pin init
    gpio.init(PIN_PANEL_BUSY);
    gpio.mode(PIN_PANEL_BUSY, INPUT);

    gpio.init(PIN_PANEL_DC);
    gpio.mode(PIN_PANEL_DC, OUTPUT);
    gpio.write(PIN_PANEL_DC, HIGH);

    gpio.init(PIN_PANEL_RESET);
    gpio.mode(PIN_PANEL_RESET, OUTPUT);
    gpio.write(PIN_PANEL_RESET, HIGH);

    gpio.init(PIN_PANEL_CS_M);
    gpio.mode(PIN_PANEL_CS_M, OUTPUT);
    gpio.write(PIN_PANEL_CS_M, HIGH);

    gpio.init(PIN_PANEL_CS_S);
    gpio.mode(PIN_PANEL_CS_S, OUTPUT);
    gpio.write(PIN_PANEL_CS_S, HIGH);


    // SPI init
    struct spi_config config = {
        .sck = PIN_PANEL_SCK,
        .mosi = PIN_PANEL_MOSI,
        .clock_rate = 8000000,
    };
    spi.init(config);

    panel_reset();

    // Wait until complete
    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);
}

// Reset the panel with the HW reset sequence
void panel_reset() {
    console.debug("panel reset: %lu\n", time.millis());

    uint32_t delays[] = {200, 20, 200, 200};

    time.sleep_ms(delays[0]);
    gpio.write(PIN_PANEL_RESET, HIGH);
    time.sleep_ms(delays[1]);
    gpio.write(PIN_PANEL_RESET, LOW);
    time.sleep_ms(delays[2]);
    gpio.write(PIN_PANEL_RESET, HIGH);
    time.sleep_ms(delays[3]);
}

// Write image data to the on-chip RAM
void panel_write(uint8_t* image_data, uint32_t len) {
    // Prep chip-on-glass controllers for transfers
    struct epd_cmd duw_cmd[] = {
        {0x13, 0x00, 0, false}, //DUW for Both Master and Slave
        {0xff, 0x3b, 0, true},
        {0xff, 0x00, 0, true},
        {0xff, 0x00, 0, true},
        {0xff, 0xff, 0, true},
        {0xff, 0x02, 0, true},
    };
    _panel_write(ALL, duw_cmd, NUM(duw_cmd));

    struct epd_cmd drfw_cmd[] = {
        {0x90, 0x00, 0, false}, //DRFW for Both Master and Slave
        {0xff, 0x3b, 0, true},
        {0xff, 0x00, 0, true},
        {0xff, 0xc1, 0, true},
    };
    _panel_write(ALL, drfw_cmd, NUM(drfw_cmd));

    // Send the image data
    _send_image(MASTER, BLACK, image_data, len);
    _send_image(SLAVE, BLACK, image_data, len);
    _send_image(MASTER, RED, image_data, len);
    _send_image(SLAVE, RED, image_data, len);

    // Wait until complete
    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);
}

// Refresh the panel with image data stored in on-chip RAM
void panel_refresh() {
    _cog_init();
    _dcdc_softstart();
    _panel_refresh();
}

// Shutdown the panel
void panel_shutdown() {
    _dcdc_shutdown();
}

// Internal
// Start a SPI transaction. Asserts the proper chip selects
// depending on which target is passed (ALL, MASTER, or SLAVE).
static void _spi_transaction_start(enum spi_target target) {
    gpio.write(PIN_PANEL_DC, LOW);
    gpio.write(PIN_LED, HIGH);

    switch(target) {
        case MASTER:
            gpio.write(PIN_PANEL_CS_M, LOW);
            break;
        case SLAVE:
            gpio.write(PIN_PANEL_CS_S, LOW);
            break;
        default: //ALL
            gpio.write(PIN_PANEL_CS_M, LOW);
            gpio.write(PIN_PANEL_CS_S, LOW);
            break;
    }
}

// End a SPI transaction. Deasserts all chip selects.
static void _spi_transaction_end() {
    gpio.write(PIN_PANEL_CS_M, HIGH);
    gpio.write(PIN_PANEL_CS_S, HIGH);
    gpio.write(PIN_LED, LOW);
}

// Write a sequence of commands to the panel. Sends the commands 
// to the target specified with spi_target (ALL, MASTER, or SLAVE).
static void _panel_write(enum spi_target target, struct epd_cmd* cmd, size_t num) {
    console.debug("panel write: %d commands\n", num);
    for (int i=0; i<num; i++) {
        console.debug("    cmd[%d]: 0x%02x=0x%02x, delay %dms, chain=%s\n",
            i, cmd[i].reg, cmd[i].data, cmd[i].wait_ms, cmd[i].chain_previous == true ? "true" : "false");
        /*
        Send a start command (chipselect + index register). 
        Always send a start command for the first command, and 
        do not send a start command if the current command
        is chained with the previous command.
        */
        if ((i == 0) || (!cmd[i].chain_previous)) {
            _spi_transaction_start(target);
            spi.write(&cmd[i].reg, 1);
    
            gpio.write(PIN_PANEL_DC, HIGH);
        }

        // Send the data
        spi.write(&cmd[i].data, 1);

        /*
        Send an end command. Always send an end command
        for the last command, and do not send an end command
        if the next command will be chained with the current command.
        */
        if (i == num-1 || !cmd[i+1].chain_previous) {
            _spi_transaction_end();
        }
        time.sleep_ms(cmd[i].wait_ms);
    }

    // Wait until complete
    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);

    console.debug("panel write done\n");
}

// Initialize the chip-on-glass (CoG) controller.
static void _cog_init() {
    console.debug("cog init: %lu\n", time.millis());

    // Timings and register values taken from EPD1200 datasheet, sec 3.1 flow chart
    struct epd_cmd cog_init_cmd[] = {
        {0x05, 0x7d, 200, false},
        {0x05, 0x00, 10, false},
        {0xd8, 0x80, 0, false},     // MS_SYNC
        {0xd6, 0x00, 0, false},     // BVSS
        {0xa7, 0x10, 100, false},
        {0xa7, 0x00, 100, false},
        {0x03, 0x00, 0, false},     // OSC 1st
        {0xFF, 0x11, 0, true},      // OSC 2nd
        {0x44, 0x00, 0, false},
        {0x45, 0x80, 0, false},
        {0xa7, 0x10, 100, false},
        {0xa7, 0x00, 100, false},
        {0x44, 0x06, 0, false},
        {0x45, 0x82, 0, false},     // Temperature 0x82@25C
        {0xa7, 0x10, 100, false},
        {0xa7, 0x00, 100, false},
        {0x60, 0x25, 0, false},     // TCON
        {0x61, 0x01, 0, false},     // STV_DIR for Master
        {0x02, 0x00, 0, false},     // VCOM
    };
    _panel_write(ALL, cog_init_cmd, NUM(cog_init_cmd));

    // Wait until complete
    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);
}

// Start the DC/DC converter
static void _dcdc_softstart() {
    console.debug("dc/dc softstart: %lu\n", time.millis());

    struct epd_cmd softstart_stage0_cmd[] = {
        {0x51, 0x50, 0, false},
        {0xFF, 0x01, 0, true},
    };
    _panel_write(ALL, softstart_stage0_cmd, NUM(softstart_stage0_cmd));

    // Soft-start stage 1
    for (uint8_t i=1; i<=4; i++) {
        struct epd_cmd softstart_stage1_cmd[] = {
            {0x09, 0x1f, 0, false},
            {0x51, 0x50, 0, false},
            {0xFF, i,    0, true},
            {0x09, 0x9f, 2, false},
        };
        _panel_write(ALL, softstart_stage1_cmd, NUM(softstart_stage1_cmd));
    }

    // Soft-start stage 2
    for (uint8_t i=1; i<=10; i++) {
        struct epd_cmd softstart_stage2_cmd[] = {
            {0x09, 0x1f, 0, false},
            {0x51, 0x0a, 0, false},
            {0xFF, i,    0, true},
            {0x09, 0x9f, 2, false},
        };
        _panel_write(ALL, softstart_stage2_cmd, NUM(softstart_stage2_cmd));
    }

    // Soft-start stage 3 (problematic!!!)
    for (uint8_t i=3; i<=10; i++) {
        struct epd_cmd softstart_stage3_cmd[] = {
            {0x09, 0x7f, 0, false},
            {0x51, 0x0a, 0, false},
            {0xFF, i,    0, true},
            {0x09, 0xff, 2, false},
        };
        _panel_write(ALL, softstart_stage3_cmd, NUM(softstart_stage3_cmd));
    }

    // Soft-start stage 4
    for (uint8_t i=9; i>=2; i--) {
        struct epd_cmd softstart_stage4_cmd[] = {
            {0x09, 0x7f, 0, false},
            {0x51, i,    0, false},
            {0xFF, 0x10, 0, true},
            {0x09, 0xff, 2, false},
        };
        _panel_write(ALL, softstart_stage4_cmd, NUM(softstart_stage4_cmd));
    }

    struct epd_cmd softstart_stage5_cmd[] = {
        {0x09, 0xff, 10, false},
    };
    _panel_write(ALL, softstart_stage5_cmd, NUM(softstart_stage5_cmd));

    // Wait until complete
    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);
}


// Refresh the panel
static void _panel_refresh() {
    console.debug("panel_refresh: %lu\n", time.millis());
    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);

    struct epd_cmd refresh_cmd[] = {
        {0x15, 0x3c, 5, false},
    };
    _panel_write(ALL, refresh_cmd, NUM(refresh_cmd));

    // Wait until complete
    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);

    console.debug("panel_refresh done: %lu\n", time.millis());
}

// Send the image to the on-chip RAM
static void _send_image(enum spi_target target, enum pixel_color color, uint8_t* image_data, uint32_t len) {
    console.debug("_send_image: %lu\n", time.millis());

    // Bounds checks
    if (len < 2*IMAGE_SIZE_BYTES) {
        console.print("WARNING: image length (%lu) is smaller than IMAGE_SIZE_BYTES (%lu)\n", len, IMAGE_SIZE_BYTES);
    }

    int ri, rf, ci, cf;

    ci = (target == MASTER) ? 0 : PANEL_WIDTH_BYTES/2;
    cf = (target == MASTER) ? PANEL_WIDTH_BYTES/2 : PANEL_WIDTH_BYTES;
    ri = (color == BLACK) ? 0 : PANEL_HEIGHT;
    rf = (color == BLACK) ? PANEL_HEIGHT : 2*PANEL_HEIGHT;

    struct epd_cmd ram_rw_cmd[] = {
        {0x12, 0x3b, 0, false},
        {0xFF, 0x00, 0, true},
        {0xFF, 0x14, 0, true},
    };
    _panel_write(target, ram_rw_cmd, NUM(ram_rw_cmd));

    // Image data
    uint8_t reg = (color == BLACK) ? 0x10 : 0x11;
    _spi_transaction_start(target); //RAM_RW
    spi.write(&reg, 1);
    gpio.write(PIN_PANEL_DC, HIGH);
    console.debug("_send_image(%s, %s)\n", (target == MASTER ) ? "master" : "slave", (color == BLACK ) ? "black" : "red");
    console.debug("ci:%d cf:%d ri:%d rf:%d\n", ci, cf, ri, rf);
    for (int r=ri; r<rf; r++) {
        spi.write(&image_data[((r * PANEL_WIDTH_BYTES) + ci)], (cf-ci));
    }
    _spi_transaction_end();
}

// Shut down the DC/DC converter
static void _dcdc_shutdown() {
    console.debug("dcdc_shutdown: %lu\n", time.millis());

    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);

    console.debug("dcdc_shutdown: done waiting\n");

    struct epd_cmd shutdown_cmd[] = {
        {0x09, 0x7f, 0},
        {0x05, 0x7d, 0},
        {0x09, 0x00, 0},
    };
    _panel_write(ALL, shutdown_cmd, NUM(shutdown_cmd));

    while (gpio.read(PIN_PANEL_BUSY) != HIGH) time.sleep_ms(100);

    gpio.write(PIN_PANEL_DC, LOW);
    gpio.write(PIN_PANEL_CS_M, LOW);
    gpio.write(PIN_PANEL_CS_S, LOW);
    gpio.write(PIN_PANEL_RESET, LOW);
    gpio.write(PIN_PANEL_BUSY, LOW);

    console.debug("dcdc_shutdown complete: %lu\n", time.millis());
}
