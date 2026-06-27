#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

struct Config {
    // Core knobs
    int rounds = 100;
    int repeats = 5;
    int seed = 0;

    
    double epsilon = 0.0;
    std::vector<double> epsilons;         

    // Payoffs (T,R,P,S)
    int T = 5, R = 3, P = 1, S = 0;

    // Strategies & format
    std::vector<std::string> strategies = { "ALLC","ALLD","TFT","GRIM","PAVLOV" };
    std::string format = "text"; // text|csv|json

    // Evolution flags
    bool evolve = false;
    int population = 50;
    int generations = 50;
    double mutation = 0.05;

    // CLI control
    bool verbose = false;
    bool listStrategies = false;
    std::vector<std::string> run;   
    bool runAll = false;
    bool force = false;            

    
    bool scb = false;                       
    std::map<std::string, double> scbCost;  

    // Save/Load
    std::optional<std::string> savePath;
    std::optional<std::string> loadPath;

    // Helpers
    bool validatePayoffs(std::string& msg) const; // T>R>P>S and 2R > T+S
    void save(const std::string& path) const;
    void load(const std::string& path);
};

// CLI
Config parseArgs(int argc, char* argv[]);
