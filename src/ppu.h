#ifndef PPU_H
#define PPU_H

#include <cstdint>

class emulator;

class ppu {
   public:
    ppu(emulator* emu);
    ~ppu();

    void tick(int cycles);
    uint8_t read_vram(uint16_t offset);
    void write_vram(uint16_t offset, uint8_t value);
    uint8_t read_oam(uint16_t offset);
    void write_oam(uint16_t offset, uint8_t value);
    uint8_t read_reg(uint16_t address);
    void write_reg(uint16_t address, uint8_t value);

    const uint32_t* get_framebuffer() const { return framebuffer; }
    bool is_frame_ready() const { return frame_ready; }
    void clear_frame_ready() { frame_ready = false; }

   private:
    emulator* emu;
    uint8_t vram[0x2000];
    uint8_t oam[0xA0];
    uint32_t framebuffer[160 * 144];
    bool frame_ready;

    // ppu registers (0xFF40 - 0xFF4B)
    uint8_t lcdc;
    uint8_t stat;
    uint8_t scy;
    uint8_t scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t dma;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wy;
    uint8_t wx;

    int scanline_counter;

    void set_mode(uint8_t mode);
    void check_lyc();
    void start_dma(uint8_t value);

    void render_scanline();
    void render_bg(uint8_t* scanline_color_ids);
    void render_window(uint8_t* scanline_color_ids);
    void render_sprites(uint8_t* scanline_color_ids);
    uint32_t get_dmg_color(uint8_t shade);
};

#endif
