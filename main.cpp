// main.cpp
#include <iostream>
#include "00 empty project/app.hpp"

int main() {
    try {
        // Instantiate the app inside main to safely catch constructor exceptions
        App app;

        if (app.init()) {
            return app.run();
        }
    }
    catch (std::exception const& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}