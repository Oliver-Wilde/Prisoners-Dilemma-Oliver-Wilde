#include "Player.hpp"
#include "Strategy.hpp"

// ---- ctors ----
Player::Player(std::unique_ptr<Strategy> s) : strat_(std::move(s)) {}
Player::Player(std::shared_ptr<Strategy> s_shared) : stratShared_(std::move(s_shared)) {}
Player::Player(std::shared_ptr<Strategy> s_shared, bool) : stratShared_(std::move(s_shared)) {}

// ---- dtor ----
Player::~Player() = default;

// ---- API ----
std::string Player::getName() const {
    if (strat_)       return strat_->name();
    if (stratShared_) return stratShared_->name();
    return "UNKNOWN";
}

void Player::reset() {
    if (strat_)       strat_->reset();
    if (stratShared_) stratShared_->reset();
}

bool Player::makeDecision(int roundIndex, const Player& opponent) {
    if (strat_)       return strat_->makeDecision(roundIndex, opponent);
    return stratShared_->makeDecision(roundIndex, opponent);
}

void Player::recordOpponentMove(bool oppCooperated) {
    if (strat_)            strat_->recordOpponentMove(oppCooperated);
    else if (stratShared_) stratShared_->recordOpponentMove(oppCooperated);
}

void Player::recordMyMove(bool myCooperated) {
    if (strat_)            strat_->recordMyMove(myCooperated);
    else if (stratShared_) stratShared_->recordMyMove(myCooperated);
}

void Player::recordMyIntended(bool myIntended) {
    if (strat_)            strat_->recordMyIntended(myIntended);
    else if (stratShared_) stratShared_->recordMyIntended(myIntended);
}

void Player::notifyRoundEnd(bool myCooperate, bool oppCooperate) {
    // forward both notifications in correct order
    recordMyMove(myCooperate);
    recordOpponentMove(oppCooperate);
}

void Player::setRNG(std::mt19937* rng) {
    if (strat_)            strat_->setRNG(rng);
    else if (stratShared_) stratShared_->setRNG(rng);
}
