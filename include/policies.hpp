#pragma once

#include <string>
#include <memory>

enum class ReplacementPolicy {
    LRU,        // Least Recently Used
    MRU,        // Most Recently Used
    OPTIMAL,    // Optimal (Look-ahead) replacement
    FIFO,       // First In First Out
    RANDOM      // Random replacement
};

// Helper function to convert policy to string
inline std::string getPolicyName(ReplacementPolicy policy) {
    switch (policy) {
        case ReplacementPolicy::LRU:     return "LRU";
        case ReplacementPolicy::MRU:     return "MRU";
        case ReplacementPolicy::OPTIMAL: return "OPTIMAL";
        case ReplacementPolicy::FIFO:    return "FIFO";
        case ReplacementPolicy::RANDOM:  return "RANDOM";
        default:                         return "UNKNOWN";
    }
}

// Centralized PLRUNode definition
struct PLRUNode {
    bool bit{false};
    std::shared_ptr<PLRUNode> left{nullptr};
    std::shared_ptr<PLRUNode> right{nullptr};
    
    // Default constructor
    PLRUNode() = default;
    
    // Copy constructor
    PLRUNode(const PLRUNode& other) 
        : bit(other.bit)
        , left(other.left ? std::make_shared<PLRUNode>(*other.left) : nullptr)
        , right(other.right ? std::make_shared<PLRUNode>(*other.right) : nullptr) {}
    
    // Copy assignment operator
    PLRUNode& operator=(const PLRUNode& other) {
        if (this != &other) {
            bit = other.bit;
            left = other.left ? std::make_shared<PLRUNode>(*other.left) : nullptr;
            right = other.right ? std::make_shared<PLRUNode>(*other.right) : nullptr;
        }
        return *this;
    }
};
