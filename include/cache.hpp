#pragma once
#include <vector>
#include <string>
#include <memory>
#include "cache_set.hpp"
#include "address_parser.hpp"

class Cache {
public:
    struct AccessResult {
        std::string originalAddress;
        std::string index;
        std::string tag;
        std::string offset;
        char hitMiss;
        bool isColdMiss;
        bool isConflictMiss;
        int replacedWay;
        std::string replacementInfo;
    };

    Cache(int N, int B, int I, int ways = 1, ReplacementPolicy policy = ReplacementPolicy::LRU);
    AccessResult access(const std::string& addrStr);
    const CacheStats& getStats() const;

    // Getter for sets to support OPTIMAL policy preprocessing
    const std::vector<CacheSet>& getSets() const { return sets; }
    std::vector<CacheSet>& getSets() { return sets; }

private:
    const int N;
    const int blockSize;
    const int numSets;
    const int ways;
    std::vector<CacheSet> sets;
    AddressParser parser;
    ReplacementPolicy policy;
    CacheStats stats;
    
    std::string formatHex(uint64_t value) const;
};