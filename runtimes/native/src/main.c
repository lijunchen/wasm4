#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apu.h"
#include "runtime.h"
#include "wasm0.h"
#include "window.h"
#include "util.h"


extern unsigned char __tinypong_wasm[];
extern unsigned int __tinypong_wasm_len;

void w4_windowBoot (const char* title);

int main (int argc, const char* argv[]) {
    w4_Disk disk = {0};
    int len = __tinypong_wasm_len;
    unsigned char* wasm = xmalloc(len);
    for (int i = 0; i < len; i++) {
        wasm[i] = __tinypong_wasm[i];
    }
    uint8_t* memory = w4_wasmInit();
    w4_runtimeInit(memory, &disk);
    w4_wasmLoadModule(wasm, len);
    w4_windowBoot("test");
}
