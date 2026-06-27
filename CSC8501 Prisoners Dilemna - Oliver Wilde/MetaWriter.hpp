#pragma once
#include "Config.hpp"
#include "TournamentRunner.hpp"  // for Payoffs

#include <string>
#include <vector>

namespace MetaWriter {

    
    void writeScoresMeta(const std::string& filename,
        const Config& cfg,
        const std::string& label,
        double epsilonUsed,
        const std::vector<std::string>& strategies,
        const Payoffs& p);

   
    void writeEvolutionMeta(const std::string& filename,
        const Config& cfg,
        const std::vector<std::string>& strategies,
        const Payoffs& p);
}
