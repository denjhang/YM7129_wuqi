/**
 * YM7129 (OPK) Console Sound Test - Melody + Drum
 *
 * Usage: ym7129_test.exe [device_index]
 */

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#define Sleep(ms) usleep((ms) * 1000)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "spfm.h"
#include "ym7129.h"

// ============================================================
// Preset patches
// ============================================================
static const ym7129_patch_t patch_piano = {
    .feedback_lr = 0xE0,  // bit7=1, L=1, R=1, FB=0
    .m_mlws      = 0x01,
    .m_tl        = 0x10,
    .m_ardr      = 0xF2,
    .m_slrr      = 0x07,
    .c_mlws      = 0x01,
    .c_tl        = 0x00,
    .c_ardr      = 0xF2,
    .c_slrr      = 0x07,
};

static const ym7129_patch_t patch_bass = {
    .feedback_lr = 0xE0,
    .m_mlws      = 0x02,
    .m_tl        = 0x18,
    .m_ardr      = 0xF5,
    .m_slrr      = 0x88,
    .c_mlws      = 0x01,
    .c_tl        = 0x00,
    .c_ardr      = 0xF5,
    .c_slrr      = 0x1A,
};

// ============================================================
// Drum type constants
// ============================================================
#define RHY_BD    0
#define RHY_SD    1
#define RHY_CHH   2
#define RHY_OHH   3
#define RHY_TOM   4
#define RHY_RIDE  5
#define LH_LOW    1
#define LH_HI     0

// ============================================================
// Auto-detect SPFM device
// ============================================================
static int auto_detect_spfm(int dev_idx) {
    DWORD num_devs = 0;
    FT_STATUS st = FT_CreateDeviceInfoList(&num_devs);
    if (st != FT_OK || num_devs == 0) {
        fprintf(stderr, "No FTDI devices found.\n");
        return -1;
    }
    printf("Found %lu FTDI device(s):\n", (unsigned long)num_devs);
    FT_DEVICE_LIST_INFO_NODE *info =
        (FT_DEVICE_LIST_INFO_NODE *)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * num_devs);
    if (!info) return dev_idx;
    FT_GetDeviceInfoList(info, &num_devs);
    for (DWORD i = 0; i < num_devs; i++) {
        printf("  [%lu] %s\n", (unsigned long)i, info[i].Description);
    }
    free(info);
    return dev_idx;
}

// ============================================================
// Play melody only (simple scale C E G C5)
// ============================================================
static void test_melody(uint8_t slot) {
    printf("=== Test 1: FM Melody ===\n");

    ym7129_init(slot);
    ym7129_load_patch(slot, 0, &patch_piano);
    ym7129_load_patch(slot, 1, &patch_piano);

    uint8_t melody[] = {60, 64, 67, 72, 67, 64, 60, 60};
    int n = sizeof(melody) / sizeof(melody[0]);

    for (int i = 0; i < n; i++) {
        int ch = ym7129_play_note(slot, melody[i], 100);
        printf("  Note MIDI %d on ch%d\n", melody[i], ch);
        Sleep(350);
        ym7129_note_off(slot, (uint8_t)ch);
        Sleep(50);
    }

    ym7129_mute(slot);
    printf("Melody done.\n\n");
}

// ============================================================
// Play drum only (BD + SD alternating)
// ============================================================
static void test_drums(uint8_t slot) {
    printf("=== Test 2: Drums (BD + C.HH) ===\n");

    // Init drum channels
    ym7129_drum_strike(slot, 0, RHY_BD, LH_LOW, 3);   // CH1: BD, L type, vol=3
    Sleep(200);

    for (int i = 0; i < 8; i++) {
        printf("  Beat %d: BD\n", i + 1);
        ym7129_drum_strike(slot, 0, RHY_BD, LH_LOW, 3);
        Sleep(250);
        ym7129_drum_strike(slot, 2, RHY_CHH, LH_HI, 8);  // CH3: C.HH, H type, vol=8
        Sleep(250);
    }

    ym7129_drum_mute(slot);
    printf("Drums done.\n\n");
}

// ============================================================
// Play melody + drums together
// ============================================================
static void test_melody_drum(uint8_t slot) {
    printf("=== Test 3: Melody + Drums ===\n");

    ym7129_init(slot);

    // Load piano on ch0-1 for melody
    ym7129_load_patch(slot, 0, &patch_piano);
    ym7129_load_patch(slot, 1, &patch_piano);

    // Melody: C E G C5 E G C5 (ascending)
    uint8_t melody[] = {60, 64, 67, 72, 64, 67, 72, 72};
    int n = sizeof(melody) / sizeof(melody[0]);

    for (int i = 0; i < n; i++) {
        // Drum on beat
        ym7129_drum_strike(slot, 0, RHY_BD, LH_LOW, 3);
        Sleep(80);
        ym7129_drum_strike(slot, 2, RHY_CHH, LH_HI, 8);

        // Melody note
        int ch = ym7129_play_note(slot, melody[i], 100);
        printf("  Beat %d: BD + C.HH + MIDI %d\n", i + 1, melody[i]);

        Sleep(300);
        ym7129_note_off(slot, (uint8_t)ch);
        Sleep(100);
    }

    ym7129_mute(slot);
    ym7129_drum_mute(slot);
    printf("Melody+Drum done.\n\n");
}

// ============================================================
// Main
// ============================================================
int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  YM7129 (OPK) Test: Melody + Drum\n");
    printf("========================================\n\n");

    int dev_idx = 0;
    if (argc > 1) dev_idx = atoi(argv[1]);

    int dev = auto_detect_spfm(dev_idx);
    if (dev < 0) return 1;

    printf("Opening SPFM device %d...\n", dev);
    if (spfm_init(dev) != 0) {
        fprintf(stderr, "ERROR: Failed to open SPFM device.\n");
        return 1;
    }

    uint8_t slot = 0;

    test_melody(slot);
    Sleep(500);

    test_drums(slot);
    Sleep(500);

    test_melody_drum(slot);
    Sleep(500);

    printf("All tests complete.\n");
    ym7129_mute(slot);
    ym7129_drum_mute(slot);
    spfm_cleanup();
    return 0;
}
