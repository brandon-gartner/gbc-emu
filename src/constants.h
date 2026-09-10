#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

namespace gb {

// memory map region boundaries
constexpr uint16_t ROM0_START   = 0x0000;
constexpr uint16_t ROM0_END     = 0x3FFF;
constexpr uint16_t ROM1_START   = 0x4000;
constexpr uint16_t ROM1_END     = 0x7FFF;
constexpr uint16_t VRAM_START   = 0x8000;
constexpr uint16_t VRAM_END     = 0x9FFF;
constexpr uint16_t SRAM_START   = 0xA000;
constexpr uint16_t SRAM_END     = 0xBFFF;
constexpr uint16_t WRAM0_START  = 0xC000;
constexpr uint16_t WRAM0_END    = 0xCFFF;
constexpr uint16_t WRAM1_START  = 0xD000;
constexpr uint16_t WRAM1_END    = 0xDFFF;
constexpr uint16_t ECHO_START   = 0xE000;
constexpr uint16_t ECHO_END     = 0xFDFF;
constexpr uint16_t OAM_START    = 0xFE00;
constexpr uint16_t OAM_END      = 0xFE9F;
constexpr uint16_t UNUSED_START = 0xFEA0;
constexpr uint16_t UNUSED_END   = 0xFEFF;
constexpr uint16_t IO_START     = 0xFF00;
constexpr uint16_t IO_END       = 0xFF7F;
constexpr uint16_t HRAM_START   = 0xFF80;
constexpr uint16_t HRAM_END     = 0xFFFE;
constexpr uint16_t IE_REG       = 0xFFFF;

// hardware i/o registers
constexpr uint16_t REG_JOYP  = 0xFF00;
constexpr uint16_t REG_SB    = 0xFF01;
constexpr uint16_t REG_SC    = 0xFF02;
constexpr uint16_t REG_DIV   = 0xFF04;
constexpr uint16_t REG_TIMA  = 0xFF05;
constexpr uint16_t REG_TMA   = 0xFF06;
constexpr uint16_t REG_TAC   = 0xFF07;
constexpr uint16_t REG_IF    = 0xFF0F;
constexpr uint16_t REG_LCDC  = 0xFF40;
constexpr uint16_t REG_STAT  = 0xFF41;
constexpr uint16_t REG_SCY   = 0xFF42;
constexpr uint16_t REG_SCX   = 0xFF43;
constexpr uint16_t REG_LY    = 0xFF44;
constexpr uint16_t REG_LYC   = 0xFF45;
constexpr uint16_t REG_DMA   = 0xFF46;
constexpr uint16_t REG_BGP   = 0xFF47;
constexpr uint16_t REG_OBP0  = 0xFF48;
constexpr uint16_t REG_OBP1  = 0xFF49;
constexpr uint16_t REG_WY    = 0xFF4A;
constexpr uint16_t REG_WX    = 0xFF4B;
constexpr uint16_t REG_KEY1  = 0xFF4D;
constexpr uint16_t REG_VBK   = 0xFF4F;
constexpr uint16_t REG_HDMA1 = 0xFF51;
constexpr uint16_t REG_HDMA2 = 0xFF52;
constexpr uint16_t REG_HDMA3 = 0xFF53;
constexpr uint16_t REG_HDMA4 = 0xFF54;
constexpr uint16_t REG_HDMA5 = 0xFF55;
constexpr uint16_t REG_BCPS  = 0xFF68;
constexpr uint16_t REG_BCPD  = 0xFF69;
constexpr uint16_t REG_OCPS  = 0xFF6A;
constexpr uint16_t REG_OCPD  = 0xFF6B;
constexpr uint16_t REG_SVBK  = 0xFF70;

// display dimensions and scanline counts
constexpr int SCREEN_WIDTH        = 160;
constexpr int SCREEN_HEIGHT       = 144;
constexpr int TOTAL_SCANLINES     = 154;
constexpr int VBLANK_START_LINE   = 144;

// timing and clock frequencies
constexpr uint32_t CPU_CLOCK_HZ       = 4194304;
constexpr uint32_t CGB_CPU_CLOCK_HZ   = 8388608;
constexpr int CYCLES_PER_SCANLINE     = 456;
constexpr int CYCLES_PER_FRAME        = 70224;
constexpr double FRAME_RATE_HZ        = 59.7275;

}  // namespace gb

#endif
