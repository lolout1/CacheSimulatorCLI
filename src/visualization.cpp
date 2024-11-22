#include "visualization.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

void CacheVisualizer::printResults(
    const std::vector<Cache::AccessResult>& results,
    const CacheStats& stats) {
    std::cout << "Index\tTag\tH/M" << std::endl;
    std::cout << std::string(30, '-') << std::endl;
    for (const auto& result : results) {
        std::cout << result.indexHex << "\t"
                  << result.tagHex << "\t"
                  << result.hitMiss << std::endl;
    }
    std::cout << "\nCache Statistics:" << std::endl;
    std::cout << "-----------------" << std::endl;
    std::cout << "Total Accesses: " << stats.totalAccesses << std::endl;
    std::cout << "Total Hits: " << stats.hits << std::endl;
    std::cout << "Total Misses: " << stats.misses << std::endl;
    std::cout << "Hit Rate: " << std::fixed << std::setprecision(2)
              << stats.getHitRate() << "%" << std::endl;

    if (stats.misses > 0) {
        std::cout << "\nMiss Breakdown:" << std::endl;
        std::cout << "---------------" << std::endl;
        std::cout << "Cold: " << stats.coldMisses
                  << " (" << (stats.coldMisses * 100.0 / stats.misses) << "%)" << std::endl;
        std::cout << "Conflict: " << stats.conflictMisses
                  << " (" << (stats.conflictMisses * 100.0 / stats.misses) << "%)" << std::endl;
        std::cout << "Capacity: " << stats.capacityMisses
                  << " (" << (stats.capacityMisses * 100.0 / stats.misses) << "%)" << std::endl;
    }
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
                              << results[i].indexHex << " "
                              << results[i].tagHex << std::endl;
        }

        std::ofstream scriptFile("cache_visualization.gp");
        scriptFile << "set terminal pngcairo enhanced size 1200,800\n";
        scriptFile << "set output '" << outputFile << "'\n";
        scriptFile << "set multiplot layout 2,1\n";

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
