#include "address_parser.hpp"
#include <cmath>
#include <stdexcept>

AddressParser::AddressParser(int N, int blockSize, int numSets) 
    : N(N),
      offsetBits(static_cast<int>(std::log2(blockSize))),
      indexBits(static_cast<int>(std::log2(numSets))),
      tagBits(N - offsetBits - indexBits) {
    validateConfiguration();
}

AddressParser::ParsedAddress AddressParser::parseAddress(const std::string& addrStr) {
    uint64_t addr = std::stoull(addrStr.substr(addrStr.find('x') + 1), nullptr, 16);
    
    ParsedAddress result;
    result.offset = addr & ((1ULL << offsetBits) - 1);
    result.index = (addr >> offsetBits) & ((1ULL << indexBits) - 1);
    result.tag = addr >> (offsetBits + indexBits);
    result.fullAddress = addr;
    result.isColdAccess = !seenAddresses[addr];
    
    seenAddresses[addr] = true;
    return result;
}

void AddressParser::validateConfiguration() {
    if (tagBits < 0) {
        throw std::invalid_argument("Invalid cache configuration: address bits insufficient");
    }
}

// Define the toBinaryString method here
std::string AddressParser::ParsedAddress::toBinaryString() const {
    std::stringstream ss;
    ss << "Tag: " << std::bitset<32>(tag) 
       << " Index: " << std::bitset<16>(index)
       << " Offset: " << std::bitset<8>(offset);
    return ss.str();
}