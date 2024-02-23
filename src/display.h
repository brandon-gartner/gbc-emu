#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <cstdint>
#include <string>

class emulator;

class display {
   public:
    display();
    ~display();

    bool init(const char* title = "gbc-emu", int scale = 3);
    void update(const uint32_t* framebuffer);
    void poll_events(emulator* emu);
    void sync_frame();
    void cleanup();

    bool is_initialized() const { return initialized; }
    double get_fps() const { return current_fps; }
    void set_turbo(bool enabled) { turbo_mode = enabled; }
    bool is_turbo() const { return turbo_mode; }

   private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int scale;
    bool initialized;

    std::string base_title;
    uint64_t last_frame_time;
    uint64_t perf_freq;
    uint32_t frame_count;
    uint64_t fps_timer;
    double current_fps;
    bool turbo_mode;
};

#endif
