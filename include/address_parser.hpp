#pragma once
#include <string>
#include <unordered_map>
#include <bitset>
#include <sstream>
#include <cstdint>

class AddressParser {
public:
    struct ParsedAddress {
        uint64_t tag;
        uint64_t index;
        uint64_t offset;
        uint64_t fullAddress;
        bool isColdAccess;
        
        // Declare the function but don't define it here
        std::string toBinaryString() const;
    };

    AddressParser(int N, int blockSize, int numSets);
    ParsedAddress parseAddress(const std::string& addrStr);

private:
    const int N;
    const int offsetBits;
    const int indexBits;
    const int tagBits;
    std::unordered_map<uint64_t, bool> seenAddresses;
    
    void validateConfiguration();
};