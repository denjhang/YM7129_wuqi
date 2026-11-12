# YM7129 硬件验证测试报告

**创建日期**: 2026-05-22
**硬件**: SPFM Light + YM7129 (OPK) 芯片
**协议**: 3字节 raw 模式 `[slot, 0x80, addr] [slot, 0x81, data]`

> 本文档记录所有经过实际硬件验证的数据。之前所有测试文档 (V1, V2) 均已废弃。

---

## 1. SPFM Light 协议 (已验证)

### 写寄存器
```c
void ym7129_write_reg(uint8_t slot, uint8_t addr, uint8_t data) {
    uint8_t cmd[6];
    cmd[0] = slot; cmd[1] = 0x80; cmd[2] = addr;  // A0=0, 地址
    cmd[3] = slot; cmd[4] = 0x81; cmd[5] = data;  // A0=1, 数据
    spfm_write_raw(cmd, 6);
    spfm_flush();
    spfm_sleep_us(90);
}
```

- 每次写入必须 **flush + 90us 延时**，满足 89.2us 连续写入间隔要求
- 不加 flush: 多次写入缓冲在 USB 包内连续发送，两次写入间隔远不够 89.2us
- YM7129 用 `0x80=地址, 0x81=数据`（注意 SAA1099 极性相反）
- **不能用 `spfm_hw_wait(0x80)` 插入延时**，0x80 在 raw 模式下是命令字节，会破坏数据流

---

## 2. 寄存器映射 (已验证)

| 地址范围 | 功能 | 位定义 |
|----------|------|--------|
| $01 | 鼓左右声道选择 | bit0-3=Rhy0-3 L, bit4-7=Rhy0-3 R（写 $FF = 四路鼓全开 L+R） |
| $10-$1F | KON/VIB/WS/MUL | bit7=KON, bit6=VIB, bit5=WS, bit3-0=MUL |
| $20-$2F | DP/TL | bit7=DP, bit6-0=TL (0=最响) |
| $30-$3F | AR/DR | bit7-4=AR, bit3-0=DR |
| $40-$4F | SL/RR | bit7-4=SL, bit3-0=RR |
| $50-$5F | BLK/FNUM-H | bit7-5=BLK(0-7), bit4-0=FNUM[10:6] |
| $60-$6F | FNUM-L | bit7-2=FNUM[5:0] (左对齐) |
| $70-$77 | L/R/FB | bit6=R, bit5=L, bit2-0=FB (每通道线性) |
| $80-$83 | 鼓通道控制 | bit7=L/H, bit6-4=RhyNo, bit3-0=Volume |
| $84 | 鼓触发 | bit3-0=k3-k0, **0→1 上升沿触发** |

### 通道偏移表 (非线性映射)
```
逻辑通道: 0  1  2  3  4  5  6  7
物理偏移: 0  1  4  5  8  9  12 13
Modulator = base + offset
Carrier   = base + offset + 2
```

---

## 3. FM 旋律 (已验证)

### 初始化序列 (全寄存器写入，保证每次运行音色一致)
每次上电后芯片内部状态不确定，必须把**所有** FM 寄存器写一遍确定值：

```
$01 = FF                     (鼓左右声道: 4 个节奏通道全部 L+R)

每个通道 (ch 0-7, offset = ch_offset[ch]):
  Modulator:
    $10+off   = 00            MLWS: KON=0, ML=0
    $20+off   = FF            TL: 最大衰减 (静音)
    $30+off   = 00            ARDR: AR=0, DR=0
    $40+off   = 00            SLRR: SL=0, RR=0
  Carrier:
    $10+off+2 = 0D            MLWS: KON=0, ML=13
    $20+off+2 = 7F            TL: 最大衰减 (静音)
    $30+off+2 = 00            ARDR: AR=0, DR=0
    $40+off+2 = 0B            SLRR: SL=0, RR=11
  频率:
    $50+off   = 00            BLK=0, FNUM-H=0
    $60+off   = 00            FNUM-L=0
  输出:
    $70+ch    = 00            L/R/FB=0 (静音)
flush
```

> **重要**: 不写全寄存器会导致每次运行音色不同（芯片内部状态残留）

### 音符频率
- F-Number 表 (3.579545MHz 时钟):
  ```
  C:0x2B1  C#:0x2E5  D:0x311  D#:0x340
  E:0x365  F:0x398   F#:0x3DC  G:0x409
  G#:0x446 A:0x488   A#:0x4DE  B:0x516
  ```
- BLK = octave - 1 (C-1=BLK0, C4=BLK3)
- 如果 FNUM >= $800, 右移1位并 BLK+1

### 音符触发
1. 写频率: $52+off = (BLK<<5)|(FNUM>>6), $62+off = (FNUM<<2)&0xF0
2. 写 Carrier TL (velocity: 0=响, 127=轻)
3. Key-On: MLWS+off+2 的 bit7 置1
4. Key-Off: MLWS+off+2 的 bit7 清0, 恢复 SLRR

### 验证结果
- C E G C5 音阶播放正常 (MIDI 60,64,67,72)
- 每次运行音色一致 (全寄存器初始化后)
- 8通道轮换正常

---

## 4. 鼓声 (已验证)

### 鼓声类型
| RhyNo | 名称 | L/H 类型 |
|-------|------|----------|
| 0 | BD | L(low) |
| 1 | SD | L(low) |
| 2 | C.HH | H(hi) |
| 3 | O.HH | H(hi) |
| 4 | Tom | L(low) |
| 5 | Ride | H(hi) |

### 触发方式
- **$84 上升沿触发**: 先写 $84=clear, 等待, 再写 $84=set
- $80-$83 设置鼓类型, bit7=L/H, bit6-4=RhyNo, bit3-0=Volume

### 时序 (来自波形分析)
- $80-$83 设置后等 **1.5ms**
- $84 clear 后等 **3ms** 再 set (上升沿)
- Vol=0 最响, Vol=15 最轻

### 验证结果
- 12种鼓声全部出声 (6类型 x 2 L/H)
- 鼓声自然衰减，**不需要额外 mute** ($84 清零会触发额外杂音)
- BD + C.HH 交替节奏正常
- 音量扫描 (vol=0~15) 正常

---

## 5. 源文件清单

| 文件 | 功能 |
|------|------|
| `ym7129.c/h` | YM7129 驱动 (寄存器写入、音符、鼓) |
| `spfm_lite.c/h` | SPFM Light 接口 (FTDI通信) |
| `ym7129_test.c` | 旋律+鼓综合测试 |
| `ym7129_fm_test_v2.c` | FM 纯旋律测试 |
| `ym7129_drum_all.c` | 12种鼓声逐个测试 |
| `ym7129_replay.c` | 波形数据回放 (--drum/--fm/--all) |

### 运行指令
```bash
cd /d/working/vscode-projects/YM7129/ym7129_test

# FM 纯旋律测试
./ym7129_fm_test_v2.exe 0

# 旋律+鼓综合测试
./ym7129_test.exe 0

# 12种鼓声逐个测试
./ym7129_drum_all.exe 0

# 波形数据回放
./ym7129_replay.exe --drum data_rhythm_0.txt 0
./ym7129_replay.exe --fm data_fm_11.txt 0
```
