/**
 * ym7129_fm_test_v2.c
 * Pure FM test - exact copy of ym7129_test.c melody logic, no drums.
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

static const ym7129_patch_t patch_piano = {
    .feedback_lr = 0xE0,
    .m_mlws      = 0x01,
    .m_tl        = 0x10,
    .m_ardr      = 0xF2,
    .m_slrr      = 0x07,
    .c_mlws      = 0x01,
    .c_tl        = 0x00,
    .c_ardr      = 0xF2,
    .c_slrr      = 0x07,
};

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  YM7129 FM Test v2\n");
    printf("========================================\n\n");

    int dev_idx = 0;
    if (argc > 1) dev_idx = atoi(argv[1]);

    if (spfm_init(dev_idx) != 0) {
        fprintf(stderr, "ERROR: Failed to open SPFM device.\n");
        return 1;
    }

    uint8_t slot = 0;

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
    printf("FM test done.\n");
    spfm_cleanup();
    return 0;
}
