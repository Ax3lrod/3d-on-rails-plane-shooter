#include "Engine.h"

int main() {
    Engine engine(1280, 720, "Aegis Starfighter - 3D Rail Shooter Engine");
    if (!engine.Init()) {
        return -1;
    }

    engine.Run();
    return 0;
}
