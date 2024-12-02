#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <chrono>
#include "policies.hpp"

class CacheStats {
public:
    // Performance metrics
    uint64_t totalAccesses{0};
    uint64_t hits{0};
    uint64_t misses{0};
    uint64_t coldMisses{0};
    uint64_t conflictMisses{0};
    uint64_t capacityMisses{0};

    // Tracking data
    std::vector<double> hitRateHistory;
    std::map<uint64_t, uint64_t> addressFrequency;
    std::chrono::microseconds totalAccessTime{0};

    // Configuration
    int ways{1};
    ReplacementPolicy policy{ReplacementPolicy::LRU};  // Default to LRU

    // Configure cache statistics
    void setConfiguration(int associativity, ReplacementPolicy replacementPolicy) {
        ways = associativity;
        policy = replacementPolicy;
    }

    // Calculate hit rate
    [[nodiscard]] double getHitRate() const {
        return totalAccesses > 0 
            ? (static_cast<double>(hits) / totalAccesses) * 100.0 
            : 0.0;
    }

    // Record cache access
    void recordAccess(bool isHit, bool isCold = false, bool isConflict = false) {
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

    // Reset statistics
    void reset() {
        totalAccesses = 0;
        hits = 0;
        misses = 0;
        coldMisses = 0;
        conflictMisses = 0;
        capacityMisses = 0;
        hitRateHistory.clear();
        addressFrequency.clear();
        totalAccessTime = std::chrono::microseconds(0);
        policy = ReplacementPolicy::LRU;  // Reset to default
    }
};