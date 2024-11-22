#pragma once
#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <map>
#include <cmath>
#include "cache_entry.hpp"

enum class ReplacementPolicy {
    LRU,
    MRU,
    OPTIMAL,
    RANDOM,
    FIFO,
    PLRU,
    LFU,
    ARC
};

struct CacheStats {
    uint64_t totalAccesses{0};
    uint64_t hits{0};
    uint64_t misses{0};
    uint64_t coldMisses{0};
    uint64_t conflictMisses{0};
    uint64_t capacityMisses{0};
    std::vector<double> hitRateHistory;
    std::map<uint64_t, uint64_t> addressFrequency;
    std::chrono::microseconds totalAccessTime{0};
    
    double getHitRate() const {
        return totalAccesses > 0 ? (static_cast<double>(hits) / totalAccesses) * 100.0 : 0.0;
    }
    
    void recordAccess(bool isHit, bool isCold, bool isConflict) {
        totalAccesses++;
        if (isHit) {
            hits++;
        } else {
            misses++;
            if (isCold) coldMisses++;
            else if (isConflict) conflictMisses++;
            else capacityMisses++;
        }
        
        hitRateHistory.push_back(getHitRate());
    }
};

class CacheSet {
public:
    CacheSet() = default;
    CacheSet(int ways, int blockSize, ReplacementPolicy p);
    
    // Copy constructor
    CacheSet(const CacheSet& other);
    
    // Move constructor
    CacheSet(CacheSet&& other) noexcept;
    
    // Copy assignment operator
    CacheSet& operator=(const CacheSet& other);
    
    // Move assignment operator
    CacheSet& operator=(CacheSet&& other) noexcept;

    bool lookup(uint64_t tag, size_t& wayIndex);
    void updateAccess(size_t wayIndex);
    size_t findVictim(uint64_t newTag);
    
    std::vector<CacheEntry> entries;

private:
    std::shared_ptr<PLRUNode> plruRoot;
    std::vector<uint64_t> fifoQueue;
    std::map<uint64_t, uint64_t> frequencyCount;
    std::random_device rd;
    std::mt19937 gen;
    ReplacementPolicy policy = ReplacementPolicy::LRU;
    uint64_t accessCount = 0;

    size_t findLRUVictim();
    size_t findMRUVictim();
    size_t findRandomVictim();
    size_t findFIFOVictim();
    size_t findPLRUVictim();
    size_t findLFUVictim();
    size_t findARCVictim();
    
    void initializePLRUTree(int ways);
    void initializePLRUNode(PLRUNode* node, int depth);
    void updatePLRUBits(size_t accessedWay);
};