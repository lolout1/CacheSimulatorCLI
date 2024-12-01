#pragma once
#include <vector>
#include <string>
#include <memory>
#include "cache_set.hpp"
#include "address_parser.hpp"

class Cache {
public:
    struct AccessResult {
        std::string index;
        std::string tag;
        char hitMiss;
        std::string binaryRep;
        uint64_t accessTime;
        bool isColdMiss;
        bool isConflictMiss;
        std::string originalAddress;  // Add this new field
    };

    Cache(int N, int B, int I, int ways = 1, ReplacementPolicy policy = ReplacementPolicy::LRU);
    AccessResult access(const std::string& addrStr);
    const CacheStats& getStats() const;

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