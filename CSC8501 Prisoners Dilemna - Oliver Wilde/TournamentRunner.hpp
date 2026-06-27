#pragma once
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

class Player;

// Shared payoff bundle
struct Payoffs { int T, R, P, S; };

namespace TournamentRunner {
    // Build the roster (already used elsewhere)
    std::vector<std::shared_ptr<Player>>
        buildRoster(const std::vector<std::string>& tokens);

    // Existing single round-robin totals (sum over all opponents per player)
    std::map<std::string, int>
        runRoundRobin(const std::vector<std::shared_ptr<Player>>& roster,
            int rounds, double epsilon, const Payoffs& p,
            std::mt19937& rng);

    
    std::map<std::string, std::map<std::string, double>>
        runRoundRobinPairwise(const std::vector<std::shared_ptr<Player>>& roster,
            int rounds, int repeats, double epsilon, const Payoffs& p,
            std::mt19937& rng);
}
