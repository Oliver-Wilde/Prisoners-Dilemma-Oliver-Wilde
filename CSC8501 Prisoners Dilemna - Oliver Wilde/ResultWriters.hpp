#pragma once
#include <map>
#include <string>
#include <vector>

// Keep this alias consistent with the rest of the project
using ScoresMap = std::map<std::string, std::vector<int>>;

namespace ResultWriters {

    
    void writeScores(const ScoresMap& scores, int repeats, const std::string& format);

   
    void writeDistribution(const std::map<std::string, double>& pop,
        const std::string& format,
        int population = 0);

    
    void writePairwiseMatrixText(
        const std::map<std::string, std::map<std::string, double>>& matrix,
        const std::vector<std::string>& order);

    void writePairwiseMatrixCSV(
        const std::map<std::string, std::map<std::string, double>>& matrix,
        const std::vector<std::string>& order,
        const std::string& filename = "pairwise_matrix.csv");
}
