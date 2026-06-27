#include "App.hpp"
#include "Config.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        Config cfg = parseArgs(argc, argv);
        App app(cfg);
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "[fatal] " << e.what() << "\n";
        return 2;
    }
}
