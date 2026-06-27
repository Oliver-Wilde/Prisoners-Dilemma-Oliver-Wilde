#include "Match.hpp"
#include "Player.hpp"
#include <random>

Match::Match(Player& a, Player& b, int T_, int R_, int P_, int S_, double eps)
    : A(a), B(b), T(T_), R(R_), P(P_), S(S_), epsilon(eps)
{}

std::pair<int, int> Match::play(int rounds, std::mt19937& rng) {
    
    A.reset();
    B.reset();
    A.setRNG(&rng);
    B.setRNG(&rng);

    std::bernoulli_distribution noise(epsilon);

    int scoreA = 0;
    int scoreB = 0;

    for (int r = 0; r < rounds; ++r) {
        // Intended (pre-noise)
        bool mA_intended = A.makeDecision(r, B);
        bool mB_intended = B.makeDecision(r, A);

        // Let strategies know what they intended
        A.recordMyIntended(mA_intended);
        B.recordMyIntended(mB_intended);

        // Actual after noise
        bool mA = mA_intended;
        bool mB = mB_intended;
        if (noise(rng)) mA = !mA;
        if (noise(rng)) mB = !mB;

        
        A.recordMyMove(mA);
        A.recordOpponentMove(mB);
        B.recordMyMove(mB);
        B.recordOpponentMove(mA);

        // Payoffs: C=true, D=false
        if (mA && mB) { scoreA += R; scoreB += R; }
        else if (!mA && !mB) { scoreA += P; scoreB += P; }
        else if (mA && !mB) { scoreA += S; scoreB += T; }
        else { scoreA += T; scoreB += S; }
    }

    return { scoreA, scoreB };
}
