#include "SweepRunner.hpp"
#include "TournamentRunner.hpp"
#include "Player.hpp"            
#include "ResultWriters.hpp"
#include "MetaWriter.hpp"
#include "Tournament.hpp"        

#include <iostream>              
#include <iomanip>
#include <sstream>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <set>

using ScoresMap = std::map<std::string, std::vector<int>>;

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------

static int runOnce_scores_withPayoffs(const Config& cfg,
    double epsOverride,           // <0 => use cfg.epsilon
    const Payoffs& p)             // explicit payoff set
{
    auto roster = TournamentRunner::buildRoster(cfg.strategies);
    std::mt19937 rng(cfg.seed);

    ScoresMap scores;
    for (auto& pl : roster) scores[pl->getName()] = {};

    for (int r = 0; r < cfg.repeats; ++r) {
        auto totals = TournamentRunner::runRoundRobin(
            roster, cfg.rounds,
            (epsOverride >= 0.0 ? epsOverride : cfg.epsilon),
            p, rng
        );
        for (auto& kv : totals) scores[kv.first].push_back(kv.second);
    }

    // Route through Tournament::outputResults so every format includes mean & 95% CI
    {
        Tournament t;
        t.outputResults(scores, cfg.repeats, cfg.format);
    }
    return 0;
}

static int runOnce_scores(const Config& cfg, double epsOverride /*<0 = use cfg.epsilon*/) {
    Payoffs p{ cfg.T, cfg.R, cfg.P, cfg.S };
    return runOnce_scores_withPayoffs(cfg, epsOverride, p);
}

// ------------------------------------------------------------
// Q1–Q3 public API
// ------------------------------------------------------------

int SweepRunner::q1_baseline(const Config& cfg) {
    // 1) CI95 leaderboard
    int code = runOnce_scores(cfg, -1.0);

    // 2) Pairwise payoff matrix (average per match across repeats)
    auto roster = TournamentRunner::buildRoster(cfg.strategies);
    std::mt19937 rng(cfg.seed);
    Payoffs p{ cfg.T, cfg.R, cfg.P, cfg.S };
    auto matrix = TournamentRunner::runRoundRobinPairwise(
        roster, cfg.rounds, cfg.repeats, cfg.epsilon, p, rng);

    // Preserve CLI order for rows/cols
    std::vector<std::string> order = cfg.strategies;

    if (cfg.format == "csv") {
        ResultWriters::writePairwiseMatrixCSV(matrix, order, "pairwise_matrix.csv");
    }
    else {
        ResultWriters::writePairwiseMatrixText(matrix, order);
    }

    // 3) Meta
    MetaWriter::writeScoresMeta("Q1_meta.json", cfg, "Q1", cfg.epsilon, cfg.strategies, p);
    return code;
}

int SweepRunner::q2_noiseSweep(const Config& cfg) {
    if (cfg.epsilons.empty()) {
        int code = runOnce_scores(cfg, cfg.epsilon);
        Payoffs p{ cfg.T, cfg.R, cfg.P, cfg.S };
        MetaWriter::writeScoresMeta("Q2_meta.json", cfg, "Q2", cfg.epsilon, cfg.strategies, p);
        return code;
    }
    for (double e : cfg.epsilons) {
        std::cout << "\n[epsilon=" << e << "]\n";
        int code = runOnce_scores(cfg, e);
        Payoffs p{ cfg.T, cfg.R, cfg.P, cfg.S };
        std::ostringstream fn; fn << "Q2_e" << std::setprecision(6) << e << "_meta.json";
        MetaWriter::writeScoresMeta(fn.str(), cfg, "Q2", e, cfg.strategies, p);
        if (code) return code;
    }
    return 0;
}


int SweepRunner::q3_payoffSweep(const Config& cfg) {
    Payoffs p{ cfg.T, cfg.R, cfg.P, cfg.S };

    // Pass 1: epsilon = 0.00
    {
        std::cout << "\n[Q3 exploiter test | epsilon=0.00]\n";
        int code = runOnce_scores_withPayoffs(cfg, /*epsOverride*/ 0.0, p);
        if (code) return code;
        MetaWriter::writeScoresMeta("Q3_e0.00_meta.json", cfg, "Q3", 0.0, cfg.strategies, p);

        
    }

    // Pass 2: epsilon = cfg.epsilon (if different from 0)
    if (cfg.epsilon != 0.0) {
        std::cout << "\n[Q3 exploiter test | epsilon=" << cfg.epsilon << "]\n";
        int code = runOnce_scores_withPayoffs(cfg, /*epsOverride*/ cfg.epsilon, p);
        if (code) return code;

        std::ostringstream fn; fn << "Q3_e" << std::setprecision(6) << cfg.epsilon << "_meta.json";
        MetaWriter::writeScoresMeta(fn.str(), cfg, "Q3", cfg.epsilon, cfg.strategies, p);

        
    }

    return 0;
}
