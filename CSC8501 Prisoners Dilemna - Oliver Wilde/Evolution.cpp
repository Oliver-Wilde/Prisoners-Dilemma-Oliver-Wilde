#include "Evolution.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>

namespace {
    // Tiny epsilon for finite/sanity checks
    constexpr double kTiny = 1e-12;

    // Kahan summation for better numeric stability
    template <typename Iter, typename Getter>
    double kahan_sum(Iter begin, Iter end, Getter get) {
        double sum = 0.0;
        double c = 0.0;
        for (auto it = begin; it != end; ++it) {
            const double y = get(*it) - c;
            const double t = sum + y;
            c = (t - sum) - y;
            sum = t;
        }
        return sum;
    }
}

// ------------------ ctor ------------------

Evolution::Evolution(const std::vector<std::string>& strategies,
    double mutationRate)
{
    // clamp mutation into [0,1]
    mutation_ = std::max(0.0, std::min(1.0, mutationRate));

    const double n = std::max(1.0, static_cast<double>(strategies.size()));
    const double init = 1.0 / n;

    population.clear();
    for (const auto& s : strategies) {
        population[s] = init;
    }
    normalize();
}

// --------------- public: update ------------

void Evolution::update(const std::map<std::string, double>& fitness)
{
    if (population.empty()) return;

    // Compute mean fitness \bar{f} = sum_i s_i * f_i
    const double fbar = kahan_sum(population.begin(), population.end(),
        [&](const auto& kv) {
            const auto it = fitness.find(kv.first);
            const double f = (it == fitness.end()) ? 0.0 : it->second;
            // Treat non-finite or non-positive fitness as zero
            const double f_clamped = (std::isfinite(f) && f > 0.0) ? f : 0.0;
            return kv.second * f_clamped;
        });

    // Degenerate or non-finite mean fitness => reset to uniform
    if (!(fbar > 0.0) || !std::isfinite(fbar)) {
        resetUniform();
        return;
    }

    
    const double N = static_cast<double>(population.size());
    const double uniformShare = (N > 0.0) ? (1.0 / N) : 1.0;

    std::map<std::string, double> next;

    for (const auto& kv : population) {
        const std::string& name = kv.first;
        const double s = kv.second;

        const auto it = fitness.find(name);
        const double f = (it == fitness.end()) ? 0.0 : it->second;
        const double f_clamped = (std::isfinite(f) && f > 0.0) ? f : 0.0;

        const double replicator = (s > 0.0) ? (s * (f_clamped / fbar)) : 0.0;
        const double mutated = (1.0 - mutation_) * replicator + (mutation_)*uniformShare;
        next[name] = std::isfinite(mutated) ? std::max(0.0, mutated) : 0.0;
    }

    population.swap(next);
    normalize();
}

// --------------- public: print -------------

void Evolution::print(int generation, std::ostream& os) const
{
    // Sort by descending share for a nice leaderboard
    std::vector<std::pair<std::string, double>> v(population.begin(), population.end());
    std::sort(v.begin(), v.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    os << "Generation " << generation << " population:\n";
    for (const auto& [name, share] : v) {
        const double pct = 100.0 * share;
        os << "  " << std::setw(12) << name << " : "
            << std::fixed << std::setprecision(2) << pct << "%\n";
    }
}

// --------------- private: normalize --------

void Evolution::normalize()
{
    if (population.empty()) return;

    // clamp small negatives to 0 (can happen after numeric noise)
    for (auto& kv : population) {
        if (!std::isfinite(kv.second) || kv.second < 0.0) kv.second = 0.0;
    }

    double sum = kahan_sum(population.begin(), population.end(),
        [](const auto& kv) { return kv.second; });

    if (!(sum > 0.0) || !std::isfinite(sum)) {
        resetUniform();
        return;
    }

    // Normalize to 1.0
    for (auto& kv : population) {
        kv.second /= sum;
    }

    // Fix residual so the sum is exactly 1.0 (avoids drift in tight tests)
    const double check = kahan_sum(population.begin(), population.end(),
        [](const auto& kv) { return kv.second; });
    const double resid = 1.0 - check;

    // Add the residual to the max entry to keep all values in [0,1]
    auto it_max = std::max_element(population.begin(), population.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
    if (it_max != population.end()) {
        it_max->second = std::max(0.0, it_max->second + resid);
    }

    // Final guard
    const double final = kahan_sum(population.begin(), population.end(),
        [](const auto& kv) { return kv.second; });
    if (!(std::abs(final - 1.0) <= 1e-12)) {
        // Worst-case fallback to uniform (extreme numeric edge)
        resetUniform();
    }
}



void Evolution::resetUniform()
{
    const double N = std::max(1.0, static_cast<double>(population.size()));
    const double eq = 1.0 / N;
    for (auto& kv : population) kv.second = eq;
}
