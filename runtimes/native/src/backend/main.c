#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../apu.h"
#include "../runtime.h"
#include "../wasm.h"
#include "../window.h"
#include "../util.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#define DISK_FILE_EXT ".disk"

typedef struct {
    // Should be the 4 byte ASCII string "CART" (1414676803)
    uint32_t magic;

    // Window title
    char title[128];

    // Length of the cart.wasm bytes used to offset backwards from the footer
    uint32_t cartLength;
} FileFooter;

static void loadDiskFile (w4_Disk* disk, const char *diskPath) {
    FILE *file = fopen(diskPath, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        uint16_t saveSz = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (saveSz > sizeof(disk->data)) {
            saveSz = sizeof(disk->data);
        }
        disk->size = fread(disk->data, 1, saveSz, file);
        fclose(file);
    }
}

static void saveDiskFile (const w4_Disk* disk, const char *diskPath) {
    if (disk->size) {
        FILE* file = fopen(diskPath, "wb");
        fwrite(disk->data, 1, disk->size, file);
        fclose(file);
    } else {
        remove(diskPath);
    }
}

static void trimFileExtension (char *path) {
    size_t len = strlen(path);
    while (len--) {
        if (path[len] == '.') {
            path[len] = 0; // Set null terminator
            return;
        } else if (path[len] == '/' || path[len] == '\\') {
            return;
        }
    }
}

int main (int argc, const char* argv[]) {
    uint8_t* cartBytes;
    size_t cartLength;
    w4_Disk disk = {0};
    const char* title = "WASM-4";
    char* diskPath = NULL;

    if (argc < 2) {
        FILE* file = fopen(argv[0], "rb");
        fseek(file, -sizeof(FileFooter), SEEK_END);

        FileFooter footer;
        if (fread(&footer, 1, sizeof(FileFooter), file) < sizeof(FileFooter) || footer.magic != 1414676803) {
            // No bundled cart found
            fprintf(stderr, "Usage: wasm4 <cart>\n");
            return 1;
        }

        // Make sure the title is null terminated
        footer.title[sizeof(footer.title)-1] = '\0';
        title = footer.title;

        cartBytes = xmalloc(footer.cartLength);
        fseek(file, -sizeof(FileFooter) - footer.cartLength, SEEK_END);
        cartLength = fread(cartBytes, 1, footer.cartLength, file);
        fclose(file);

        // Look for disk file
        diskPath = xmalloc(strlen(argv[0]) + sizeof(DISK_FILE_EXT));
        strcpy(diskPath, argv[0]);
#ifdef _WIN32
        trimFileExtension(diskPath); // Trim .exe on Windows
#endif
        strcat(diskPath, DISK_FILE_EXT);
        loadDiskFile(&disk, diskPath);

    } else if (!strcmp(argv[1], "-") || !strcmp(argv[1], "/dev/stdin")) {
        size_t bufsize = 1024;
        cartBytes = xmalloc(bufsize);
        cartLength = 0;
        int c;

        while((c = getc(stdin)) != EOF) {
            cartBytes[cartLength++] = c;
            if(cartLength == bufsize) {
                if (cartLength >= 64 * 1024) {
                    fprintf(stderr, "Error, overflown cartridge size limit of 64 KB\n");
                    return 1;
                }

                bufsize *= 2;
                cartBytes = xrealloc(cartBytes, bufsize);

                if(!cartBytes) {
                    fprintf(stderr, "Error reallocating cartridge buffer\n");
                    return 1;
                }
            }
        }
    }
    else {
        FILE* file = fopen(argv[1], "rb");
        if (file == NULL) {
            fprintf(stderr, "Error opening %s\n", argv[1]);
            return 1;
        }

        fseek(file, 0, SEEK_END);
        cartLength = ftell(file);
        fseek(file, 0, SEEK_SET);

        cartBytes = xmalloc(cartLength);
        cartLength = fread(cartBytes, 1, cartLength, file);
        fclose(file);

        // Look for disk file
        diskPath = xmalloc(strlen(argv[1]) + sizeof(DISK_FILE_EXT));
        strcpy(diskPath, argv[1]);
        trimFileExtension(diskPath); // Trim .wasm
        strcat(diskPath, DISK_FILE_EXT);
        loadDiskFile(&disk, diskPath);
    }

    uint8_t* memory = w4_wasmInit();
    w4_runtimeInit(memory, &disk);

    w4_wasmLoadModule(cartBytes, cartLength);

    w4_windowBoot(title);

    saveDiskFile(&disk, diskPath);
}
