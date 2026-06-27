#pragma once
#include <map>
#include <string>
#include <vector>
#include <iostream>   


class Evolution {
public:
    explicit Evolution(const std::vector<std::string>& strategies,
        double mutationRate = 0.05);

    void update(const std::map<std::string, double>& fitness);

    // Keep default to std::cout now that <iostream> is included.
    void print(int generation, std::ostream& os = std::cout) const;

    std::map<std::string, double> population;

private:
    double mutation_{ 0.05 };
    void normalize();
    void resetUniform();
};
