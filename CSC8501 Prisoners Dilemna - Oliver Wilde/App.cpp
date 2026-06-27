#include "App.hpp"

#include "SweepRunner.hpp"
#include "EvolutionRunner.hpp"
#include "SCBRunner.hpp"       
#include "StrategyFactory.hpp"

#include <iostream>
#include <string>
#include <vector>

App::App(const Config& cfg) : cfg_(cfg) {}

int App::run() {
    if (cfg_.listStrategies) {
        for (const auto& s : StrategyFactory::listRegistered()) {
            std::cout << s << "\n";
        }
        return 0;
    }

    std::vector<std::string> runs;
    if (cfg_.runAll || cfg_.run.empty()) {
        runs = { "Q1","Q2","Q3" };
        if (cfg_.evolve) runs.push_back("Q4");
        if (cfg_.scb)    runs.push_back("Q5");
    }
    else {
        runs = cfg_.run;
    }

    for (const auto& label : runs) {
        int code = 0;
        if (label == "Q1") code = SweepRunner::q1_baseline(cfg_);
        else if (label == "Q2") code = SweepRunner::q2_noiseSweep(cfg_);
        else if (label == "Q3") code = SweepRunner::q3_payoffSweep(cfg_);
        else if (label == "Q4") code = EvolutionRunner::q4_evolve(cfg_);
        else if (label == "Q5") code = SCBRunner::q5_scb(cfg_);   // <-- changed
        else {
            std::cerr << "[error] Unknown run label: " << label << "\n";
            return 2;
        }
        if (code != 0) return code;
    }
    return 0;
}
