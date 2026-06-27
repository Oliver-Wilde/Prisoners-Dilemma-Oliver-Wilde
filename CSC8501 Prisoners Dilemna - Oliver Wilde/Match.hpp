#pragma once
#include <random>

class Player;

class Match {
public:
    Match(Player& a, Player& b,
        int T, int R, int P, int S,
        double epsilon);

    
    std::pair<int, int> play(int rounds, std::mt19937& rng);

private:
    Player& A;
    Player& B;
    int T, R, P, S;
    double epsilon; 
