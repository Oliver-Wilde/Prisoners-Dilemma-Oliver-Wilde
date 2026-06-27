#pragma once
#include "Config.hpp"

class App {
public:
    explicit App(const Config& cfg);
    int run(); 

private:
    Config cfg_;
};
