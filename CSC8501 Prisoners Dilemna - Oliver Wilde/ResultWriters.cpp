#include "ResultWriters.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>

namespace {
    std::vector<std::string> deriveOrder(
        const std::map<std::string, std::map<std::string, double>>& matrix)
    {
        std::vector<std::string> order;
        order.reserve(matrix.size());
        for (auto& kv : matrix) order.push_back(kv.first);
        std::sort(order.begin(), order.end());
        return order;
    }

    template <typename T>
    static double meanOf(const std::vector<T>& v) {
        long long s = 0; for (auto x : v) s += static_cast<long long>(x);
        return v.empty() ? 0.0 : static_cast<double>(s) / static_cast<double>(v.size());
    }
}


void ResultWriters::writeScores(const ScoresMap& scores, int repeats, const std::string& format) {
    if (format == "csv") {
        std::cout << "strategy,mean,repeats\n";
        for (auto& kv : scores) {
            std::cout << kv.first << "," << meanOf(kv.second) << "," << repeats << "\n";
        }
        return;
    }

    if (format == "json") {
        std::cout << "{\n  \"results\": [\n";
        bool first = true;
        for (auto& kv : scores) {
            if (!first) std::cout << ",\n";
            first = false;
            std::cout << "    {\"strategy\":\"" << kv.first << "\","
                << "\"mean\":" << meanOf(kv.second) << ","
                << "\"repeats\":" << repeats << "}";
        }
        std::cout << "\n  ]\n}\n";
        return;
    }

    std::cout << "Strategy                 Mean        Repeats\n";
    for (auto& kv : scores) {
        std::cout << std::left << std::setw(22) << kv.first
            << std::right << std::setw(12) << std::fixed << std::setprecision(3)
            << meanOf(kv.second)
            << std::setw(10) << repeats << "\n";
    }
}


void ResultWriters::writeDistribution(const std::map<std::string, double>& pop,
    const std::string& format,
    int population)
{
    std::vector<std::pair<std::string, double>> items(pop.begin(), pop.end());
    std::sort(items.begin(), items.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    if (format == "csv") {
        if (population > 0) {
            std::cout << "strategy,share,count\n";
            for (auto& kv : items) {
                const int count = static_cast<int>(std::llround(kv.second * population));
                std::cout << kv.first << "," << std::fixed << std::setprecision(6) << kv.second
                    << "," << count << "\n";
            }
        }
        else {
            std::cout << "strategy,share\n";
            for (auto& kv : items) {
                std::cout << kv.first << "," << std::fixed << std::setprecision(6) << kv.second << "\n";
            }
        }
        return;
    }

    if (format == "json") {
        std::cout << "{\n  \"distribution\": [\n";
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& [name, share] = items[i];
            std::cout << "    {\"strategy\":\"" << name << "\","
                << "\"share\":" << std::fixed << std::setprecision(6) << share;
            if (population > 0) {
                const int count = static_cast<int>(std::llround(share * population));
                std::cout << ",\"count\":" << count;
            }
            std::cout << "}";
            if (i + 1 < items.size()) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "  ]\n}\n";
        return;
    }

    // text
    if (population > 0) {
        std::cout << "Final population (N=" << population << ")\n";
        std::cout << "Strategy                 Count        Share        Percent\n";
        for (auto& kv : items) {
            const int count = static_cast<int>(std::llround(kv.second * population));
            std::cout << std::left << std::setw(22) << kv.first
                << std::right << std::setw(8) << count
                << std::setw(13) << std::fixed << std::setprecision(6) << kv.second
                << std::setw(12) << std::fixed << std::setprecision(2) << (kv.second * 100.0) << "%\n";
        }
    }
    else {
        std::cout << "Final population distribution\n";
        std::cout << "Strategy                 Share        Percent\n";
        for (auto& kv : items) {
            std::cout << std::left << std::setw(22) << kv.first
                << std::right << std::setw(12) << std::fixed << std::setprecision(6) << kv.second
                << std::setw(11) << std::fixed << std::setprecision(2) << (kv.second * 100.0) << "%\n";
        }
    }
}

// -----------------------------------------------------------------------------
// Pairwise matrix writers
// -----------------------------------------------------------------------------
void ResultWriters::writePairwiseMatrixText(
    const std::map<std::string, std::map<std::string, double>>& matrix,
    const std::vector<std::string>& maybeOrder)
{
    std::vector<std::string> order = maybeOrder.empty() ? deriveOrder(matrix) : maybeOrder;

    size_t maxName = 8;
    for (auto& n : order) maxName = std::max(maxName, n.size());
    maxName = std::min<size_t>(maxName, 20);

    const int cellW = 9;

    std::cout << "\nPairwise payoff matrix (average per match)\n";
    std::cout << std::setw(static_cast<int>(maxName)) << "" << " ";
    for (auto& col : order) {
        std::cout << std::setw(cellW) << col.substr(0, std::min<size_t>(col.size(), cellW));
    }
    std::cout << "\n";

    for (auto& row : order) {
        std::cout << std::left << std::setw(static_cast<int>(maxName)) << row.substr(0, maxName) << " ";
        for (auto& col : order) {
            if (row == col) {
                std::cout << std::setw(cellW) << "--";
            }
            else {
                double v = 0.0;
                auto itRow = matrix.find(row);
                if (itRow != matrix.end()) {
                    auto itCol = itRow->second.find(col);
                    if (itCol != itRow->second.end()) v = itCol->second;
                }
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << v;
                std::cout << std::setw(cellW) << ss.str();
            }
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void ResultWriters::writePairwiseMatrixCSV(
    const std::map<std::string, std::map<std::string, double>>& matrix,
    const std::vector<std::string>& maybeOrder,
    const std::string& filename)
{
    std::vector<std::string> order = maybeOrder.empty() ? deriveOrder(matrix) : maybeOrder;

    std::ofstream out(filename);
    if (!out) {
        std::cerr << "[error] could not open '" << filename << "' for writing\n";
        return;
    }

    out << "strategy";
    for (auto& col : order) out << "," << col;
    out << "\n";

    for (auto& row : order) {
        out << row;
        for (auto& col : order) {
            if (row == col) {
                out << ",0";
            }
            else {
                double v = 0.0;
                auto itRow = matrix.find(row);
                if (itRow != matrix.end()) {
                    auto itCol = itRow->second.find(col);
                    if (itCol != itRow->second.end()) v = itCol->second;
                }
                out << "," << std::fixed << std::setprecision(3) << v;
            }
        }
        out << "\n";
    }
    out.close();
    std::cout << "[info] wrote pairwise matrix CSV -> " << filename << "\n";
}
