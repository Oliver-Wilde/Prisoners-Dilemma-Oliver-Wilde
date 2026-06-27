#include "StrategyFactory.hpp"
#include "Strategies.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

    // trim both ends (spaces/tabs)
    std::string trim(std::string s) {
        auto issp = [](unsigned char c) { return std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char c) { return !issp(c); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char c) { return !issp(c); }).base(), s.end());
        return s;
    }

    std::string upper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return s;
    }

    bool starts_with(const std::string& s, const std::string& pref) {
        return s.size() >= pref.size() && std::equal(pref.begin(), pref.end(), s.begin());
    }

    bool parse_double_strict(const std::string& tok, double& out) {
        try {
            size_t idx = 0;
            out = std::stod(tok, &idx);
            return idx == tok.size();
        }
        catch (...) {
            return false;
        }
    }

    double parse_rnd_prob_or_throw(const std::string& originalTokenUpper) {
        // Handles RND(0.3) or RND0.3
        double p = -1.0;
        const std::string& t = originalTokenUpper;

        if (t.size() > 3 && t[3] == '(' && t.back() == ')') {
            // RND(0.3)
            const std::string inside = t.substr(4, t.size() - 5);
            if (!parse_double_strict(inside, p))
                throw std::runtime_error("Bad RND token; expected RNDx or RND(x) where x is in [0,1]: '" + t + "'");
        }
        else {
            // RND0.3
            const std::string tail = t.substr(3);
            if (tail.empty())
                throw std::runtime_error("Bad RND token; expected RNDx or RND(x) where x is in [0,1]: '" + t + "'");
            if (!parse_double_strict(tail, p))
                throw std::runtime_error("Bad RND token; expected RNDx or RND(x) where x is in [0,1]: '" + t + "'");
        }

        if (!(p >= 0.0 && p <= 1.0))
            throw std::runtime_error("RND probability must be in [0,1], got: " + std::to_string(p));
        return p;
    }

} // namespace

std::shared_ptr<Strategy> StrategyFactory::create(const std::string& token) {
    std::string cleaned = trim(token);
    std::string t = upper(cleaned);

    // Alias
    if (t == "CONTRITE") t = "CTFT";

    // Exact names
    if (t == "ALLC")    return std::make_shared<ALLC>();
    if (t == "ALLD")    return std::make_shared<ALLD>();
    if (t == "TFT")     return std::make_shared<TFT>();
    if (t == "GRIM")    return std::make_shared<GRIM>();
    if (t == "PAVLOV")  return std::make_shared<PAVLOV>();
    if (t == "PROBER")  return std::make_shared<PROBER>();
    if (t == "CTFT")    return std::make_shared<CTFT>();
    if (t == "TTFT")    return std::make_shared<TTFT>();
    if (t == "GRUDGE2") return std::make_shared<GRUDGE2>();

    // RND forms
    if (starts_with(t, "RND")) {
        const double p = parse_rnd_prob_or_throw(t);
        return std::make_shared<RND>(p);
    }

    throw std::runtime_error("Unknown strategy token: '" + token + "'");
}

std::vector<std::string> StrategyFactory::listRegistered() {
    return {
        "ALLC", "ALLD", "TFT", "GRIM", "PAVLOV", "PROBER", "CTFT",
        "RND(p) e.g. RND0.3 or RND(0.3)",
        "TTFT", "GRUDGE2"
    };
}
