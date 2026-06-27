#include "SCBRunner.hpp"
#include "TournamentRunner.hpp"
#include "Player.hpp"
#include "ResultWriters.hpp"

#include <cmath>
#include <map>
#include <random>
#include <string>
#include <vector>

using ScoresMap = std::map<std::string, std::vector<int>>;

static int runOnce_scores_scb(const Config& cfg, double epsOverride /*<0 = use cfg.epsilon*/) {
    Payoffs p{ cfg.T, cfg.R, cfg.P, cfg.S };
    auto roster = TournamentRunner::buildRoster(cfg.strategies);
    std::mt19937 rng(cfg.seed);

    // Collect raw totals across repeats
    ScoresMap raw;
    for (auto& pl : roster) raw[pl->getName()] = {};

    for (int r = 0; r < cfg.repeats; ++r) {
        auto totals = TournamentRunner::runRoundRobin(
            roster, cfg.rounds,
            (epsOverride >= 0.0 ? epsOverride : cfg.epsilon),
            p, rng
        );
        for (auto& kv : totals) raw[kv.first].push_back(kv.second);
    }

    // Apply cost penalties: adjusted = raw / (1 + cost)
    ScoresMap adjusted = raw;
    for (auto& kv : adjusted) {
        const std::string& name = kv.first;
        const double cost = [&] {
            auto it = cfg.scbCost.find(name);
            return (it == cfg.scbCost.end()) ? 0.0 : it->second;
            }();
        const double denom = 1.0 + ((cost > 0.0) ? cost : 0.0);
        for (int& v : kv.second) {
            const double adj = static_cast<double>(v) / denom;
            v = static_cast<int>(std::lround(adj));
        }
    }

    // Output adjusted scores using the configured format
    ResultWriters::writeScores(adjusted, cfg.repeats, cfg.format);
    return 0;
}

int SCBRunner::q5_scb(const Config& cfg) {
    // If no costs provided, just run baseline path to avoid surprising results
    const bool haveAnyCost = !cfg.scbCost.empty();
    if (!haveAnyCost) {
        return runOnce_scores_scb(cfg, -1.0); // behaves like baseline
    }
    // Otherwise run with cost penalties
    return runOnce_scores_scb(cfg, -1.0);
}
