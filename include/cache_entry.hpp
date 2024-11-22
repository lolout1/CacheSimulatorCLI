#pragma once
#include <vector>
#include <chrono>
#include <cstdint>
#include <memory>

class CacheEntry {
public:
    uint64_t tag;
    bool valid;
    bool dirty;
    uint64_t lastUsed;
    uint64_t accessCount;
    std::chrono::system_clock::time_point insertionTime;
    std::vector<uint8_t> data;

    CacheEntry(int blockSize) 
        : tag(0), valid(false), dirty(false), lastUsed(0), accessCount(0),
          insertionTime(std::chrono::system_clock::now()), data(blockSize, 0) {}
};

struct PLRUNode {
    bool bit;
    std::shared_ptr<PLRUNode> left;
    std::shared_ptr<PLRUNode> right;
    
    PLRUNode() : bit(false) {}
    
    // Implement deep copy constructor
    PLRUNode(const PLRUNode& other) : bit(other.bit) {
        if (other.left) {
            left = std::make_shared<PLRUNode>(*other.left);
        }
        if (other.right) {
            right = std::make_shared<PLRUNode>(*other.right);
        }
    }
    
    // Copy assignment operator
    PLRUNode& operator=(const PLRUNode& other) {
        if (this != &other) {
            bit = other.bit;
            if (other.left) {
                left = std::make_shared<PLRUNode>(*other.left);
            } else {
                left.reset();
            }
            if (other.right) {
                right = std::make_shared<PLRUNode>(*other.right);
            } else {
                right.reset();
            }
        }
        return *this;
    }
};