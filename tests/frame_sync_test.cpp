#include <cassert>
#include <chrono>
#include <iostream>
#include "display.h"

void test_normal_sync() {
    display disp;
    if (!disp.init("Sync Test", 1)) {
        std::cout << "Skipping display sync test (no video output available)." << std::endl;
        return;
    }

    // run 30 frames at ~59.7275 fps (expected duration: ~0.502 seconds)
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 30; ++i) {
        disp.sync_frame();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    double fps = 30.0 / diff.count();

    std::cout << "30 synchronized frames took: " << diff.count() << " s (~" << fps << " FPS)" << std::endl;
    assert(diff.count() >= 0.45 && diff.count() <= 0.60);

    // test turbo mode (should be unthrottled / near-zero time)
    disp.set_turbo(true);
    assert(disp.is_turbo());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 30; ++i) {
        disp.sync_frame();
    }
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "30 turbo frames took: " << diff.count() << " s" << std::endl;
    assert(diff.count() < 0.05);

    disp.set_turbo(false);
    assert(!disp.is_turbo());

    disp.cleanup();
    std::cout << "Display frame synchronization verified successfully!" << std::endl;
}

int main() {
    test_normal_sync();
    return 0;
}
