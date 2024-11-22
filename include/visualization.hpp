#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

#include "cache.hpp"
#include <vector>
#include <string>

class CacheVisualizer {
public:
    static void printResults(
        const std::vector<Cache::AccessResult>& results,
        const CacheStats& stats);

    static void generateVisualization(
        const std::vector<Cache::AccessResult>& results,
        const CacheStats& stats,
        const std::string& outputFile);
};

#endif // VISUALIZATION_HPP