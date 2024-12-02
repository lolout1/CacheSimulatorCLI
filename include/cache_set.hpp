#pragma once

#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <map>
#include <unordered_map>
#include <cmath>
#include <deque>
#include <limits>
#include <algorithm>

#include "cache_entry.hpp"
#include "policies.hpp"
#include "stats.hpp"

class CacheSet {
private:
    // Core cache storage and configuration
    std::vector<CacheEntry> entries;
    ReplacementPolicy policy;
    uint64_t accessCount{0};
    
    // Trace analysis for optimal replacement
    struct FutureAccess {
        std::vector<size_t> positions;    // All positions where this address appears
        
        // Get next access position after given position
        size_t getNextAccess(size_t currentPos) const {
            auto it = std::lower_bound(positions.begin(), positions.end(), currentPos + 1);
            return it != positions.end() ? *it : std::numeric_limits<size_t>::max();
        }
    };
    
    // Enhanced cache state tracking
    struct CacheState {
        size_t currentPosition{0};          // Current position in trace
        std::vector<uint64_t> fullTrace;    // Complete memory trace
        int offsetBits{0};                  // Number of bits for block offset
        int indexBits{0};                   // Number of bits for set index
        std::unordered_map<uint64_t, FutureAccess> futureMap;  // Tag -> future accesses map
    } state;

    // Support for other replacement policies
    mutable std::mt19937 gen{std::random_device{}()};
    mutable std::uniform_int_distribution<size_t> dis;
    std::deque<size_t> fifoQueue;

    // Replacement policy implementations
    [[nodiscard]] size_t findLRUVictim() const;
    [[nodiscard]] size_t findMRUVictim() const;
    [[nodiscard]] size_t findFIFOVictim() const;
    [[nodiscard]] size_t findRandomVictim() const;
    [[nodiscard]] size_t findOptimalVictim(uint64_t newTag);
    
    // Optimal policy helpers
    [[nodiscard]] size_t getNextAccess(uint64_t tag) const;
    void updateOptimalState();
    void preprocessTrace(const std::vector<uint64_t>& trace);

public:
    // Constructors and assignment operators
    CacheSet() = default;
    explicit CacheSet(int ways, int blockSize, ReplacementPolicy p);
    CacheSet(const CacheSet& other);
    CacheSet(CacheSet&& other) noexcept;
    CacheSet& operator=(const CacheSet& other);
    CacheSet& operator=(CacheSet&& other) noexcept;

    // Core cache operations
    [[nodiscard]] bool lookup(uint64_t tag, size_t& wayIndex);
    void insert(uint64_t tag);
    size_t findVictim(uint64_t newTag);
    void setOptimalTrace(const std::vector<uint64_t>& trace);

    // Accessors
    [[nodiscard]] std::vector<CacheEntry>& getEntries() { return entries; }
    [[nodiscard]] const std::vector<CacheEntry>& getEntries() const { return entries; }
    
    friend class Cache;
};