#pragma once
#include <vector>
#include <chrono>
#include <cstdint>
#include <memory>
#include "policies.hpp"

// Forward declaration of PLRUNode for header organization
struct PLRUNode;

class CacheEntry {
public:
    // Cache line data
    uint64_t tag;
    bool valid;
    bool dirty;
    uint64_t lastUsed;
    uint64_t accessCount;
    std::chrono::system_clock::time_point insertionTime;
    std::vector<uint8_t> data;

    // Constructor
    explicit CacheEntry(int blockSize) 
        : tag(0)
        , valid(false)
        , dirty(false)
        , lastUsed(0)
        , accessCount(0)
        , insertionTime(std::chrono::system_clock::now())
        , data(blockSize, 0) {}
    
    // Default constructor
    CacheEntry() : CacheEntry(0) {}

    // Reset the cache entry
    void reset() {
        tag = 0;
        valid = false;
        dirty = false;
        lastUsed = 0;
        accessCount = 0;
        insertionTime = std::chrono::system_clock::now();
        std::fill(data.begin(), data.end(), 0);
    }
};