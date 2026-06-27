#pragma once
#include <memory>
#include <random>
#include <string>

class Strategy;

class Player {
public:
    explicit Player(std::unique_ptr<Strategy> s);
    explicit Player(std::shared_ptr<Strategy> s_shared);
    Player(std::shared_ptr<Strategy> s_shared, bool); // legacy overload

    ~Player();

    
    std::string getName() const;

    
    void reset();

    
    bool makeDecision(int roundIndex, const Player& opponent);

   
    void recordOpponentMove(bool oppCooperated);
    void recordMyMove(bool myCooperated);

   
    void recordMyIntended(bool myIntended);

    
    void notifyRoundEnd(bool myCooperate, bool oppCooperate);

    
    void setRNG(std::mt19937* rng);

private:
    
    std::unique_ptr<Strategy> strat_;
    std::shared_ptr<Strategy> stratShared_;
};
