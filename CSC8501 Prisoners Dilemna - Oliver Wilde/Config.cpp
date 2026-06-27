#include "Config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

// ---------- helpers (order matters; do not move up consumeListArgs above splitCSV) ----------

static inline std::string trim(std::string s) {
    auto notspace = [](int ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
    return s;
}

static std::vector<std::string> splitCSV(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream iss(s);
    while (std::getline(iss, cur, ',')) {
        cur = trim(cur);
        if (!cur.empty()) out.push_back(cur);
    }
    return out;
}

static bool isFlag(const char* s) {
    return s && s[0] == '-' && s[1] == '-';
}

// Collect subsequent argv items until next flag; also split any CSV chunks.
static std::vector<std::string> consumeListArgs(int& i, int argc, char* argv[]) {
    std::vector<std::string> out;
    while (i + 1 < argc && !isFlag(argv[i + 1])) {
        ++i;
        std::string tok = argv[i];
        auto parts = splitCSV(tok);
        for (auto& p : parts) {
            p = trim(p);
            if (!p.empty()) out.push_back(p);
        }
    }
    return out;
}

static std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return s;
}

static bool starts_with(const std::string& s, const std::string& pref) {
    return s.size() >= pref.size() && std::equal(pref.begin(), pref.end(), s.begin());
}

static bool parse_double_strict(const std::string& token, double& out) {
    try {
        size_t idx = 0;
        out = std::stod(token, &idx);
        return idx == token.size();
    }
    catch (...) {
        return false;
    }
}

// ---------- Config methods ----------

bool Config::validatePayoffs(std::string& msg) const {
    bool ok = true;
    std::ostringstream oss;
    if (!(T > R && R > P && P > S)) {
        ok = false;
        oss << "Invalid ordering: require T>R>P>S (got T=" << T << ",R=" << R << ",P=" << P << ",S=" << S << ")";
    }
    if (!(2 * R > (T + S))) {
        if (!ok) oss << "; ";
        ok = false;
        oss << "Invalid dilemma: require 2R > T+S (got 2R=" << (2 * R) << ", T+S=" << (T + S) << ")";
    }
    msg = oss.str();
    return ok;
}

void Config::save(const std::string& path) const {
    std::ofstream out(path);
    if (!out) { std::cerr << "[warn] Could not open " << path << " for writing\n"; return; }

    out << "rounds=" << rounds << "\n";
    out << "repeats=" << repeats << "\n";
    out << "seed=" << seed << "\n";

    out << "epsilon=" << epsilon << "\n";
    if (!epsilons.empty()) {
        out << "epsilons=";
        for (size_t i = 0; i < epsilons.size(); ++i) {
            out << epsilons[i] << (i + 1 < epsilons.size() ? "," : "");
        }
        out << "\n";
    }

    out << "payoffs=" << T << "," << R << "," << P << "," << S << "\n";

    out << "strategies=";
    for (size_t i = 0; i < strategies.size(); ++i) {
        out << strategies[i];
        if (i + 1 < strategies.size()) out << ",";
    }
    out << "\n";

    out << "format=" << format << "\n";
    out << "evolve=" << (evolve ? 1 : 0) << "\n";
    out << "population=" << population << "\n";
    out << "generations=" << generations << "\n";
    out << "mutation=" << mutation << "\n";

    out << "scb=" << (scb ? 1 : 0) << "\n";
    if (!scbCost.empty()) {
        out << "scb_cost=";
        size_t i = 0, n = scbCost.size();
        for (auto& kv : scbCost) {
            out << kv.first << "=" << kv.second;
            if (++i < n) out << ",";
        }
        out << "\n";
    }
}

void Config::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) { std::cerr << "[warn] Could not open " << path << " for reading\n"; return; }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        if (key == "rounds") rounds = std::stoi(val);
        else if (key == "repeats") repeats = std::stoi(val);
        else if (key == "seed") seed = std::stoi(val);
        else if (key == "epsilon") {
            double e = 0.0;
            if (!parse_double_strict(val, e)) { std::cerr << "[error] epsilon (in file) is not a valid number: '" << val << "'\n"; std::exit(2); }
            if (e < 0.0 || e > 1.0) { std::cerr << "[error] epsilon (in file) must be in [0,1]; got " << e << "\n"; std::exit(2); }
            epsilon = e;
        }
        else if (key == "epsilons") {
            epsilons.clear();
            for (auto& tok : splitCSV(val)) {
                double e = 0.0;
                if (!parse_double_strict(tok, e)) { std::cerr << "[error] epsilons (in file) contains non-numeric value: '" << tok << "'\n"; std::exit(2); }
                if (e < 0.0 || e > 1.0) { std::cerr << "[error] epsilons (in file) value out of range [0,1]: " << e << "\n"; std::exit(2); }
                epsilons.push_back(e);
            }
            if (!epsilons.empty()) epsilon = epsilons.front();
        }
        else if (key == "payoffs") {
            auto v = splitCSV(val);
            if (v.size() == 4) { T = std::stoi(v[0]); R = std::stoi(v[1]); P = std::stoi(v[2]); S = std::stoi(v[3]); }
        }
        else if (key == "strategies") {
            strategies = splitCSV(val);
        }
        else if (key == "format") format = val;
        else if (key == "evolve") evolve = (val == "1" || upper(val) == "TRUE" || upper(val) == "ON");
        else if (key == "population") population = std::stoi(val);
        else if (key == "generations") generations = std::stoi(val);
        else if (key == "mutation") {
            double m = 0.0;
            if (!parse_double_strict(val, m)) { std::cerr << "[error] mutation (in file) is not a valid number: '" << val << "'\n"; std::exit(2); }
            if (m < 0.0 || m > 1.0) { std::cerr << "[error] mutation (in file) must be in [0,1]; got " << m << "\n"; std::exit(2); }
            mutation = m;
        }
        else if (key == "scb") scb = (val == "1" || upper(val) == "ON" || upper(val) == "TRUE");
        else if (key == "scb_cost") {
            scbCost.clear();
            auto items = splitCSV(val);
            for (auto& it : items) {
                auto eq2 = it.find('=');
                if (eq2 == std::string::npos) continue;
                std::string name = trim(it.substr(0, eq2));
                std::string cv = trim(it.substr(eq2 + 1));
                double cost = 0.0;
                if (!parse_double_strict(cv, cost)) {
                    std::cerr << "[error] scb_cost contains non-numeric value for '" << name << "': '" << cv << "'\n";
                    std::exit(2);
                }
                scbCost[name] = cost;
            }
        }
    }
}

// ---------- CLI parsing ----------

Config parseArgs(int argc, char* argv[]) {
    Config cfg;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        auto need = [&](int n) {
            if (i + n >= argc || isFlag(argv[i + 1])) {
                std::cerr << "[error] Missing value for " << arg << "\n";
                std::exit(2);
            }
            };

        if (arg == "--rounds") { need(1); cfg.rounds = std::stoi(argv[++i]); }
        else if (arg == "--repeats") { need(1); cfg.repeats = std::stoi(argv[++i]); }
        else if (arg == "--seed") { need(1); cfg.seed = std::stoi(argv[++i]); }

        else if (arg == "--epsilon") {
            need(1);
            std::string v = trim(argv[++i]);
            cfg.epsilons.clear();

            if (v.find(',') == std::string::npos) {
                double x = 0.0;
                if (!parse_double_strict(v, x)) { std::cerr << "[error] --epsilon scalar value is not a valid number: '" << v << "'\n"; std::exit(2); }
                if (x < 0.0 || x > 1.0) { std::cerr << "[error] epsilon must be in [0,1]; got " << x << "\n"; std::exit(2); }
                cfg.epsilon = x;
                cfg.epsilons = { x };
            }
            else {
                auto toks = splitCSV(v);
                if (toks.empty()) { std::cerr << "[error] --epsilon list parsed to zero entries\n"; std::exit(2); }
                for (auto& t : toks) {
                    double e = 0.0;
                    if (!parse_double_strict(t, e)) { std::cerr << "[error] --epsilon list contains a non-numeric value: '" << t << "'\n"; std::exit(2); }
                    if (e < 0.0 || e > 1.0) { std::cerr << "[error] epsilon value out of range [0,1]: " << e << "\n"; std::exit(2); }
                    cfg.epsilons.push_back(e);
                }
                cfg.epsilon = cfg.epsilons.front();
            }
        }
        else if (arg == "--epsilons") {
            auto toks = consumeListArgs(i, argc, argv); // supports space or CSV
            if (toks.empty()) { std::cerr << "[error] --epsilons needs at least one value\n"; std::exit(2); }
            cfg.epsilons.clear();
            for (auto& t : toks) {
                double e = 0.0;
                if (!parse_double_strict(t, e) || e < 0.0 || e > 1.0) {
                    std::cerr << "[error] epsilon value invalid: " << t << "\n"; std::exit(2);
                }
                cfg.epsilons.push_back(e);
            }
            cfg.epsilon = cfg.epsilons.front();
        }

        else if (arg == "--payoffs") {
            auto toks = consumeListArgs(i, argc, argv);
            if (toks.size() == 1) toks = splitCSV(toks[0]);
            if (toks.size() != 4) {
                std::cerr << "[error] --payoffs expects 4 values: T R P S  (or CSV T,R,P,S)\n";
                std::exit(2);
            }
            cfg.T = std::stoi(toks[0]);
            cfg.R = std::stoi(toks[1]);
            cfg.P = std::stoi(toks[2]);
            cfg.S = std::stoi(toks[3]);

            if (!cfg.force) {
                std::string msg;
                if (!cfg.validatePayoffs(msg)) {
                    std::cerr << "[fatal] " << msg << "  (use --force to bypass)\n";
                    std::exit(2);
                }
            }
        }

        else if (arg == "--strategies") {
            auto toks = consumeListArgs(i, argc, argv);
            if (toks.empty()) { std::cerr << "[error] --strategies needs at least one token\n"; std::exit(2); }
            cfg.strategies = toks;
        }

        else if (arg == "--format") { need(1); cfg.format = trim(argv[++i]); }

        else if (arg == "--evolve") { need(1); cfg.evolve = (std::stoi(argv[++i]) != 0); }
        else if (arg == "--population") { need(1); cfg.population = std::stoi(argv[++i]); }
        else if (arg == "--generations") { need(1); cfg.generations = std::stoi(argv[++i]); }

        else if (arg == "--mutation") {
            need(1);
            std::string mv = argv[++i];
            double mu = 0.0;
            if (!parse_double_strict(mv, mu)) { std::cerr << "[error] mutation is not a valid number: '" << mv << "'\n"; std::exit(2); }
            if (mu < 0.0 || mu > 1.0) { std::cerr << "[error] mutation must be in [0,1]\n"; std::exit(2); }
            cfg.mutation = mu;
        }

        else if (arg == "--save") { need(1); cfg.savePath = std::string(argv[++i]); }
        else if (arg == "--load") { need(1); cfg.loadPath = std::string(argv[++i]); }

        else if (arg == "--verbose") { cfg.verbose = true; }
        else if (arg == "--list-strategies") { cfg.listStrategies = true; }

        else if (arg == "--run-all") {
            cfg.runAll = true;
            cfg.run.clear();
        }
        else if (arg == "--run") {
            auto items = consumeListArgs(i, argc, argv);
            cfg.run.clear();
            cfg.runAll = false;
            for (auto s : items) {
                auto u = upper(s);
                if (u == "ALL") { cfg.runAll = true; cfg.run.clear(); break; }
                cfg.run.push_back(u);
            }
        }

        else if (arg == "--scb") {
            // allow --scb (toggle on) or legacy --scb on|off
            if (i + 1 < argc && !isFlag(argv[i + 1])) {
                std::string v = upper(argv[i + 1]);
                if (v == "ON" || v == "TRUE" || v == "1" || v == "OFF" || v == "FALSE" || v == "0") {
                    ++i;
                    cfg.scb = (v == "ON" || v == "TRUE" || v == "1");
                }
                else {
                    cfg.scb = true;
                }
            }
            else {
                cfg.scb = true;
            }
        }
        else if (arg == "--scb-cost") {
            auto toks = consumeListArgs(i, argc, argv); 
            cfg.scbCost.clear();
            for (auto& chunk : toks) {
                for (auto& kv : splitCSV(chunk)) {
                    auto eq = kv.find('=');
                    if (eq == std::string::npos) continue;
                    auto key = trim(kv.substr(0, eq));
                    auto val = trim(kv.substr(eq + 1));
                    if (!key.empty() && !val.empty()) cfg.scbCost[key] = std::stod(val);
                }
            }
        }

        else if (arg == "--force") { cfg.force = true; }

        else if (arg == "--help" || arg == "-h") {
            std::cout <<
                "Usage:\n"
                "  --rounds N              Rounds per match (default 100)\n"
                "  --repeats N             Repeats per run (default 5)\n"
                "  --seed N                RNG seed\n"
                "  --epsilon E             Noise in [0,1] (CSV ok)\n"
                "  --epsilons E1 E2 ...    Multiple epsilons (space or CSV)\n"
                "  --strategies A B ...    Strategy tokens (space or CSV)\n"
                "  --format F              text|csv|json\n"
                "  --payoffs T R P S       Payoffs (or CSV T,R,P,S)\n"
                "  --evolve 0|1            Enable Q4 evolution\n"
                "  --population N          Evolution population size\n"
                "  --generations G         Evolution generations\n"
                "  --mutation MU           Mutation rate in [0,1]\n"
                "  --run L1 L2 ...         Which to run: Q1 Q2 Q3 Q4 Q5 (or CSV)\n"
                "  --run-all               Run Q1..Q3 (+Q4/Q5 if enabled)\n"
                "  --save/--load FILE      Save/load config file\n"
                "  --scb [on|off]          Strategic Complexity Budget toggle\n"
                "  --scb-cost k=v ...      Per-strategy costs (spaced or CSV)\n"
                "  --force                 Allow invalid payoff tuple\n"
                "  --verbose               Extra logging\n"
                "  --list-strategies       List registered strategies and exit\n";
            std::exit(0);
        }

        else if (starts_with(arg, "--")) {
            std::cerr << "[warn] Unknown flag: " << arg << "\n";
        }
    }

    // Honor load/save if provided
    if (cfg.loadPath)  cfg.load(*cfg.loadPath);
    if (cfg.savePath)  cfg.save(*cfg.savePath);

    return cfg;
}
