#include "cache.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <cmath>
#include <bitset>

Cache::Cache(int N, int B, int I, int ways, ReplacementPolicy policy)
    : N(N),
      blockSize(static_cast<int>(std::pow(2, std::ceil(std::log2(B))))),
      numSets(static_cast<int>(std::pow(2, std::ceil(std::log2(I))))),
      ways(ways),
      parser(N, blockSize, numSets),
      policy(policy) {
    
    // Update stats with the configuration
    stats.setConfiguration(ways, policy);

    if (N <= 0 || blockSize <= 0 || numSets <= 0 || ways <= 0) {
        throw std::invalid_argument("Cache parameters must be positive");
    }
    
    // Initialize sets with proper constructor
    sets.reserve(numSets);
    for (int i = 0; i < numSets; ++i) {
        sets.emplace_back(ways, blockSize, policy);
    }

    // Set configuration in stats
}

Cache::AccessResult Cache::access(const std::string& addrStr) {
    auto start = std::chrono::high_resolution_clock::now();

    auto parsed = parser.parseAddress(addrStr);
    size_t wayIndex;
    bool hit = sets[parsed.index].lookup(parsed.tag, wayIndex);
    bool isColdMiss = !hit && parsed.isColdAccess;
    bool isConflictMiss = !hit && !parsed.isColdAccess;

    std::string replacementInfo;
    int replacedWay = -1;

    if (!hit) {
        wayIndex = sets[parsed.index].findVictim(parsed.tag);
        auto& entry = sets[parsed.index].getEntries()[wayIndex];
        
        // Store replacement info for misses
        replacementInfo = "Replaced way " + std::to_string(wayIndex);
        if (entry.valid) {
            replacementInfo += " (old tag: 0x" + formatHex(entry.tag) + ")";
        }
        
        entry.tag = parsed.tag;
        entry.valid = true;
        replacedWay = wayIndex;
    } else {
        // Store update info for hits
        replacementInfo = "Updated way " + std::to_string(wayIndex);
        replacedWay = wayIndex;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    stats.recordAccess(hit, isColdMiss, isConflictMiss);
    stats.totalAccessTime += duration;
    stats.addressFrequency[parsed.fullAddress]++;

    // Format offset in binary with leading zeros
    std::stringstream ss;
    ss << "0x" << std::setfill('0') << std::setw(8) 
       << std::bitset<8>(static_cast<unsigned char>(parsed.offset)).to_string();
    std::string offsetStr = ss.str();

    return {
        "0x" + formatHex(parsed.fullAddress),  // originalAddress
        "0x" + formatHex(parsed.index),        // index
        "0x" + formatHex(parsed.tag),          // tag
        offsetStr,                      // offset
        hit ? 'H' : 'M',               // hitMiss
        isColdMiss,                    // isColdMiss
        isConflictMiss,                // isConflictMiss
        replacedWay,                   // replacedWay
        replacementInfo                // replacementInfo
    };
}

const CacheStats& Cache::getStats() const {
    return stats;
}

std::string Cache::formatHex(uint64_t value) const {
    std::stringstream ss;
    ss << std::hex << std::uppercase << value;
    return ss.str();
}