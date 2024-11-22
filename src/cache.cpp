#include "cache.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <cmath>

Cache::Cache(int N, int B, int I, int ways, ReplacementPolicy policy)
    : N(N),
      blockSize(static_cast<int>(std::pow(2, std::ceil(std::log2(B))))),
      numSets(static_cast<int>(std::pow(2, std::ceil(std::log2(I))))),
      ways(ways),
      parser(N, blockSize, numSets),
      policy(policy) {
    
    if (N <= 0 || blockSize <= 0 || numSets <= 0 || ways <= 0) {
        throw std::invalid_argument("Cache parameters must be positive");
    }
    
    // Initialize sets with proper constructor
    sets.reserve(numSets);
    for (int i = 0; i < numSets; ++i) {
        sets.emplace_back(ways, blockSize, policy);
    }
}

Cache::AccessResult Cache::access(const std::string& addrStr) {
    auto start = std::chrono::high_resolution_clock::now();
    
    auto parsed = parser.parseAddress(addrStr);
    size_t wayIndex;
    bool hit = sets[parsed.index].lookup(parsed.tag, wayIndex);
    bool isColdMiss = !hit && parsed.isColdAccess;
    bool isConflictMiss = !hit && !parsed.isColdAccess;

    if (!hit) {
        wayIndex = sets[parsed.index].findVictim(parsed.tag);
        auto& entry = sets[parsed.index].entries[wayIndex];
        entry.tag = parsed.tag;
        entry.valid = true;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    stats.recordAccess(hit, isColdMiss, isConflictMiss);
    stats.totalAccessTime += duration;
    stats.addressFrequency[parsed.fullAddress]++;

    return {
        formatHex(parsed.index),
        formatHex(parsed.tag),
        hit ? 'H' : 'M',
        parsed.toBinaryString(),
        static_cast<uint64_t>(duration.count()),
        isColdMiss,
        isConflictMiss
    };
}

const CacheStats& Cache::getStats() const {
    return stats;
}

std::string Cache::formatHex(uint64_t value) const {
    std::stringstream ss;
    ss << "x" << std::hex << std::uppercase << value;
    return ss.str();
}