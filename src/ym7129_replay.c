/**
 * ym7129_replay.c
 * Replay YM7129 bus capture data from .txt files.
 *
 * Supports two modes:
 *   --drum  <file>  : Rhythm-only replay (filters $80-$84 register writes)
 *   --fm    <file>  : FM-only replay (filters $01,$10-$6F,$70-$77 writes)
 *   --all   <file>  : Replay all register writes
 *
 * Usage: ym7129_replay.exe [--drum|--fm|--all] <file.txt> [device_index]
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
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "spfm.h"
#include "ym7129.h"

// ============================================================
// Bus event: one data byte with timestamp
// ============================================================
typedef struct {
    uint64_t timestamp;  // sample number
    uint8_t  data;
} bus_event_t;

#define MAX_EVENTS 500000
static bus_event_t events[MAX_EVENTS];
static uint32_t    num_events = 0;

// ============================================================
// Register write command (addr + data)
// ============================================================
typedef struct {
    uint64_t timestamp;
    uint8_t  addr;
    uint8_t  data;
} reg_write_t;

#define MAX_WRITES 200000
static reg_write_t writes[MAX_WRITES];
static uint32_t    num_writes = 0;

// ============================================================
// Parse .txt capture file
// Format: <start>-<end> Z80: Data bus: <hex>
// ============================================================
static int parse_txt(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Cannot open: %s\n", filename);
        return -1;
    }

    char line[512];
    num_events = 0;

    while (fgets(line, sizeof(line), f)) {
        if (num_events >= MAX_EVENTS) break;

        // Find "Data bus: " prefix
        const char *p = strstr(line, "Data bus: ");
        if (!p) continue;
        p += 10; // skip "Data bus: "

        // Parse hex value
        uint8_t data = (uint8_t)strtoul(p, NULL, 16);

        // Parse timestamp (first number before '-')
        uint64_t ts = 0;
        const char *q = line;
        while (*q && !isdigit((unsigned char)*q)) q++;
        ts = strtoull(q, NULL, 10);

        events[num_events].timestamp = ts;
        events[num_events].data = data;
        num_events++;
    }

    fclose(f);
    printf("Parsed %u events from %s\n", num_events, filename);
    return 0;
}

// ============================================================
// Decode bus events into register writes
// YM7129 protocol (from waveform analysis):
//   Two consecutive bytes = [addr, data] pair
//   Written to Z80 data bus in sequence
//
// For rhythm captures ($80-$84 range):
//   Each byte is a direct register value (addr is implicit from
//   the previous address byte in the sequence)
//
// We use a simple state machine:
//   state 0: expecting address byte
//   state 1: expecting data byte (previous was address)
// ============================================================
static void decode_rhythm_only(void) {
    num_writes = 0;
    uint8_t pending_addr = 0xFF;
    bool have_addr = false;

    for (uint32_t i = 0; i < num_events && num_writes < MAX_WRITES; i++) {
        uint8_t d = events[i].data;

        if (!have_addr) {
            pending_addr = d;
            have_addr = true;
        } else {
            // Filter: only rhythm registers $80-$84
            if (pending_addr >= 0x80 && pending_addr <= 0x84) {
                writes[num_writes].timestamp = events[i - 1].timestamp;
                writes[num_writes].addr = pending_addr;
                writes[num_writes].data = d;
                num_writes++;
            }
            have_addr = false;
        }
    }
}

static void decode_all_regs(void) {
    num_writes = 0;
    uint8_t pending_addr = 0xFF;
    bool have_addr = false;

    for (uint32_t i = 0; i < num_events && num_writes < MAX_WRITES; i++) {
        uint8_t d = events[i].data;

        if (!have_addr) {
            pending_addr = d;
            have_addr = true;
        } else {
            writes[num_writes].timestamp = events[i - 1].timestamp;
            writes[num_writes].addr = pending_addr;
            writes[num_writes].data = d;
            num_writes++;
            have_addr = false;
        }
    }
}

static void decode_fm_only(void) {
    num_writes = 0;
    uint8_t pending_addr = 0xFF;
    bool have_addr = false;

    for (uint32_t i = 0; i < num_events && num_writes < MAX_WRITES; i++) {
        uint8_t d = events[i].data;

        if (!have_addr) {
            pending_addr = d;
            have_addr = true;
        } else {
            // Filter: FM registers are $01, $10-$6F, $70-$77
            bool is_fm = (pending_addr == 0x01) ||
                         (pending_addr >= 0x10 && pending_addr <= 0x6F) ||
                         (pending_addr >= 0x70 && pending_addr <= 0x77);
            if (is_fm) {
                writes[num_writes].timestamp = events[i - 1].timestamp;
                writes[num_writes].addr = pending_addr;
                writes[num_writes].data = d;
                num_writes++;
            }
            have_addr = false;
        }
    }
}

// ============================================================
// Replay register writes with timing
// ============================================================
static void replay(uint8_t slot, double speed) {
    if (num_writes == 0) {
        printf("No writes to replay.\n");
        return;
    }

    printf("Replaying %u writes (speed=%.1fx)...\n", num_writes, speed);

    for (uint32_t i = 0; i < num_writes; i++) {
        // Calculate delay from timestamp difference
        if (i > 0) {
            uint64_t diff = writes[i].timestamp - writes[i - 1].timestamp;
            // Samples to microseconds: sample_rate = 24MHz
            // 1 sample = 1/24e6 seconds = ~41.67ns
            double us = (double)diff / 24.0; // microseconds
            us /= speed;

            if (us > 5000000.0) {
                // Gap > 5s: cap to 2s
                printf("  [gap %.1fs -> capped to 2s]\n", us / 1e6);
                Sleep(2000);
            } else if (us > 1000.0) {
                Sleep((DWORD)(us / 1000.0));
            } else if (us > 50.0) {
                // Sub-ms: use Sleep(1) as minimum granularity
                Sleep(1);
            }
            // < 50us: no delay, just send
        }

        // Write register via YM7129 raw protocol
        ym7129_write_reg(slot, writes[i].addr, writes[i].data);
    }

    spfm_flush();
    printf("Replay complete.\n");
}

// ============================================================
// Usage
// ============================================================
static void print_usage(const char *prog) {
    printf("Usage: %s [--drum|--fm|--all] <file.txt> [device_index] [speed]\n", prog);
    printf("  --drum  : Rhythm-only replay ($80-$84)\n");
    printf("  --fm    : FM-only replay ($01,$10-$6F,$70-$77)\n");
    printf("  --all   : All register writes\n");
    printf("  speed   : Playback speed multiplier (default 1.0)\n");
    printf("\nExample:\n");
    printf("  %s --drum data_rhythm_0.txt 0\n", prog);
    printf("  %s --all data_fm_11.txt 0 2.0\n", prog);
}

// ============================================================
// Main
// ============================================================
int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  YM7129 Bus Capture Replay\n");
    printf("========================================\n\n");

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse mode
    const char *mode = argv[1];
    const char *filename = argv[2];
    int dev_idx = (argc > 3) ? atoi(argv[3]) : 0;
    double speed = (argc > 4) ? atof(argv[4]) : 1.0;

    bool do_drum = false, do_fm = false, do_all = false;
    if (strcmp(mode, "--drum") == 0) do_drum = true;
    else if (strcmp(mode, "--fm") == 0) do_fm = true;
    else if (strcmp(mode, "--all") == 0) do_all = true;
    else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        print_usage(argv[0]);
        return 1;
    }

    // Parse capture file
    if (parse_txt(filename) != 0) return 1;

    // Decode
    if (do_drum) {
        printf("Mode: Rhythm-only ($80-$84)\n");
        decode_rhythm_only();
    } else if (do_fm) {
        printf("Mode: FM-only ($01,$10-$6F,$70-$77)\n");
        decode_fm_only();
    } else {
        printf("Mode: All registers\n");
        decode_all_regs();
    }
    printf("Decoded %u register writes\n", num_writes);

    if (num_writes == 0) {
        printf("Nothing to replay.\n");
        return 0;
    }

    // Init SPFM
    printf("Opening SPFM device %d...\n", dev_idx);
    if (spfm_init(dev_idx) != 0) {
        fprintf(stderr, "ERROR: Failed to open SPFM device.\n");
        return 1;
    }

    uint8_t slot = 0;

    // For FM mode, do chip init first
    if (do_fm || do_all) {
        ym7129_init(slot);
    }

    // Replay
    replay(slot, speed);

    // Cleanup
    Sleep(500);
    ym7129_mute(slot);
    ym7129_drum_mute(slot);
    spfm_cleanup();
    return 0;
}
