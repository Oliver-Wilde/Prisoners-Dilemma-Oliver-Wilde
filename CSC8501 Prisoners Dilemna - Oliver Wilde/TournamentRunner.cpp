#include "TournamentRunner.hpp"
#include "StrategyFactory.hpp"
#include "Player.hpp"
#include "Match.hpp"

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <utility>

// Build roster from strategy tokens
std::vector<std::shared_ptr<Player>>
TournamentRunner::buildRoster(const std::vector<std::string>& tokens) {
    std::vector<std::shared_ptr<Player>> roster;
    roster.reserve(tokens.size());
    for (const auto& t : tokens) {
        auto strat = StrategyFactory::create(t);                  
        roster.emplace_back(std::make_shared<Player>(strat));      
    }
    return roster;
}

// Sum total payoffs per player across a single round-robin
std::map<std::string, int>
TournamentRunner::runRoundRobin(const std::vector<std::shared_ptr<Player>>& roster,
    int rounds, double epsilon, const Payoffs& p,
    std::mt19937& rng) {
    std::map<std::string, int> totals;
    for (const auto& pl : roster) {
        totals[pl->getName()] = 0;
    }

    for (size_t i = 0; i < roster.size(); ++i) {
        for (size_t j = i + 1; j < roster.size(); ++j) {
            Match m(*roster[i], *roster[j], p.T, p.R, p.P, p.S, epsilon);
            auto [a, b] = m.play(rounds, rng);
            totals[roster[i]->getName()] += a;
            totals[roster[j]->getName()] += b;
        }
    }
    return totals;
}


std::map<std::string, std::map<std::string, double>>
TournamentRunner::runRoundRobinPairwise(const std::vector<std::shared_ptr<Player>>& roster,
    int rounds, int repeats, double epsilon, const Payoffs& p,
    std::mt19937& rng) {
    // Prepare names list
    std::vector<std::string> names;
    names.reserve(roster.size());
    for (const auto& pl : roster) names.push_back(pl->getName());

    // Accumulators (ints) then convert to double averages at the end
    std::map<std::string, std::map<std::string, long long>> accum;
    for (const auto& a : names) {
        for (const auto& b : names) {
            if (a == b) continue; // no self-play
            accum[a][b] = 0;
        }
    }

    // Play each unordered pair 'repeats' times and accumulate per-direction scores
    for (int r = 0; r < repeats; ++r) {
        for (size_t i = 0; i < roster.size(); ++i) {
            for (size_t j = i + 1; j < roster.size(); ++j) {
                Match m(*roster[i], *roster[j], p.T, p.R, p.P, p.S, epsilon);
                auto [scoreA, scoreB] = m.play(rounds, rng);

                const std::string& nameA = names[i];
                const std::string& nameB = names[j];

                accum[nameA][nameB] += scoreA;
                accum[nameB][nameA] += scoreB;
            }
        }
    }

    // Convert to averages (per match) as doubles
    std::map<std::string, std::map<std::string, double>> matrix;
    for (const auto& a : names) {
        for (const auto& b : names) {
            if (a == b) continue;
            // Each pair was played exactly 'repeats' times
            matrix[a][b] = repeats > 0 ? static_cast<double>(accum[a][b]) / static_cast<double>(repeats)
                : 0.0;
        }
    }

    // Ensure diagonal exists and is 0.0 for cleanliness (optional)
    for (const auto& a : names) {
        matrix[a][a] = 0.0;
    }

    return matrix;
}
