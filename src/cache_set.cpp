#include "cache_set.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <random>
#include <chrono>

// Constructor implementation
CacheSet::CacheSet(int ways, int blockSize, ReplacementPolicy p)
    : entries(ways, CacheEntry(blockSize)),
      policy(p),
      dis(0, ways - 1) {
    state.offsetBits = static_cast<int>(std::log2(blockSize));
    state.indexBits = static_cast<int>(std::log2(ways));
}

// Copy constructor
CacheSet::CacheSet(const CacheSet& other)
    : entries(other.entries),
      policy(other.policy),
      accessCount(other.accessCount),
      state(other.state),
      gen(std::random_device{}()),
      dis(other.dis),
      fifoQueue(other.fifoQueue) {
}

// Move constructor
CacheSet::CacheSet(CacheSet&& other) noexcept
    : entries(std::move(other.entries)),
      policy(other.policy),
      accessCount(other.accessCount),
      state(std::move(other.state)),
      gen(std::random_device{}()),
      dis(other.dis),
      fifoQueue(std::move(other.fifoQueue)) {
}

// Copy assignment operator
CacheSet& CacheSet::operator=(const CacheSet& other) {
    if (this != &other) {
        entries = other.entries;
        policy = other.policy;
        accessCount = other.accessCount;
        state = other.state;
        fifoQueue = other.fifoQueue;
        
        // Reset RNG
        gen.seed(std::random_device{}());
        dis = other.dis;
    }
    return *this;
}

// Move assignment operator
CacheSet& CacheSet::operator=(CacheSet&& other) noexcept {
    if (this != &other) {
        entries = std::move(other.entries);
        policy = other.policy;
        accessCount = other.accessCount;
        state = std::move(other.state);
        fifoQueue = std::move(other.fifoQueue);
        
        // Reset RNG
        gen.seed(std::random_device{}());
        dis = other.dis;
    }
    return *this;
}

// Lookup method with wayIndex
bool CacheSet::lookup(uint64_t tag, size_t& wayIndex) {
    bool hit = false;
    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].valid && entries[i].tag == tag) {
            wayIndex = i;
            hit = true;
            break;
        }
    }

    if (hit) {
        entries[wayIndex].lastUsed = ++accessCount;
        entries[wayIndex].accessCount++;
    }

    // Update current position for OPTIMAL policy
    if (policy == ReplacementPolicy::OPTIMAL) {
        state.currentPosition++;
    }

    return hit;
}

// Insert method
void CacheSet::insert(uint64_t tag) {
    size_t victimWay = findVictim(tag);
    
    // Replace the victim entry
    entries[victimWay].tag = tag;
    entries[victimWay].valid = true;
    entries[victimWay].lastUsed = ++accessCount;
    entries[victimWay].insertionTime = std::chrono::system_clock::now();
    entries[victimWay].accessCount = 1;
}

// Find victim way based on replacement policy
size_t CacheSet::findVictim(uint64_t newTag) {
    // First check for invalid entries
    for (size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].valid) {
            if (policy == ReplacementPolicy::FIFO) {
                fifoQueue.push_back(i);
            }
            return i;
        }
    }

    // Apply the appropriate replacement policy
    switch (policy) {
        case ReplacementPolicy::LRU:
            return findLRUVictim();
        case ReplacementPolicy::MRU:
            return findMRUVictim();
        case ReplacementPolicy::FIFO:
            return findFIFOVictim();
        case ReplacementPolicy::OPTIMAL:
            return findOptimalVictim(newTag);
        case ReplacementPolicy::RANDOM:
            return findRandomVictim();
        default:
            throw std::runtime_error("Unknown replacement policy");
    }
}

// LRU victim selection
size_t CacheSet::findLRUVictim() const {
    size_t oldestWay = 0;
    uint64_t oldestAccess = std::numeric_limits<uint64_t>::max();
    
    for (size_t way = 0; way < entries.size(); ++way) {
        if (entries[way].lastUsed < oldestAccess) {
            oldestWay = way;
            oldestAccess = entries[way].lastUsed;
        }
    }
    
    return oldestWay;
}

// MRU victim selection
size_t CacheSet::findMRUVictim() const {
    size_t mostRecentWay = 0;
    uint64_t mostRecentAccess = 0;
    
    for (size_t way = 0; way < entries.size(); ++way) {
        if (entries[way].lastUsed > mostRecentAccess) {
            mostRecentWay = way;
            mostRecentAccess = entries[way].lastUsed;
        }
    }
    
    return mostRecentWay;
}

// FIFO victim selection
size_t CacheSet::findFIFOVictim() const {
    size_t oldestWay = 0;
    auto oldestTime = std::chrono::system_clock::now();
    
    for (size_t way = 0; way < entries.size(); ++way) {
        if (entries[way].insertionTime < oldestTime) {
            oldestWay = way;
            oldestTime = entries[way].insertionTime;
        }
    }
    
    return oldestWay;
}

// Random victim selection
size_t CacheSet::findRandomVictim() const {
    return dis(gen);
}

// Optimal victim selection
size_t CacheSet::findOptimalVictim(uint64_t newTag) {
    // First check for invalid entries
    for (size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].valid) {
            return i;
        }
    }
    
    size_t victimWay = 0;
    size_t furthestNextUse = 0;
    bool foundNoFutureUse = false;

    // Find the entry that will be used furthest in the future
    for (size_t i = 0; i < entries.size(); i++) {
        uint64_t tag = entries[i].tag;
        size_t nextUse = getNextAccess(tag);
        
        // If this entry won't be used again, it's a perfect candidate
        if (nextUse == std::numeric_limits<size_t>::max()) {
            if (!foundNoFutureUse) {
                victimWay = i;
                foundNoFutureUse = true;
            }
            continue;
        }
        
        // If we haven't found an entry with no future use,
        // keep track of the entry used furthest in the future
        if (!foundNoFutureUse && nextUse > furthestNextUse) {
            furthestNextUse = nextUse;
            victimWay = i;
        }
    }
    
    return victimWay;
}

// Get next access for a tag
size_t CacheSet::getNextAccess(uint64_t tag) const {
    auto it = state.futureMap.find(tag);
    if (it == state.futureMap.end()) {
        return std::numeric_limits<size_t>::max();
    }
    return it->second.getNextAccess(state.currentPosition);
}

// Update optimal state (placeholder)
void CacheSet::updateOptimalState() {
    // This method is now handled in the lookup method
}

// Preprocess trace for optimal policy
void CacheSet::preprocessTrace(const std::vector<uint64_t>& trace) {
    if (trace.empty()) {
        std::cerr << "Warning: Empty trace provided for OPTIMAL policy" << std::endl;
        return;
    }

    state.fullTrace = trace;
    state.currentPosition = 0;
    state.futureMap.clear();
    
    // Build future access map with more robust tag extraction
    for (size_t i = 0; i < trace.size(); i++) {
        // Ensure we handle different address representations
        uint64_t tag = trace[i] >> (state.offsetBits + state.indexBits);
        state.futureMap[tag].positions.push_back(i);
    }
    
    // Initialize next access tracking with error checking
    for (auto& [tag, access] : state.futureMap) {
        // Sort positions to ensure correct next access tracking
        std::sort(access.positions.begin(), access.positions.end());
    }
}

// Set optimal trace
void CacheSet::setOptimalTrace(const std::vector<uint64_t>& trace) {
    preprocessTrace(trace);
}