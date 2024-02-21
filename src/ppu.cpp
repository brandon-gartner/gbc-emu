#include "ppu.h"

#include <algorithm>
#include <cstring>
#include "databus.h"
#include "emulator.h"
#include "interrupt.h"

ppu::ppu(emulator* emu) : emu(emu) {
    std::memset(this->vram, 0, sizeof(this->vram));
    std::memset(this->oam, 0, sizeof(this->oam));
    std::memset(this->framebuffer, 0, sizeof(this->framebuffer));
    this->frame_ready = false;

    this->lcdc = 0x91;
    this->stat = 0x82;
    this->scy = 0;
    this->scx = 0;
    this->ly = 0;
    this->lyc = 0;
    this->dma = 0xFF;
    this->bgp = 0xFC;
    this->obp0 = 0xFF;
    this->obp1 = 0xFF;
    this->wy = 0;
    this->wx = 0;

    this->scanline_counter = 0;
}

ppu::~ppu() {}

void ppu::set_mode(uint8_t mode) {
    this->stat = (this->stat & 0xFC) | (mode & 0x03);

    // check stat interrupt conditions
    bool request_intr = false;
    if (mode == 0 && (this->stat & 0x08)) {
        // mode 0 hblank interrupt (bit 3)
        request_intr = true;
    } else if (mode == 1 && (this->stat & 0x10)) {
        // mode 1 vblank interrupt (bit 4)
        request_intr = true;
    } else if (mode == 2 && (this->stat & 0x20)) {
        // mode 2 oam interrupt (bit 5)
        request_intr = true;
    }

    if (request_intr && this->emu && this->emu->intr) {
        this->emu->intr->request_interrupt(interrupt::lcd);
    }
}

void ppu::check_lyc() {
    if (this->ly == this->lyc) {
        // set coincidence flag (bit 2)
        this->stat |= 0x04;
        if ((this->stat & 0x40) && this->emu && this->emu->intr) {
            // lyc=ly stat interrupt enable (bit 6)
            this->emu->intr->request_interrupt(interrupt::lcd);
        }
    } else {
        // clear coincidence flag
        this->stat &= ~0x04;
    }
}

void ppu::start_dma(uint8_t value) {
    this->dma = value;
    uint16_t source = ((uint16_t)value) << 8;
    for (int i = 0; i < 0xA0; i++) {
        this->oam[i] = this->emu->bus->read8(source + i);
    }
}

void ppu::tick(int cycles) {
    // if lcd is disabled (bit 7 of lcdc is 0), do not advance scanlines
    if (!(this->lcdc & 0x80)) {
        return;
    }

    // 4 t-cycles per m-cycle
    this->scanline_counter += cycles * 4;

    if (this->scanline_counter >= 456) {
        this->scanline_counter -= 456;
        this->ly++;

        if (this->ly == 144) {
            // enter vblank
            this->set_mode(1);
            this->frame_ready = true;
            if (this->emu && this->emu->intr) {
                this->emu->intr->request_interrupt(interrupt::vblank);
            }
        } else if (this->ly > 153) {
            // frame complete, return to scanline 0
            this->ly = 0;
            this->set_mode(2);
        }

        this->check_lyc();
    }

    // update mode within visible scanlines (0 - 143)
    if (this->ly < 144) {
        if (this->scanline_counter < 80) {
            if ((this->stat & 0x03) != 2) {
                this->set_mode(2);  // mode 2: oam search (80 cycles)
            }
        } else if (this->scanline_counter < 252) {
            if ((this->stat & 0x03) != 3) {
                this->set_mode(3);  // mode 3: pixel transfer (172 cycles)
            }
        } else {
            if ((this->stat & 0x03) != 0) {
                this->set_mode(0);  // mode 0: hblank (204 cycles)
                this->render_scanline();
            }
        }
    }
}

uint8_t ppu::read_vram(uint16_t offset) {
    if (offset < 0x2000) {
        return this->vram[offset];
    }
    return 0xFF;
}

void ppu::write_vram(uint16_t offset, uint8_t value) {
    if (offset < 0x2000) {
        this->vram[offset] = value;
    }
}

uint8_t ppu::read_oam(uint16_t offset) {
    if (offset < 0xA0) {
        return this->oam[offset];
    }
    return 0xFF;
}

void ppu::write_oam(uint16_t offset, uint8_t value) {
    if (offset < 0xA0) {
        this->oam[offset] = value;
    }
}

uint8_t ppu::read_reg(uint16_t address) {
    switch (address) {
        case 0xFF40:
            return this->lcdc;
        case 0xFF41:
            // bit 7 is always 1 on dmg
            return this->stat | 0x80;
        case 0xFF42:
            return this->scy;
        case 0xFF43:
            return this->scx;
        case 0xFF44:
            return this->ly;
        case 0xFF45:
            return this->lyc;
        case 0xFF46:
            return this->dma;
        case 0xFF47:
            return this->bgp;
        case 0xFF48:
            return this->obp0;
        case 0xFF49:
            return this->obp1;
        case 0xFF4A:
            return this->wy;
        case 0xFF4B:
            return this->wx;
        default:
            return 0xFF;
    }
}

void ppu::write_reg(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF40: {
            bool was_enabled = (this->lcdc & 0x80);
            this->lcdc = value;
            bool is_enabled = (this->lcdc & 0x80);
            if (was_enabled && !is_enabled) {
                // lcd disabled: reset ly and scanline counter
                this->ly = 0;
                this->scanline_counter = 0;
                this->set_mode(0);
            } else if (!was_enabled && is_enabled) {
                // lcd enabled
                this->scanline_counter = 0;
                this->check_lyc();
            }
            break;
        }
        case 0xFF41:
            // bits 6-3 are writable; bits 2-0 are read-only; bit 7 is always 1
            this->stat = (this->stat & 0x07) | (value & 0x78) | 0x80;
            break;
        case 0xFF42:
            this->scy = value;
            break;
        case 0xFF43:
            this->scx = value;
            break;
        case 0xFF44:
            // ly is read-only on dmg
            break;
        case 0xFF45:
            this->lyc = value;
            this->check_lyc();
            break;
        case 0xFF46:
            this->start_dma(value);
            break;
        case 0xFF47:
            this->bgp = value;
            break;
        case 0xFF48:
            this->obp0 = value;
            break;
        case 0xFF49:
            this->obp1 = value;
            break;
        case 0xFF4A:
            this->wy = value;
            break;
        case 0xFF4B:
            this->wx = value;
            break;
    }
}

uint32_t ppu::get_dmg_color(uint8_t shade) {
    // Classic DMG green palette shades:
    // shade 0: lightest (pale green / white)
    // shade 1: light green
    // shade 2: dark green
    // shade 3: darkest (near-black)
    static const uint32_t dmg_shades[4] = {
        0xFFE0F8D0,
        0xFF88C070,
        0xFF346856,
        0xFF081820
    };
    return dmg_shades[shade & 0x03];
}

void ppu::render_scanline() {
    if (this->ly >= 144) {
        return;
    }

    uint8_t scanline_color_ids[160] = {0};

    this->render_bg(scanline_color_ids);
    this->render_window(scanline_color_ids);
    this->render_sprites(scanline_color_ids);
}

void ppu::render_bg(uint8_t* scanline_color_ids) {
    // LCDC bit 0: BG & Window display enable
    if (!(this->lcdc & 0x01)) {
        uint32_t blank = get_dmg_color(0);
        for (int x = 0; x < 160; x++) {
            this->framebuffer[this->ly * 160 + x] = blank;
            scanline_color_ids[x] = 0;
        }
        return;
    }

    // BG tile map select: LCDC bit 3 (0 = 0x9800 -> offset 0x1800, 1 = 0x9C00 -> offset 0x1C00)
    uint16_t map_offset = (this->lcdc & 0x08) ? 0x1C00 : 0x1800;
    bool unsigned_tiles = (this->lcdc & 0x10) != 0;

    uint8_t y_pos = this->ly + this->scy;
    uint16_t tile_row = ((uint16_t)(y_pos / 8)) * 32;
    uint8_t tile_y = y_pos % 8;

    for (int x = 0; x < 160; x++) {
        uint8_t x_pos = x + this->scx;
        uint16_t tile_col = x_pos / 8;
        uint8_t tile_x = x_pos % 8;

        uint16_t map_addr = map_offset + tile_row + tile_col;
        uint8_t tile_num = this->vram[map_addr];

        uint16_t tile_data_offset = 0;
        if (unsigned_tiles) {
            tile_data_offset = ((uint16_t)tile_num) * 16 + tile_y * 2;
        } else {
            tile_data_offset = 0x1000 + ((int16_t)(int8_t)tile_num) * 16 + tile_y * 2;
        }

        uint8_t byte1 = this->vram[tile_data_offset];
        uint8_t byte2 = this->vram[tile_data_offset + 1];

        int bit_idx = 7 - tile_x;
        uint8_t color_id = (((byte2 >> bit_idx) & 1) << 1) | ((byte1 >> bit_idx) & 1);

        scanline_color_ids[x] = color_id;
        uint8_t shade = (this->bgp >> (color_id * 2)) & 0x03;
        this->framebuffer[this->ly * 160 + x] = get_dmg_color(shade);
    }
}

void ppu::render_window(uint8_t* scanline_color_ids) {
    // LCDC bit 5: Window enable
    if (!(this->lcdc & 0x20)) {
        return;
    }

    // LCDC bit 0: BG & Window display enable
    if (!(this->lcdc & 0x01)) {
        return;
    }

    if (this->ly < this->wy) {
        return;
    }

    int window_screen_x = (int)this->wx - 7;
    if (window_screen_x >= 160) {
        return;
    }

    // Window tile map select: LCDC bit 6 (0 = 0x9800 -> offset 0x1800, 1 = 0x9C00 -> offset 0x1C00)
    uint16_t map_offset = (this->lcdc & 0x40) ? 0x1C00 : 0x1800;
    bool unsigned_tiles = (this->lcdc & 0x10) != 0;

    uint8_t window_y = this->ly - this->wy;
    uint16_t tile_row = ((uint16_t)(window_y / 8)) * 32;
    uint8_t tile_y = window_y % 8;

    int start_x = (window_screen_x < 0) ? 0 : window_screen_x;
    for (int x = start_x; x < 160; x++) {
        int win_x = x - window_screen_x;
        uint16_t tile_col = win_x / 8;
        uint8_t tile_x = win_x % 8;

        uint16_t map_addr = map_offset + tile_row + tile_col;
        uint8_t tile_num = this->vram[map_addr];

        uint16_t tile_data_offset = 0;
        if (unsigned_tiles) {
            tile_data_offset = ((uint16_t)tile_num) * 16 + tile_y * 2;
        } else {
            tile_data_offset = 0x1000 + ((int16_t)(int8_t)tile_num) * 16 + tile_y * 2;
        }

        uint8_t byte1 = this->vram[tile_data_offset];
        uint8_t byte2 = this->vram[tile_data_offset + 1];

        int bit_idx = 7 - tile_x;
        uint8_t color_id = (((byte2 >> bit_idx) & 1) << 1) | ((byte1 >> bit_idx) & 1);

        scanline_color_ids[x] = color_id;
        uint8_t shade = (this->bgp >> (color_id * 2)) & 0x03;
        this->framebuffer[this->ly * 160 + x] = get_dmg_color(shade);
    }
}

void ppu::render_sprites(uint8_t* scanline_color_ids) {
    // LCDC bit 1: OBJ enable
    if (!(this->lcdc & 0x02)) {
        return;
    }

    int sprite_height = (this->lcdc & 0x04) ? 16 : 8;

    struct sprite_entry {
        int oam_idx;
        int x;
        int y;
        uint8_t tile;
        uint8_t flags;
    };

    sprite_entry visible_sprites[10];
    int sprite_count = 0;

    for (int i = 0; i < 40 && sprite_count < 10; i++) {
        int y = (int)this->oam[i * 4] - 16;
        int x = (int)this->oam[i * 4 + 1] - 8;
        uint8_t tile = this->oam[i * 4 + 2];
        uint8_t flags = this->oam[i * 4 + 3];

        if (this->ly >= y && this->ly < y + sprite_height) {
            visible_sprites[sprite_count++] = {i, x, y, tile, flags};
        }
    }

    // Sort visible sprites by DMG priority: smaller X first; equal X: smaller OAM index first.
    std::sort(visible_sprites, visible_sprites + sprite_count, [](const sprite_entry& a, const sprite_entry& b) {
        if (a.x != b.x) {
            return a.x < b.x;
        }
        return a.oam_idx < b.oam_idx;
    });

    bool sprite_drawn[160] = {false};

    for (int i = 0; i < sprite_count; i++) {
        const auto& spr = visible_sprites[i];

        int sprite_row = this->ly - spr.y;
        if (spr.flags & 0x40) {
            // Y-flip
            sprite_row = sprite_height - 1 - sprite_row;
        }

        uint8_t tile_index = spr.tile;
        if (sprite_height == 16) {
            tile_index &= 0xFE;
        }

        uint16_t tile_data_offset = ((uint16_t)tile_index) * 16 + sprite_row * 2;
        uint8_t byte1 = this->vram[tile_data_offset];
        uint8_t byte2 = this->vram[tile_data_offset + 1];

        uint8_t palette = (spr.flags & 0x10) ? this->obp1 : this->obp0;
        bool bg_priority = (spr.flags & 0x80) != 0;
        bool x_flip = (spr.flags & 0x20) != 0;

        for (int px = 0; px < 8; px++) {
            int screen_x = spr.x + px;
            if (screen_x < 0 || screen_x >= 160) {
                continue;
            }

            if (sprite_drawn[screen_x]) {
                continue;
            }

            int bit_idx = x_flip ? px : (7 - px);
            uint8_t color_id = (((byte2 >> bit_idx) & 1) << 1) | ((byte1 >> bit_idx) & 1);

            if (color_id == 0) {
                continue;
            }

            // Non-transparent pixel takes this X position for sprite priority
            sprite_drawn[screen_x] = true;

            // If BG priority is set and BG is not color 0, BG covers sprite
            if (bg_priority && scanline_color_ids[screen_x] != 0) {
                continue;
            }

            uint8_t shade = (palette >> (color_id * 2)) & 0x03;
            this->framebuffer[this->ly * 160 + screen_x] = get_dmg_color(shade);
        }
    }
}
