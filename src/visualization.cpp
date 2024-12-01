#include "visualization.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

void CacheVisualizer::printResults(const std::vector<Cache::AccessResult>& results, const CacheStats& stats) {
    // Print header
    std::cout << "Index\tTag\tResult\n";

    // Print each access result
    for (const auto& result : results) {
        std::cout << "x" << result.index << "\t"
                  << "x" << result.tag << "\t"
                  << result.hitMiss << "\n";
    }

    // Print statistics
    std::cout << "\nCache Statistics:\n";
    std::cout << "----------------\n";
    std::cout << "Total Accesses: " << stats.totalAccesses << "\n";
    std::cout << "Hits: " << stats.hits << "\n";
    std::cout << "Misses: " << stats.misses << "\n";
    std::cout << "Cold Misses: " << stats.coldMisses << "\n";
    std::cout << "Conflict Misses: " << stats.conflictMisses << "\n";
    std::cout << "Hit Rate: " << (stats.totalAccesses > 0 ? 
        (static_cast<double>(stats.hits) / stats.totalAccesses * 100) : 0) << "%\n";
}

void CacheVisualizer::generateVisualization(
    const std::vector<Cache::AccessResult>& results,
    const CacheStats& stats,
    const std::string& outputFile) {
    try {
        std::ofstream hitRateFile("hit_rate_data.txt");
        for (size_t i = 0; i < stats.hitRateHistory.size(); ++i) {
            hitRateFile << i << " " << stats.hitRateHistory[i] << std::endl;
        }

        std::ofstream accessPatternFile("access_pattern.txt");
        for (size_t i = 0; i < results.size(); ++i) {
            accessPatternFile << i << " "
                              << (results[i].hitMiss == 'H' ? 1 : 0) << " "
                              << results[i].index << " "
                              << results[i].tag << std::endl;
        }

        std::ofstream scriptFile("cache_visualization.gp");
        scriptFile << "set terminal pngcairo enhanced size 1200,800\n";
        scriptFile << "set output '" << outputFile << "'\n";

        // Hit rate plot
        scriptFile << "set title 'Cache Hit Rate Over Time'\n";
        scriptFile << "set xlabel 'Access Number'\n";
        scriptFile << "set ylabel 'Hit Rate (%)'\n";
        scriptFile << "plot 'hit_rate_data.txt' using 1:2 with lines title 'Hit Rate' lw 2\n";

        // Access pattern plot
        scriptFile << "set title 'Access Pattern'\n";
        scriptFile << "set ylabel 'Hit/Miss'\n";
        scriptFile << "set yrange [-0.5:1.5]\n";
        scriptFile << "plot 'access_pattern.txt' using 1:2 with points title 'Accesses'\n";

        scriptFile.close();
        system("gnuplot cache_visualization.gp");

        // Clean up temporary files
        remove("hit_rate_data.txt");
        remove("access_pattern.txt");
        remove("cache_visualization.gp");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
