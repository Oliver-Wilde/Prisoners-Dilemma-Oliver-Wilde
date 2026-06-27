#pragma once
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <random>

class Player;

class Tournament {
public:
    Tournament() = default;

    // Add a player (shared so caller can reuse if needed; names are read from Player)
    void addPlayer(std::shared_ptr<Player> p);

    
    std::map<std::string, int> run(int rounds,
        double epsilon,
        int T, int R, int P, int S,
        std::mt19937& rng);

    
    void outputResults(const std::map<std::string, std::vector<int>>& scores,
        int repeats,
        const std::string& format) const;

    // Utility: clear roster if needed between runs.
    void clear();

private:
    std::vector<std::shared_ptr<Player>> roster;
};

