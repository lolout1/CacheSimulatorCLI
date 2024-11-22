#include <gnuplot-iostream.h> 
#include <CLI11.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>
#include <deque>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <bitset>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <map>
#include <random>
#include <CLI11.hpp> 
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
class AddressParser {
private:
    const int N;
    const int offsetBits;
    const int indexBits;
    const int tagBits;
    std::unordered_map<uint64_t, bool> seenAddresses; 
public:
    struct ParsedAddress {
        uint64_t tag;
        uint64_t index;
        uint64_t offset;
        uint64_t fullAddress;
        bool isColdAccess;
        std::string toBinaryString() const {
            std::stringstream ss;
            ss << "Tag: " << std::bitset<32>(tag) 
               << " Index: " << std::bitset<16>(index)
               << " Offset: " << std::bitset<8>(offset);
            return ss.str();
        }
    };
    AddressParser(int N, int blockSize, int numSets) 
        : N(N),
          offsetBits(static_cast<int>(std::log2(blockSize))),
          indexBits(static_cast<int>(std::log2(numSets))),
          tagBits(N - offsetBits - indexBits) {
        validateConfiguration();
    }
    ParsedAddress parseAddress(const std::string& addrStr) {
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
private:
    void validateConfiguration() {
        if (tagBits < 0) {
            throw std::invalid_argument("Invalid cache configuration: address bits insufficient");
        }
    }
};
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
    std::unique_ptr<PLRUNode> left;
    std::unique_ptr<PLRUNode> right;
    PLRUNode() : bit(false) {}
};
class CacheSet {
private:
    std::vector<CacheEntry> entries;
    std::unique_ptr<PLRUNode> plruRoot;
    std::vector<uint64_t> fifoQueue;
    std::map<uint64_t, uint64_t> frequencyCount;
    std::random_device rd;
    std::mt19937 gen;
    const ReplacementPolicy policy;
    uint64_t accessCount;
public:
    CacheSet(int ways, int blockSize, ReplacementPolicy p) 
        : entries(ways, CacheEntry(blockSize)), 
          policy(p), 
          accessCount(0),
          gen(rd()) {
        if (p == ReplacementPolicy::PLRU) {
            initializePLRUTree(ways);
        }
        fifoQueue.reserve(ways);
    }
    bool lookup(uint64_t tag, size_t& wayIndex) {
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].valid && entries[i].tag == tag) {
                wayIndex = i;
                updateAccess(i);
                return true;
            }
        }
        return false;
    }
    void updateAccess(size_t wayIndex) {
        entries[wayIndex].lastUsed = ++accessCount;
        entries[wayIndex].accessCount++;
        if (policy == ReplacementPolicy::PLRU) {
            updatePLRUBits(wayIndex);
        }
        frequencyCount[entries[wayIndex].tag]++;
    }
    size_t findVictim(uint64_t newTag) {
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
private:
    size_t findLRUVictim() {
        return std::min_element(entries.begin(), entries.end(),
            [](const CacheEntry& a, const CacheEntry& b) {
                return a.lastUsed < b.lastUsed;
            }) - entries.begin();
    }
    size_t findMRUVictim() {
        return std::max_element(entries.begin(), entries.end(),
            [](const CacheEntry& a, const CacheEntry& b) {
                return a.lastUsed < b.lastUsed;
            }) - entries.begin();
    }
    size_t findRandomVictim() {
        std::uniform_int_distribution<> dis(0, entries.size() - 1);
        return dis(gen);
    }
    size_t findFIFOVictim() {
        size_t victim = fifoQueue.front();
        fifoQueue.erase(fifoQueue.begin());
        fifoQueue.push_back(victim);
        return victim;
    }
    void initializePLRUTree(int ways) {
        plruRoot = std::make_unique<PLRUNode>();
        int depth = static_cast<int>(std::ceil(std::log2(ways)));
        initializePLRUNode(plruRoot.get(), depth);
    }
    void initializePLRUNode(PLRUNode* node, int depth) {
        if (depth > 0) {
            node->left = std::make_unique<PLRUNode>();
            node->right = std::make_unique<PLRUNode>();
            initializePLRUNode(node->left.get(), depth - 1);
            initializePLRUNode(node->right.get(), depth - 1);
        }
    }
    size_t findPLRUVictim() {
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
    void updatePLRUBits(size_t accessedWay) {
        PLRUNode* node = plruRoot.get();
        int depth = static_cast<int>(std::ceil(std::log2(entries.size())));
        for (int i = depth - 1; i >= 0; i--) {
            bool bit = (accessedWay >> i) & 1;
            node->bit = !bit;
            node = bit ? node->right.get() : node->left.get();
        }
    }
    size_t findLFUVictim() {
        return std::min_element(entries.begin(), entries.end(),
            [this](const CacheEntry& a, const CacheEntry& b) {
                return frequencyCount[a.tag] < frequencyCount[b.tag];
            }) - entries.begin();
    }
    size_t findARCVictim() {
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
};
class Cache {
private:
    const int N;
    const int blockSize;
    const int numSets;
    const int ways;
    std::vector<CacheSet> sets;
    AddressParser parser;
    ReplacementPolicy policy;
    CacheStats stats;
public:
    Cache(int N, int B, int I, int ways = 1, ReplacementPolicy policy = ReplacementPolicy::LRU)
        : N(N),
          blockSize(static_cast<int>(std::pow(2, std::ceil(std::log2(B))))),
          numSets(static_cast<int>(std::pow(2, std::ceil(std::log2(I))))),
          ways(ways),
          sets(numSets, CacheSet(ways, blockSize, policy)),
          parser(N, blockSize, numSets),
          policy(policy) {}
    struct AccessResult {
        std::string indexHex;
        std::string tagHex;
        char hitMiss;
        std::string binaryRepresentation;
        uint64_t accessTime;
        bool isColdMiss;
        bool isConflictMiss;
    };
    AccessResult access(const std::string& addrStr) {
        auto start = std::chrono::high_resolution_clock::now();
        auto parsed = parser.parseAddress(addrStr);
        size_t wayIndex;
        bool hit = sets[parsed.index].lookup(parsed.tag, wayIndex);
        bool isColdMiss = !hit && parsed.isColdAccess;
        bool isConflictMiss = !hit && !parsed.isColdAccess;
        if (!hit) {
            wayIndex = sets[parsed.index].findVictim(parsed.tag);
            auto& entry = sets[parsed.index].entries[wayIndex];
            entry.tag = parsed.tag;
            entry.valid = true;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        stats.recordAccess(hit, isColdMiss, isConflictMiss);
        stats.totalAccessTime += duration;
        stats.addressFrequency[parsed.fullAddress]++;
        return {
            formatHex(parsed.index),
            formatHex(parsed.tag),
            hit ? 'H' : 'M',
            parsed.toBinaryString(),
            duration.count(),
            isColdMiss,
            isConflictMiss
        };
    }
    const CacheStats& getStats() const { return stats; }
private:
    std::string formatHex(uint64_t value) const {
        std::stringstream ss;
        ss << "x" << std::hex << std::uppercase << value;
        return ss.str();
    }
};
class CacheSimulator {
public:
    static void runSimulation(const CLIParams& params) {
        Cache cache(params.N, params.B, params.I, params.ways, params.policy);
        auto addresses = readAddresses(params.filename);
        std::vector<Cache::AccessResult> results;
        for (const auto& addr : addresses) {
            results.push_back(cache.access(addr));
        }
        printResults(results, cache.getStats());
        if (params.generateVisualization) {
            generateVisualization(results, cache.getStats(), params.visualizationFile);
        }
struct CLIParams {
    int N = 16;            
    int B = 4;             
    int I = 4;             
    int ways = 1;          
    std::string filename = "addresses.txt";
    ReplacementPolicy policy = ReplacementPolicy::LRU;
    bool generateVisualization = false;
    std::string visualizationFile = "cache_visualization.png";
    bool verbose = false;
};
void generateVisualization(const std::vector<Cache::AccessResult>& results, 
                         const CacheStats& stats,
                         const std::string& outputFile) {
    try {
        Gnuplot gp;
        std::vector<std::pair<double, double>> hitRateData;
        for (size_t i = 0; i < stats.hitRateHistory.size(); ++i) {
            hitRateData.emplace_back(i, stats.hitRateHistory[i]);
        }
        gp << "set terminal pngcairo enhanced size 1200,800\n";
        gp << "set output '" << outputFile << "'\n";
        gp << "set multiplot layout 2,2\n";
        gp << "set title 'Cache Hit Rate Over Time'\n";
        gp << "set xlabel 'Access Number'\n";
        gp << "set ylabel 'Hit Rate (%)'\n";
        gp << "plot '-' with lines title 'Hit Rate'\n";
        gp.send(hitRateData);
        std::vector<std::pair<std::string, double>> missData = {
            {"Cold", static_cast<double>(stats.coldMisses)},
            {"Conflict", static_cast<double>(stats.conflictMisses)},
            {"Capacity", static_cast<double>(stats.capacityMisses)}
        };
        gp << "set style data histogram\n";
        gp << "set style fill solid\n";
        gp << "set title 'Miss Type Distribution'\n";
        gp << "plot '-' using 2:xtic(1) title 'Miss Types'\n";
        gp.send(missData);
    } catch (const std::exception& e) {
        std::cerr << "Visualization error: " << e.what() << std::endl;
    }
}
void printResults(const std::vector<Cache::AccessResult>& results, const CacheStats& stats) {
    std::cout << std::setw(10) << "Index" 
              << std::setw(10) << "Tag"
              << std::setw(8) << "Result"
              << std::setw(15) << "Access Time"
              << std::setw(10) << "Type" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    for (const auto& result : results) {
        std::cout << std::setw(10) << result.indexHex
                  << std::setw(10) << result.tagHex
                  << std::setw(8) << result.hitMiss
                  << std::setw(15) << result.accessTime << "µs"
                  << std::setw(10) 
                  << (result.isColdMiss ? "Cold" : 
                      (result.isConflictMiss ? "Conflict" : "Hit"))
                  << std::endl;
    }
    std::cout << "\nCache Statistics:" << std::endl;
    std::cout << "-----------------" << std::endl;
    std::cout << "Total Accesses: " << stats.totalAccesses << std::endl;
    std::cout << "Hits: " << stats.hits << std::endl;
    std::cout << "Misses: " << stats.misses << std::endl;
    std::cout << "Hit Rate: " << std::fixed << std::setprecision(2) 
              << stats.getHitRate() << "%" << std::endl;
    std::cout << "Cold Misses: " << stats.coldMisses << std::endl;
    std::cout << "Conflict Misses: " << stats.conflictMisses << std::endl;
    std::cout << "Capacity Misses: " << stats.capacityMisses << std::endl;
    std::cout << "Average Access Time: " 
              << (stats.totalAccessTime.count() / static_cast<double>(stats.totalAccesses))
              << "µs" << std::endl;
}
std::vector<std::string> readAddresses(const std::string& filename) {
    std::vector<std::string> addresses;
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            if (line.substr(0, 1) != "x") {
                throw std::runtime_error("Invalid address format: " + line);
            }
            addresses.push_back(line);
        }
    }
    return addresses;
}
int main(int argc, char** argv) {
    try {
        CLI::App app{"Cache Simulator with Multiple Replacement Policies"};
        CLIParams params;
        app.add_option("-N", params.N, "Address space size in 2^N bytes");
        app.add_option("-B", params.B, "Block size in bytes");
        app.add_option("-I", params.I, "Number of blocks");
        app.add_option("-w,--ways", params.ways, "Number of ways (associativity)");
        app.add_option("-f,--file", params.filename, "Input file with addresses");
        app.add_flag("-v,--verbose", params.verbose, "Enable verbose output");
        app.add_flag("--viz", params.generateVisualization, "Generate visualization");
        app.add_option("--viz-file", params.visualizationFile, "Visualization output file");
        std::map<std::string, ReplacementPolicy> policyMap = {
            {"lru", ReplacementPolicy::LRU},
            {"mru", ReplacementPolicy::MRU},
            {"optimal", ReplacementPolicy::OPTIMAL},
            {"random", ReplacementPolicy::RANDOM},
            {"fifo", ReplacementPolicy::FIFO},
            {"plru", ReplacementPolicy::PLRU},
            {"lfu", ReplacementPolicy::LFU},
            {"arc", ReplacementPolicy::ARC}
        };
        std::string policyStr = "lru";
        app.add_option("-p,--policy", policyStr, "Replacement policy")
           ->transform(CLI::CheckedTransformer(policyMap, CLI::ignore_case));
        CLI11_PARSE(app, argc, argv);
        params.policy = policyMap[policyStr];
        CacheSimulator::runSimulation(params);
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
