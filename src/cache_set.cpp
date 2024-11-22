#include "cache_set.hpp"
#include <algorithm>
#include <stdexcept>
#include <cmath>

CacheSet::CacheSet(int ways, int blockSize, ReplacementPolicy p) 
    : entries(ways, CacheEntry(blockSize)), 
      policy(p), 
      accessCount(0),
      gen(rd()) {
    if (p == ReplacementPolicy::PLRU) {
        plruRoot = std::make_shared<PLRUNode>();
        int depth = static_cast<int>(std::ceil(std::log2(ways)));
        initializePLRUTree(ways);
    }
    fifoQueue.reserve(ways);
}

CacheSet::CacheSet(const CacheSet& other) 
    : entries(other.entries),
      policy(other.policy),
      accessCount(other.accessCount),
      fifoQueue(other.fifoQueue),
      frequencyCount(other.frequencyCount),
      gen(rd()) {
    if (other.plruRoot) {
        plruRoot = std::make_shared<PLRUNode>(*other.plruRoot);
    }
}

CacheSet::CacheSet(CacheSet&& other) noexcept 
    : entries(std::move(other.entries)),
      plruRoot(std::move(other.plruRoot)),
      policy(other.policy),
      accessCount(other.accessCount),
      fifoQueue(std::move(other.fifoQueue)),
      frequencyCount(std::move(other.frequencyCount)),
      gen(std::move(other.gen)) {}

CacheSet& CacheSet::operator=(const CacheSet& other) {
    if (this != &other) {
        entries = other.entries;
        policy = other.policy;
        accessCount = other.accessCount;
        fifoQueue = other.fifoQueue;
        frequencyCount = other.frequencyCount;
        if (other.plruRoot) {
            plruRoot = std::make_shared<PLRUNode>(*other.plruRoot);
        } else {
            plruRoot.reset();
        }
    }
    return *this;
}

CacheSet& CacheSet::operator=(CacheSet&& other) noexcept {
    if (this != &other) {
        entries = std::move(other.entries);
        plruRoot = std::move(other.plruRoot);
        policy = other.policy;
        accessCount = other.accessCount;
        fifoQueue = std::move(other.fifoQueue);
        frequencyCount = std::move(other.frequencyCount);
        gen = std::move(other.gen);
    }
    return *this;
}

bool CacheSet::lookup(uint64_t tag, size_t& wayIndex) {
    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].valid && entries[i].tag == tag) {
            wayIndex = i;
            updateAccess(i);
            return true;
        }
    }
    return false;
}

void CacheSet::updateAccess(size_t wayIndex) {
    entries[wayIndex].lastUsed = ++accessCount;
    entries[wayIndex].accessCount++;
    
    if (policy == ReplacementPolicy::PLRU) {
        updatePLRUBits(wayIndex);
    }
    
    frequencyCount[entries[wayIndex].tag]++;
}

size_t CacheSet::findVictim(uint64_t newTag) {
    // First check for empty slots
    for (size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].valid) {
            if (policy == ReplacementPolicy::FIFO) {
                fifoQueue.push_back(i);
            }
            return i;
        }
    }

    switch (policy) {
        case ReplacementPolicy::LRU:
            return findLRUVictim();
        case ReplacementPolicy::MRU:
            return findMRUVictim();
        case ReplacementPolicy::RANDOM:
            return findRandomVictim();
        case ReplacementPolicy::FIFO:
            return findFIFOVictim();
        case ReplacementPolicy::PLRU:
            return findPLRUVictim();
        case ReplacementPolicy::LFU:
            return findLFUVictim();
        case ReplacementPolicy::ARC:
            return findARCVictim();
        default:
            throw std::runtime_error("Unknown replacement policy");
    }
}

size_t CacheSet::findLRUVictim() {
    return std::min_element(entries.begin(), entries.end(),
        [](const CacheEntry& a, const CacheEntry& b) {
            return a.lastUsed < b.lastUsed;
        }) - entries.begin();
}

size_t CacheSet::findMRUVictim() {
    return std::max_element(entries.begin(), entries.end(),
        [](const CacheEntry& a, const CacheEntry& b) {
            return a.lastUsed < b.lastUsed;
        }) - entries.begin();
}

size_t CacheSet::findRandomVictim() {
    std::uniform_int_distribution<> dis(0, entries.size() - 1);
    return dis(gen);
}

size_t CacheSet::findFIFOVictim() {
    size_t victim = fifoQueue.front();
    fifoQueue.erase(fifoQueue.begin());
    fifoQueue.push_back(victim);
    return victim;
}

void CacheSet::initializePLRUTree(int ways) {
    int depth = static_cast<int>(std::ceil(std::log2(ways)));
    initializePLRUNode(plruRoot.get(), depth);
}

void CacheSet::initializePLRUNode(PLRUNode* node, int depth) {
    if (depth > 0) {
        node->left = std::make_shared<PLRUNode>();
        node->right = std::make_shared<PLRUNode>();
        initializePLRUNode(node->left.get(), depth - 1);
        initializePLRUNode(node->right.get(), depth - 1);
    }
}

size_t CacheSet::findPLRUVictim() {
    PLRUNode* node = plruRoot.get();
    size_t index = 0;
    int depth = static_cast<int>(std::ceil(std::log2(entries.size())));
    
    for (int i = 0; i < depth; i++) {
        if (node->bit) {
            node = node->right.get();
            index = (index << 1) | 1;
        } else {
            node = node->left.get();
            index = index << 1;
        }
    }
    
    return index % entries.size();
}

void CacheSet::updatePLRUBits(size_t accessedWay) {
    PLRUNode* node = plruRoot.get();
    int depth = static_cast<int>(std::ceil(std::log2(entries.size())));
    
    for (int i = depth - 1; i >= 0; i--) {
        bool bit = (accessedWay >> i) & 1;
        node->bit = !bit;
        node = bit ? node->right.get() : node->left.get();
    }
}

size_t CacheSet::findLFUVictim() {
    return std::min_element(entries.begin(), entries.end(),
        [this](const CacheEntry& a, const CacheEntry& b) {
            return frequencyCount[a.tag] < frequencyCount[b.tag];
        }) - entries.begin();
}

size_t CacheSet::findARCVictim() {
    auto now = std::chrono::system_clock::now();
    return std::min_element(entries.begin(), entries.end(),
        [&now](const CacheEntry& a, const CacheEntry& b) {
            double aScore = a.accessCount * 0.7 + 
                std::chrono::duration_cast<std::chrono::seconds>
                (now - a.insertionTime).count() * 0.3;
            double bScore = b.accessCount * 0.7 + 
                std::chrono::duration_cast<std::chrono::seconds>
                (now - b.insertionTime).count() * 0.3;
            return aScore < bScore;
        }) - entries.begin();
}