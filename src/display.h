#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <cstdint>

class emulator;

class display {
   public:
    display();
    ~display();

    bool init(const char* title = "gbc-emu", int scale = 3);
    void update(const uint32_t* framebuffer);
    void poll_events(emulator* emu);
    void cleanup();
    bool is_initialized() const { return initialized; }

   private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int scale;
    bool initialized;
};

#endif
