#pragma once
#include "Strategy.hpp"
#include <string>
#include <array>

class Player; // fwd-declare

// =========================
// Canonical strategies
// =========================

class ALLC : public Strategy {
public:
    std::string name() const override;
    bool makeDecision(int, const Player&) override;
};

class ALLD : public Strategy {
public:
    std::string name() const override;
    bool makeDecision(int, const Player&) override;
};

class TFT : public Strategy {
public:
    std::string name() const override;
    void reset() override;
    bool makeDecision(int, const Player&) override;
    void recordOpponentMove(bool oppC) override;

private:
    bool first = true;
    bool lastOppMove = true;
};

class GRIM : public Strategy {
public:
    std::string name() const override;
    void reset() override;
    bool makeDecision(int, const Player&) override;
    void recordOpponentMove(bool oppC) override;

private:
    bool defectForever = false;
};

class PAVLOV : public Strategy {
public:
    std::string name() const override;
    void reset() override;
    bool makeDecision(int, const Player&) override;
    void recordOpponentMove(bool oppC) override;

private:
    bool first = true;
    bool myLast = true;
    bool oppLast = true;
};

class PROBER : public Strategy {
public:
    std::string name() const override;
    void reset() override;
    bool makeDecision(int, const Player&) override;
    void recordOpponentMove(bool oppC) override;

private:
    int  round = 0;
    bool opponentAlwaysC = true;
    bool oppLast = true;
};


class CTFT : public Strategy {
public:
    std::string name() const override;
    void reset() override;
    bool makeDecision(int, const Player&) override;
    void recordOpponentMove(bool oppC) override;
    void recordMyMove(bool myC) override;          
    void recordMyIntended(bool myIntended) override; 

private:
    bool first = true;
    bool oppLastActual = true;       
    bool iDefectedLastActual = false;
    bool iIntendedLast = true;      
};

// =========================
// New strategies
// =========================

class RND : public Strategy {
public:
    explicit RND(double prob);
    std::string name() const override;
    bool makeDecision(int, const Player&) override;
    void setRNG(std::mt19937* rng) override;

private:
    double p;
    std::mt19937* rng_{ nullptr };
};

class TTFT : public Strategy {
public:
    std::string name() const override;
    void reset() override;
    bool makeDecision(int, const Player&) override;
    void recordOpponentMove(bool oppC) override;

private:
    std::array<bool, 2> lastTwo{ {true, true} };
    int first = 0;
};

class GRUDGE2 : public Strategy {
public:
    std::string name() const override;
    void reset() override;
    bool makeDecision(int, const Player&) override;
    void recordOpponentMove(bool oppC) override;

private:
    int dCount = 0;
};
