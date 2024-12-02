#include "visualization.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>
#include "policies.hpp"

void CacheVisualizer::printResults(const std::vector<Cache::AccessResult>& results, const CacheStats& stats) {
    std::cout << "Cache Configuration:\n";
    std::cout << "Association: " << stats.ways << "-way\n";
    std::cout << "Replacement Policy: " << getPolicyName(stats.policy) << "\n\n";

    const size_t MIN_ADDR = 12;
    const size_t MIN_TAG = 6;
    const size_t MIN_INDEX = 6;
    const size_t MIN_OFFSET = 12;
    const size_t MIN_HM = 4;
    const size_t MIN_REPL = 15;
    const size_t SPACING = 1;

    size_t addrWidth = MIN_ADDR;
    size_t tagWidth = MIN_TAG;
    size_t indexWidth = MIN_INDEX;
    size_t offsetWidth = MIN_OFFSET;
    size_t hmWidth = MIN_HM;
    size_t replWidth = MIN_REPL;

    for (const auto& result : results) {
        addrWidth = std::max(addrWidth, result.originalAddress.length() + SPACING);
        tagWidth = std::max(tagWidth, result.tag.length() + SPACING);
        indexWidth = std::max(indexWidth, result.index.length() + SPACING);
        offsetWidth = std::max(offsetWidth, result.offset.length() + SPACING);
        replWidth = std::max(replWidth, result.replacementInfo.length() + SPACING);
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    size_t termWidth = w.ws_col;
    size_t minWidth = MIN_ADDR + MIN_TAG + MIN_INDEX + MIN_OFFSET + MIN_HM + MIN_REPL + 5;

    if (termWidth < minWidth) {
        addrWidth = MIN_ADDR;
        tagWidth = MIN_TAG;
        indexWidth = MIN_INDEX;
        offsetWidth = MIN_OFFSET;
        hmWidth = MIN_HM;
        replWidth = MIN_REPL;
    }

    auto truncate = [](const std::string& str, size_t width) -> std::string {
        if (str.length() > width - 1) {
            return str.substr(0, width - 2) + ".";
        }
        return str;
    };

    auto formatCol = [](const std::string& str, size_t width) {
        std::stringstream ss;
        ss << std::left << std::setw(width) << str;
        return ss.str();
    };

    std::cout << formatCol("Address", addrWidth)
              << formatCol("Tag", tagWidth)
              << formatCol("Index", indexWidth)
              << formatCol("Offset", offsetWidth)
              << formatCol("H/M", hmWidth)
              << "Replacement\n";

    std::cout << std::string(addrWidth, '-') << " "
              << std::string(tagWidth, '-') << " "
              << std::string(indexWidth, '-') << " "
              << std::string(offsetWidth, '-') << " "
              << std::string(hmWidth, '-') << " "
              << std::string(replWidth, '-') << "\n";

    for (const auto& result : results) {
        std::cout << formatCol(truncate(result.originalAddress, addrWidth), addrWidth)
                  << formatCol(truncate(result.tag, tagWidth), tagWidth)
                  << formatCol(truncate(result.index, indexWidth), indexWidth)
                  << formatCol(truncate(result.offset, offsetWidth), offsetWidth)
                  << formatCol(std::string(1, result.hitMiss), hmWidth)
                  << truncate(result.replacementInfo, replWidth) << "\n";
    }

    std::cout << "\nCache Statistics:\n";
    std::cout << "----------------\n";
    std::cout << "Total Accesses: " << stats.totalAccesses << "\n";
    std::cout << "Hits: " << stats.hits << "\n";
    std::cout << "Misses: " << stats.misses << "\n";
    std::cout << "Cold Misses: " << stats.coldMisses << "\n";
    std::cout << "Conflict Misses: " << stats.conflictMisses << "\n";
    std::cout << "Hit Rate: " << std::fixed << std::setprecision(4) 
              << (stats.totalAccesses > 0 ? 
                  (static_cast<double>(stats.hits) / stats.totalAccesses * 100) : 0)
              << "%\n";
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

        scriptFile << "set title 'Cache Hit Rate Over Time'\n";
        scriptFile << "set xlabel 'Access Number'\n";
        scriptFile << "set ylabel 'Hit Rate (%)'\n";
        scriptFile << "plot 'hit_rate_data.txt' using 1:2 with lines title 'Hit Rate' lw 2\n";

        scriptFile << "set title 'Access Pattern'\n";
        scriptFile << "set ylabel 'Hit/Miss'\n";
        scriptFile << "set yrange [-0.5:1.5]\n";
        scriptFile << "plot 'access_pattern.txt' using 1:2 with points title 'Accesses'\n";

        scriptFile.close();
        system("gnuplot cache_visualization.gp");

        remove("hit_rate_data.txt");
        remove("access_pattern.txt");
        remove("cache_visualization.gp");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
