#include "EvolutionRunner.hpp"
#include "TournamentRunner.hpp"
#include "Evolution.hpp"
#include "ResultWriters.hpp"
#include "MetaWriter.hpp"

#include <map>
#include <random>
#include <string>
#include <vector>

int EvolutionRunner::q4_evolve(const Config& cfg) {
    Payoffs p{ cfg.T, cfg.R, cfg.P, cfg.S };
    auto roster = TournamentRunner::buildRoster(cfg.strategies);
    std::mt19937 rng(cfg.seed);

    Evolution evo(cfg.strategies, cfg.mutation);
    for (int g = 0; g < cfg.generations; ++g) {
        auto totals = TournamentRunner::runRoundRobin(roster, cfg.rounds, cfg.epsilon, p, rng);

        std::map<std::string, double> fitness;
        for (auto& kv : totals) fitness[kv.first] = static_cast<double>(kv.second);

        evo.update(fitness);
        if (cfg.verbose && (g % 10 == 0)) evo.print(g);
    }

    ResultWriters::writeDistribution(evo.population, cfg.format, cfg.population);
    MetaWriter::writeEvolutionMeta("Q4_meta.json", cfg, cfg.strategies, p);
    return 0;
}
