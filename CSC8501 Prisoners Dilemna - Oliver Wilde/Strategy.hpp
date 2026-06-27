#pragma once
#include <string>
#include <random>


class Player;


class Strategy {
public:
    virtual ~Strategy() = default;

    
    virtual std::string name() const = 0;

    
    virtual void reset() {}

    
    
    virtual bool makeDecision(int roundIndex, const Player& opponent) = 0;

    
    virtual void recordOpponentMove(bool opponentCooperated) {}

   
    virtual void recordMyIntended(bool myIntended) {}

   
    virtual void recordMyMove(bool myCooperated) {}

    
    virtual void setRNG(std::mt19937* rng_) { rng = rng_; }

protected:
    
    bool randBernoulli(double p) {
        if (!rng) return p >= 0.5; // conservative fallback (should not happen)
        std::bernoulli_distribution d(p);
        return d(*rng);
    }

    std::mt19937* rng = nullptr;
};
