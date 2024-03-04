#include "display.h"

#include <iostream>
#include "emulator.h"
#include "joypad.h"

display::display()
    : window(nullptr),
      renderer(nullptr),
      texture(nullptr),
      scale(3),
      initialized(false),
      base_title("gbc-emu"),
      last_frame_time(0),
      perf_freq(0),
      frame_count(0),
      fps_timer(0),
      current_fps(0.0),
      turbo_mode(false) {}

display::~display() {
    this->cleanup();
}

bool display::init(const char* title, int scale) {
    if (this->initialized) {
        return true;
    }

    this->scale = scale;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    int width = 160 * scale;
    int height = 144 * scale;

    this->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );

    if (!this->window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    this->renderer = SDL_CreateRenderer(
        this->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!this->renderer) {
        // fallback to software renderer if accelerated fails
        this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_SOFTWARE);
        if (!this->renderer) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(this->window);
            this->window = nullptr;
            SDL_Quit();
            return false;
        }
    }

    this->texture = SDL_CreateTexture(
        this->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        160,
        144
    );

    if (!this->texture) {
        std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(this->renderer);
        this->renderer = nullptr;
        SDL_DestroyWindow(this->window);
        this->window = nullptr;
        SDL_Quit();
        return false;
    }

    this->base_title = title;
    this->perf_freq = SDL_GetPerformanceFrequency();
    this->last_frame_time = SDL_GetPerformanceCounter();
    this->fps_timer = this->last_frame_time;
    this->frame_count = 0;
    this->current_fps = 59.7;
    this->turbo_mode = false;

    this->initialized = true;
    return true;
}

void display::update(const uint32_t* framebuffer) {
    if (!this->initialized || !this->renderer || !this->texture || !framebuffer) {
        return;
    }

    SDL_UpdateTexture(this->texture, nullptr, framebuffer, 160 * sizeof(uint32_t));
    SDL_RenderClear(this->renderer);
    SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr);
    SDL_RenderPresent(this->renderer);
}

void display::sync_frame() {
    uint64_t now = SDL_GetPerformanceCounter();

    if (!this->turbo_mode && this->perf_freq > 0) {
        // game boy runs at 4194304 hz / 70224 cycles per frame = ~59.7275 hz
        double target_ticks = (double)this->perf_freq / 59.7275;
        double elapsed_ticks = (double)(now - this->last_frame_time);

        if (elapsed_ticks < target_ticks) {
            double remaining_ms = (target_ticks - elapsed_ticks) * 1000.0 / (double)this->perf_freq;
            // sleep if more than 2ms remaining to save cpu
            if (remaining_ms > 2.0) {
                SDL_Delay((Uint32)(remaining_ms - 1.0));
            }
            // spin wait for remainder
            while ((double)(SDL_GetPerformanceCounter() - this->last_frame_time) < target_ticks) {
                // spin
            }
        }
    }

    now = SDL_GetPerformanceCounter();
    this->last_frame_time = now;
    this->frame_count++;

    // update fps once per second
    if (this->perf_freq > 0) {
        uint64_t diff = now - this->fps_timer;
        if (diff >= this->perf_freq) {
            double seconds = (double)diff / (double)this->perf_freq;
            this->current_fps = (double)this->frame_count / seconds;
            this->frame_count = 0;
            this->fps_timer = now;

            if (this->window) {
                char title_buf[128];
                snprintf(title_buf, sizeof(title_buf), "%s [%.1f FPS%s]", 
                         this->base_title.c_str(), this->current_fps, 
                         this->turbo_mode ? " - TURBO" : "");
                SDL_SetWindowTitle(this->window, title_buf);
            }
        }
    }
}

int display::map_key_to_button(SDL_Keycode key) {
    switch (key) {
        case SDLK_RIGHT:
            return joypad::RIGHT;
        case SDLK_LEFT:
            return joypad::LEFT;
        case SDLK_UP:
            return joypad::UP;
        case SDLK_DOWN:
            return joypad::DOWN;
        case SDLK_z:
        case SDLK_j:
            return joypad::A;
        case SDLK_x:
        case SDLK_k:
            return joypad::B;
        case SDLK_RETURN:
            return joypad::START;
        case SDLK_BACKSPACE:
        case SDLK_RSHIFT:
        case SDLK_SPACE:
            return joypad::SELECT;
        default:
            return -1;
    }
}

void display::poll_events(emulator* emu) {
    if (!emu) {
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                emu->set_status(emulator::status::stopped);
                break;

            case SDL_KEYDOWN:
                if (event.key.repeat) {
                    break;
                }
                if (event.key.keysym.sym == SDLK_TAB) {
                    this->turbo_mode = true;
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    emu->set_status(emulator::status::stopped);
                } else if (emu->pad) {
                    int btn = map_key_to_button(event.key.keysym.sym);
                    if (btn != -1) {
                        emu->pad->key_down((joypad::button)btn);
                    }
                }
                break;

            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_TAB) {
                    this->turbo_mode = false;
                } else if (emu->pad) {
                    int btn = map_key_to_button(event.key.keysym.sym);
                    if (btn != -1) {
                        emu->pad->key_up((joypad::button)btn);
                    }
                }
                break;
        }
    }
}

void display::cleanup() {
    if (!this->initialized) {
        return;
    }

    if (this->texture) {
        SDL_DestroyTexture(this->texture);
        this->texture = nullptr;
    }
    if (this->renderer) {
        SDL_DestroyRenderer(this->renderer);
        this->renderer = nullptr;
    }
    if (this->window) {
        SDL_DestroyWindow(this->window);
        this->window = nullptr;
    }
    SDL_Quit();
    this->initialized = false;
}
