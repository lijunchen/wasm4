#include <stdio.h>
#include "svpng.h"

#include "../window.h"
#include "../runtime.h"

static uint32_t pixels[160*160];

static int viewportX = 0;
static int viewportY = 0;
static int viewportSize = 3*160;

void w4_windowBoot (const char* title) {
    do {

        // Player 1
        uint8_t gamepad = 0;
        w4_runtimeSetGamepad(0, gamepad);

        // Player 2
        gamepad = 0;
        w4_runtimeSetGamepad(1, gamepad);

        // Mouse handling
        uint8_t mouseButtons = 0;
        int mouseX = 0;
        int mouseY = 0;
        w4_runtimeSetMouse(160*(mouseX-viewportX)/viewportSize, 160*(mouseY-viewportY)/viewportSize, mouseButtons);

        w4_runtimeUpdate();

    } while (1);
}

int counter = 0;

void w4_windowComposite (const uint32_t* palette, const uint8_t* framebuffer) {
    // Convert indexed 2bpp framebuffer to XRGB output
    uint32_t* out = pixels;
    char filename[32];
    sprintf(filename, "out/frame_%03d.png", counter++);
    FILE *fp = fopen(filename, "wb");
    unsigned char rgb[160 * 160 * 3], *p = rgb;
    unsigned x = 0;
    unsigned y = 0;
    for (int n = 0; n < 160*160/4; ++n) {
        uint8_t quartet = framebuffer[n];
        int color1 = (quartet & 0b00000011) >> 0;
        int color2 = (quartet & 0b00001100) >> 2;
        int color3 = (quartet & 0b00110000) >> 4;
        int color4 = (quartet & 0b11000000) >> 6;

        *out++ = palette[color1];
        *out++ = palette[color2];
        *out++ = palette[color3];
        *out++ = palette[color4];

        *p++ = (unsigned char)((palette[color1] & 0xff0000) >> 16);
        *p++ = (unsigned char)((palette[color1] & 0xff00) >> 8);
        *p++ = (unsigned char)(palette[color1] & 0xff);

        *p++ = (unsigned char)((palette[color2] & 0xff0000) >> 16);
        *p++ = (unsigned char)((palette[color2] & 0xff00) >> 8);
        *p++ = (unsigned char)(palette[color2] & 0xff);

        *p++ = (unsigned char)((palette[color3] & 0xff0000) >> 16);
        *p++ = (unsigned char)((palette[color3] & 0xff00) >> 8);
        *p++ = (unsigned char)(palette[color3] & 0xff);

        *p++ = (unsigned char)((palette[color4] & 0xff0000) >> 16);
        *p++ = (unsigned char)((palette[color4] & 0xff00) >> 8);
        *p++ = (unsigned char)(palette[color4] & 0xff);
    }
    svpng(fp, 160, 160, rgb, 0);
    fclose(fp);
}
