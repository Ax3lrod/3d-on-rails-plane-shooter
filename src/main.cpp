#include "Engine.h"
#include <iostream>
#include <exception>

int main() {
    try {
        std::cout << "[Main] Starting Engine..." << std::endl;
        Engine engine(1280, 720, "Aegis Starfighter - 3D Rail Shooter Engine");
        std::cout << "[Main] Initializing Engine..." << std::endl;
        if (!engine.Init()) {
            std::cerr << "[Main] Engine::Init failed!" << std::endl;
            return -1;
        }

        std::cout << "[Main] Running Engine..." << std::endl;
        engine.Run();
        std::cout << "[Main] Engine finished cleanly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[Main] Fatal exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[Main] Unknown fatal exception caught!" << std::endl;
        return 2;
    }
}
