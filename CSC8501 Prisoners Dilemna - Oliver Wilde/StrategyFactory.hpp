#pragma once
#include <memory>
#include <string>
#include <vector>

class Strategy;

struct StrategyFactory {
    // Create by token, e.g. "ALLC", "TFT", "RND0.3", "RND(0.75)", "CONTRITE", "CTFT", "TTFT", "GRUDGE2"
    static std::shared_ptr<Strategy> create(const std::string& token);

    
    static std::vector<std::string> listRegistered();
};

