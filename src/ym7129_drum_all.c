/**
 * ym7129_drum_all.c
 * Test all 12 drum sounds one by one (6 types x 2 L/H).
 * Each test: init chip, strike once, wait.
 */

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(ms) usleep((ms) * 1000)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "spfm.h"
#include "ym7129.h"

#define RHY_BD    0
#define RHY_SD    1
#define RHY_CHH   2
#define RHY_OHH   3
#define RHY_TOM   4
#define RHY_RIDE  5
#define LH_LOW    1
#define LH_HI     0

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  YM7129 Drum Test: 12 Sounds\n");
    printf("========================================\n\n");

    int dev_idx = 0;
    if (argc > 1) dev_idx = atoi(argv[1]);

    if (spfm_init(dev_idx) != 0) {
        fprintf(stderr, "ERROR: Failed to open SPFM device.\n");
        return 1;
    }

    uint8_t slot = 0;

    struct { uint8_t rhy; uint8_t lh; const char *name; } tests[] = {
        { RHY_BD,   LH_LOW, "BD L"   },
        { RHY_BD,   LH_HI, "BD H"   },
        { RHY_SD,   LH_LOW, "SD L"   },
        { RHY_SD,   LH_HI, "SD H"   },
        { RHY_CHH,  LH_LOW, "C.HH L" },
        { RHY_CHH,  LH_HI, "C.HH H" },
        { RHY_OHH,  LH_LOW, "O.HH L" },
        { RHY_OHH,  LH_HI, "O.HH H" },
        { RHY_TOM,  LH_LOW, "Tom L"  },
        { RHY_TOM,  LH_HI, "Tom H"  },
        { RHY_RIDE, LH_LOW, "Ride L" },
        { RHY_RIDE, LH_HI, "Ride H" },
    };
    int n = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < n; i++) {
        ym7129_init(slot);
        Sleep(100);

        printf("%2d/%d: %-7s ", i + 1, n, tests[i].name);
        fflush(stdout);
        ym7129_drum_strike(slot, 0, tests[i].rhy, tests[i].lh, 0);
        Sleep(500);

        ym7129_drum_mute(slot);
        printf("OK\n");
        Sleep(200);
    }

    printf("\nAll 12 drum sounds tested.\n");
    spfm_cleanup();
    return 0;
}
