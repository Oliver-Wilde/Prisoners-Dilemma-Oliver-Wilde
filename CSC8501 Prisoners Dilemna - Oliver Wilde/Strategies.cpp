#include "Strategies.hpp"
#include "Player.hpp"
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <random>
#include <string>

// ---------------- ALLC ----------------
std::string ALLC::name() const { return "ALLC"; }
bool ALLC::makeDecision(int, const Player&) { return true; }

// ---------------- ALLD ----------------
std::string ALLD::name() const { return "ALLD"; }
bool ALLD::makeDecision(int, const Player&) { return false; }

// ---------------- TFT -----------------
std::string TFT::name() const { return "TFT"; }
void TFT::reset() { lastOppMove = true; first = true; }
bool TFT::makeDecision(int, const Player&) {
    if (first) { first = false; return true; } // cooperate first
    return lastOppMove;
}
void TFT::recordOpponentMove(bool oppC) { lastOppMove = oppC; }

// ---------------- GRIM ----------------
std::string GRIM::name() const { return "GRIM"; }
void GRIM::reset() { defectForever = false; }
bool GRIM::makeDecision(int, const Player&) { return !defectForever; }
void GRIM::recordOpponentMove(bool oppC) { if (!oppC) defectForever = true; }

// ---------------- PAVLOV --------------
std::string PAVLOV::name() const { return "PAVLOV"; }
void PAVLOV::reset() { myLast = true; oppLast = true; first = true; }
bool PAVLOV::makeDecision(int, const Player&) {
    if (first) { first = false; return true; } // start C
    const bool mutual = (myLast == oppLast);
    myLast = (mutual ? myLast : !myLast);
    return myLast;
}
void PAVLOV::recordOpponentMove(bool oppC) { oppLast = oppC; }

// ---------------- PROBER --------------
std::string PROBER::name() const { return "PROBER"; }
void PROBER::reset() { round = 0; opponentAlwaysC = true; oppLast = true; }
bool PROBER::makeDecision(int, const Player&) {
    ++round;
    if (round == 1) return false; // D
    if (round == 2) return true;  // C
    if (round == 3) return true;  // C
    if (round == 4) return false; // D

    if (opponentAlwaysC) return false; // exploit
    // else TFT behavior
    return oppLast;
}
void PROBER::recordOpponentMove(bool oppC) {
    oppLast = oppC;
    if (!oppC) opponentAlwaysC = false;
}

// ---------------- CTFT (Contrite TFT) ----------------
std::string CTFT::name() const { return "CTFT"; }

void CTFT::reset() {
    first = true;
    oppLastActual = true;         // assume C before any information
    iDefectedLastActual = false;  // we haven't defected yet
    iIntendedLast = true;         // default intended C
}


bool CTFT::makeDecision(int, const Player&) {
    if (first) { first = false; return true; }    // start with C

    if (!oppLastActual) {
        if (iDefectedLastActual && iIntendedLast) {
            return true;   // apologise to fix my own mistake
        }
        return false;      // standard retaliation
    }
    return true;           // mirror C with C
}

void CTFT::recordOpponentMove(bool oppC) {
    oppLastActual = oppC;
}

void CTFT::recordMyMove(bool myC) {
    iDefectedLastActual = !myC;
}

void CTFT::recordMyIntended(bool myIntended) {
    iIntendedLast = myIntended;
}

// ---------------- RND(p) --------------
RND::RND(double prob) : p(std::clamp(prob, 0.0, 1.0)) {}
std::string RND::name() const {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "RND(%.3f)", p);
    return std::string(buf);
}
bool RND::makeDecision(int, const Player&) {
    // If RNG wasn't injected, default to 50/50 to avoid UB
    if (!rng_) return p >= 0.5;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(*rng_) < p;
}
void RND::setRNG(std::mt19937* rng) { rng_ = rng; }

// ---------------- TTFT -----------------
std::string TTFT::name() const { return "TTFT"; }
void TTFT::reset() { lastTwo = { true, true }; first = 0; }
bool TTFT::makeDecision(int, const Player&) {
    if (first < 2) { ++first; return true; }
    // Defect only if both of the last two opponent moves were D
    return !((lastTwo[0] == false) && (lastTwo[1] == false));
}
void TTFT::recordOpponentMove(bool oppC) {
    lastTwo[0] = lastTwo[1];
    lastTwo[1] = oppC;
}

// ---------------- GRUDGE2 -------------
std::string GRUDGE2::name() const { return "GRUDGE2"; }
void GRUDGE2::reset() { dCount = 0; }
bool GRUDGE2::makeDecision(int, const Player&) { return dCount < 2; }
void GRUDGE2::recordOpponentMove(bool oppC) { if (!oppC) ++dCount; }
