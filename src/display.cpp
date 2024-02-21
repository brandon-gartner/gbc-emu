#include "display.h"

#include <iostream>
#include "emulator.h"
#include "joypad.h"

display::display() : window(nullptr), renderer(nullptr), texture(nullptr), scale(3), initialized(false) {}

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
        // Fallback to software renderer if accelerated fails
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
                if (emu->pad) {
                    switch (event.key.keysym.sym) {
                        case SDLK_RIGHT:
                            emu->pad->key_down(joypad::RIGHT);
                            break;
                        case SDLK_LEFT:
                            emu->pad->key_down(joypad::LEFT);
                            break;
                        case SDLK_UP:
                            emu->pad->key_down(joypad::UP);
                            break;
                        case SDLK_DOWN:
                            emu->pad->key_down(joypad::DOWN);
                            break;
                        case SDLK_z:
                        case SDLK_j:
                            emu->pad->key_down(joypad::A);
                            break;
                        case SDLK_x:
                        case SDLK_k:
                            emu->pad->key_down(joypad::B);
                            break;
                        case SDLK_RETURN:
                            emu->pad->key_down(joypad::START);
                            break;
                        case SDLK_BACKSPACE:
                        case SDLK_RSHIFT:
                        case SDLK_SPACE:
                            emu->pad->key_down(joypad::SELECT);
                            break;
                        case SDLK_ESCAPE:
                            emu->set_status(emulator::status::stopped);
                            break;
                    }
                }
                break;

            case SDL_KEYUP:
                if (emu->pad) {
                    switch (event.key.keysym.sym) {
                        case SDLK_RIGHT:
                            emu->pad->key_up(joypad::RIGHT);
                            break;
                        case SDLK_LEFT:
                            emu->pad->key_up(joypad::LEFT);
                            break;
                        case SDLK_UP:
                            emu->pad->key_up(joypad::UP);
                            break;
                        case SDLK_DOWN:
                            emu->pad->key_up(joypad::DOWN);
                            break;
                        case SDLK_z:
                        case SDLK_j:
                            emu->pad->key_up(joypad::A);
                            break;
                        case SDLK_x:
                        case SDLK_k:
                            emu->pad->key_up(joypad::B);
                            break;
                        case SDLK_RETURN:
                            emu->pad->key_up(joypad::START);
                            break;
                        case SDLK_BACKSPACE:
                        case SDLK_RSHIFT:
                        case SDLK_SPACE:
                            emu->pad->key_up(joypad::SELECT);
                            break;
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
