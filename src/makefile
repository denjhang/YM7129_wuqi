# YM7129 (OPK) Console Sound Test Makefile
# Requires: FTDI D2XX library, yasp SPFM module

# Paths
FTD2XX_DIR = .

# Compiler settings
CC      = gcc
CFLAGS  = -Wall -O2 -I.
LDFLAGS = -L$(FTD2XX_DIR) -lftd2xx -lws2_32

# Source files (standalone - no yasp dependencies)
SRCS = ym7129_test.c ym7129.c spfm_lite.c error_lite.c
SRCS2 = pss26_play.c spfm_lite.c error_lite.c
SRCS3 = ym7129_drum_test.c ym7129.c spfm_lite.c error_lite.c
SRCS4 = ym7129_drum88_replay.c spfm_lite.c error_lite.c
SRCS5 = ym3812_test.c ym3812.c spfm_lite.c error_lite.c
SRCS6 = ym7129_melody.c spfm_lite.c error_lite.c
SRCS7 = ym7129_simple_drum.c spfm_lite.c error_lite.c
SRCS8 = ym7129_write_test.c spfm_lite.c error_lite.c
SRCS9 = ym7129_mode_a_test.c spfm_lite.c error_lite.c
SRCS10 = ym7129_stable_test.c spfm_lite.c error_lite.c
SRCS11 = ym7129_minimal_test.c spfm_lite.c error_lite.c
SRCS12 = ym7129_fm_test.c spfm_lite.c error_lite.c
SRCS13 = ym7129_reg_scan.c spfm_lite.c error_lite.c
SRCS14 = ym7129_freq_test_v2.c spfm_lite.c error_lite.c
SRCS15 = ym7129_replay.c ym7129.c spfm_lite.c error_lite.c

# Object files
OBJS = $(SRCS:.c=.o)
OBJS2 = pss26_play.o spfm_lite.o error_lite.o
OBJS3 = ym7129_drum_test.o ym7129.o spfm_lite.o error_lite.o
OBJS4 = ym7129_drum88_replay.o spfm_lite.o error_lite.o
OBJS5 = ym3812_test.o ym3812.o spfm_lite.o error_lite.o
OBJS6 = ym7129_melody.o spfm_lite.o error_lite.o
OBJS7 = ym7129_simple_drum.o spfm_lite.o error_lite.o
OBJS8 = ym7129_write_test.o spfm_lite.o error_lite.o
OBJS9 = ym7129_mode_a_test.o spfm_lite.o error_lite.o
OBJS10 = ym7129_stable_test.o spfm_lite.o error_lite.o
OBJS11 = ym7129_minimal_test.o spfm_lite.o error_lite.o
OBJS12 = ym7129_fm_test.o spfm_lite.o error_lite.o
OBJS13 = ym7129_reg_scan.o spfm_lite.o error_lite.o
OBJS14 = ym7129_freq_test_v2.o spfm_lite.o error_lite.o
OBJS15 = ym7129_replay.o ym7129.o spfm_lite.o error_lite.o
OBJS16 = ym7129_drum_all.o ym7129.o spfm_lite.o error_lite.o

# Targets
TARGET  = ym7129_test.exe
TARGET2 = pss26_play.exe
TARGET3 = ym7129_drum_test.exe
TARGET4 = ym7129_drum88_replay.exe
TARGET5 = ym3812_test.exe
TARGET6 = ym7129_melody.exe
TARGET7 = ym7129_simple_drum.exe
TARGET8 = ym7129_write_test.exe
TARGET9 = ym7129_mode_a_test.exe
TARGET10 = ym7129_stable_test.exe
TARGET11 = ym7129_minimal_test.exe
TARGET12 = ym7129_fm_test.exe
TARGET13 = ym7129_reg_scan.exe
TARGET14 = ym7129_freq_test_v2.exe
TARGET15 = ym7129_replay.exe
TARGET16 = ym7129_drum_all.exe

# Build rules
all: $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8) $(TARGET9) $(TARGET10) $(TARGET11) $(TARGET12) $(TARGET13) $(TARGET14) $(TARGET15)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

$(TARGET2): $(OBJS2)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET2)"

$(TARGET3): $(OBJS3)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET3)"

$(TARGET4): $(OBJS4)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET4)"

$(TARGET5): $(OBJS5)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET5)"

$(TARGET6): $(OBJS6)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET6)"

$(TARGET7): $(OBJS7)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET7)"

$(TARGET8): $(OBJS8)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET8)"

$(TARGET9): $(OBJS9)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET9)"

$(TARGET10): $(OBJS10)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET10)"

$(TARGET11): $(OBJS11)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET11)"

$(TARGET12): $(OBJS12)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET12)"

$(TARGET13): $(OBJS13)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET13)"

$(TARGET14): $(OBJS14)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET14)"

$(TARGET15): $(OBJS15)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET15)"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	del /Q *.o 2>/dev/null
	del /Q $(TARGET) 2>/dev/null
	del /Q $(TARGET2) 2>/dev/null
	del /Q $(TARGET3) 2>/dev/null
	del /Q $(TARGET4) 2>/dev/null
	del /Q $(TARGET5) 2>/dev/null
	del /Q $(TARGET6) 2>/dev/null
	del /Q $(TARGET7) 2>/dev/null
	del /Q $(TARGET8) 2>/dev/null
	del /Q $(TARGET9) 2>/dev/null
	del /Q $(TARGET10) 2>/dev/null
	del /Q $(TARGET11) 2>/dev/null
	del /Q $(TARGET12) 2>/dev/null
	del /Q $(TARGET13) 2>/dev/null
	del /Q $(TARGET14) 2>/dev/null
	@echo "Clean complete."

# Run
run: $(TARGET)
	$(TARGET)

run_drum: $(TARGET3)
	$(TARGET3)

run_drum88: $(TARGET4)
	$(TARGET4)

run_ym3812: $(TARGET5)
	$(TARGET5)

run_melody: $(TARGET6)
	$(TARGET6)

run_simple_drum: $(TARGET7)
	$(TARGET7)

run_write_test: $(TARGET8)
	$(TARGET8)

run_mode_a: $(TARGET9)
	$(TARGET9)

run_stable: $(TARGET10)
	$(TARGET10)

run_minimal: $(TARGET11)
	$(TARGET11)

run_fm: $(TARGET12)
	$(TARGET12)

run_scan: $(TARGET13)
	$(TARGET13)

run_freq_v2: $(TARGET14)
	$(TARGET14)

# Phony targets
.PHONY: all clean run run_drum run_drum88 run_ym3812 run_melody run_simple_drum run_write_test run_mode_a run_stable run_minimal run_fm run_scan run_freq_v2 replay
