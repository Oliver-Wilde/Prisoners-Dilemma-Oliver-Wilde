#include "Tournament.hpp"
#include "Player.hpp"
#include "Match.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

void Tournament::addPlayer(std::shared_ptr<Player> p) {
    roster.push_back(std::move(p));
}

void Tournament::clear() {
    roster.clear();
}

std::map<std::string, int> Tournament::run(int rounds,
    double epsilon,
    int T, int R, int P, int S,
    std::mt19937& rng)
{
    // Prepare results keyed by player->getName()
    std::map<std::string, int> totals;
    for (auto& p : roster) {
        totals[p->getName()] = 0;
        p->reset();
    }

    
    for (size_t i = 0; i < roster.size(); ++i) {
        for (size_t j = i + 1; j < roster.size(); ++j) {
            Match m(*roster[i], *roster[j], T, R, P, S, epsilon);
            auto result = m.play(rounds, rng); // uses rng for noise flips
            totals[roster[i]->getName()] += result.first;
            totals[roster[j]->getName()] += result.second;
        }
    }
    return totals;
}

void Tournament::outputResults(const std::map<std::string, std::vector<int>>& scores,
    int repeats,
    const std::string& format) const
{
    // Compute mean and 95% CI for each strategy
    struct Row { std::string name; double mean; double ci95; };
    std::vector<Row> rows;
    rows.reserve(scores.size());

    for (auto& kv : scores) {
        const std::string& name = kv.first;
        const auto& vals = kv.second;
        double mean = 0.0;
        for (int v : vals) mean += v;
        mean /= (vals.empty() ? 1.0 : (double)vals.size());

        double stdev = 0.0;
        for (int v : vals) stdev += (v - mean) * (v - mean);
        stdev = std::sqrt(stdev / (vals.empty() ? 1.0 : (double)vals.size()));

        double ci = 1.96 * stdev / std::sqrt((double)repeats);
        rows.push_back({ name, mean, ci });
    }

    // Sort by mean desc for consistent leaderboard ordering
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        if (a.mean == b.mean) return a.name < b.name;
        return a.mean > b.mean;
        });

    if (format == "csv") {
        std::cout << "strategy,mean,ci95,repeats\n";
        for (auto& r : rows) {
            std::cout << r.name << "," << r.mean << "," << r.ci95 << "," << repeats << "\n";
        }
    }
    else if (format == "json") {
        std::cout << "{ \"leaderboard\": [\n";
        for (size_t i = 0; i < rows.size(); ++i) {
            const auto& r = rows[i];
            std::cout << "  { \"strategy\": \"" << r.name << "\", \"mean\": " << r.mean
                << ", \"ci95\": " << r.ci95 << ", \"repeats\": " << repeats << " }";
            if (i + 1 < rows.size()) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "] }\n";
    }
    else {
        // default text table
        std::cout << "\nLeaderboard (sorted by mean):\n";
        std::cout << std::left << std::setw(16) << "Strategy"
            << std::right << std::setw(12) << "Mean"
            << std::setw(12) << "CI95"
            << std::setw(10) << "Repeats"
            << "\n";
        for (auto& r : rows) {
            std::cout << std::left << std::setw(16) << r.name
                << std::right << std::setw(12) << std::fixed << std::setprecision(3) << r.mean
                << std::setw(12) << std::fixed << std::setprecision(3) << r.ci95
                << std::setw(10) << repeats
                << "\n";
        }
    }
}
