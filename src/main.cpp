#include <iostream>
#include <fstream>
#include <unordered_set>
#include <map>
#include <CLI11.hpp>
#include "cache.hpp"
#include "visualization.hpp"

struct CLIParams {
    int N = 16;            // Address space size in 2^N bytes (default: 16)
    int B = 4;             // Block size in bytes (default: 4)
    int I = 4;             // Number of blocks (default: 4)
    int ways = 1;          // Number of ways/associativity (default: 1)
    std::string filename = "addresses.txt"; // Input file with addresses
    ReplacementPolicy policy = ReplacementPolicy::LRU; // Replacement policy
    bool generateVisualization = false; // Generate visualization
    std::string visualizationFile = "cache_visualization.png"; // Visualization output file
    bool verbose = false; // Enable verbose output
};

std::vector<std::string> readAddresses(const std::string& filename) {
    std::vector<std::string> addresses;
    std::unordered_set<std::string> uniqueAddresses;  // Track unique addresses
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
            // Only add address if we haven't seen it before in this file
            if (uniqueAddresses.insert(line).second) {
                addresses.push_back(line);
            }
        }
    }
    return addresses;
}

int main(int argc, char** argv) {
    CLI::App app{"Cache Simulator"};
    CLIParams params;
    try {
        app.add_option("-N", params.N, "Address space size in 2^N bytes (default: 16)")
            ->check(CLI::Range(1, 64));
        app.add_option("-B", params.B, "Block size in bytes (default: 4)")
            ->check(CLI::Range(1, 1024));
        app.add_option("-I", params.I, "Number of blocks (default: 4)")
            ->check(CLI::Range(1, 1024));
        app.add_option("-w,--ways", params.ways, "Number of ways/associativity (default: 1)")
            ->check(CLI::Range(1, 32));
        app.add_option("-f,--file", params.filename, "Input file with addresses")
            ->required();
        app.add_flag("-v,--verbose", params.verbose, "Enable verbose output");
        app.add_flag("--viz", params.generateVisualization, "Generate visualization");
        app.add_option("--viz-file", params.visualizationFile, 
            "Visualization output file (default: cache_visualization.png)");
        std::map<std::string, ReplacementPolicy> policyMap{
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
        app.add_option("-p,--policy", policyStr, 
            "Replacement policy (lru/mru/optimal/random/fifo/plru/lfu/arc)")
            ->transform(CLI::CheckedTransformer(policyMap, CLI::ignore_case));
        CLI11_PARSE(app, argc, argv);
        params.policy = policyMap[policyStr];

        // Create cache with given configuration
        Cache cache(params.N, params.B, params.I, params.ways, params.policy);
        
        // Print cache configuration if verbose
        if (params.verbose) {
            std::cout << "\nCache Configuration:" << std::endl;
            std::cout << "-------------------" << std::endl;
            std::cout << "Address space: 2^" << params.N << " bytes" << std::endl;
            std::cout << "Block size: " << params.B << " bytes" << std::endl;
            std::cout << "Number of blocks: " << params.I << std::endl;
            std::cout << "Associativity: " << params.ways << "-way" << std::endl;
            std::cout << "Replacement Policy: " << policyStr << std::endl;
            std::cout << "\n";
        }

        // Process the initial input file
        try {
            auto addresses = readAddresses(params.filename);
            std::vector<Cache::AccessResult> results;
            
            for (const auto& addr : addresses) {
                results.push_back(cache.access(addr));
            }

            CacheVisualizer::printResults(results, cache.getStats());

            if (params.generateVisualization) {
                CacheVisualizer::generateVisualization(
                    results, 
                    cache.getStats(), 
                    params.visualizationFile
                );
                std::cout << "\nVisualization saved to: " << params.visualizationFile << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error processing file: " << e.what() << std::endl;
            return 1;
        }

        // Ask if user wants to process another file
        std::cout << "\nWould you like to process another file? (y/n): ";
        std::string response;
        std::getline(std::cin, response);
        
        while (response == "y" || response == "Y") {
            std::cout << "Enter filename: ";
            std::string filename;
            std::getline(std::cin, filename);
            
            try {
                auto addresses = readAddresses(filename);
                std::vector<Cache::AccessResult> results;
                
                std::cout << "\nProcessing file: " << filename << std::endl;
                std::cout << "----------------------------------------\n";
                
                for (const auto& addr : addresses) {
                    results.push_back(cache.access(addr));
                }

                CacheVisualizer::printResults(results, cache.getStats());

                if (params.generateVisualization) {
                    std::string vizFile = params.visualizationFile;
                    size_t dot_pos = vizFile.find_last_of('.');
                    if (dot_pos != std::string::npos) {
                        vizFile.insert(dot_pos, "_" + filename.substr(0, filename.find_last_of('.')));
                    }
                    
                    CacheVisualizer::generateVisualization(
                        results, 
                        cache.getStats(), 
                        vizFile
                    );
                    std::cout << "\nVisualization saved to: " << vizFile << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing file: " << e.what() << std::endl;
            }

            std::cout << "\nWould you like to process another file? (y/n): ";
            std::getline(std::cin, response);
        }

        return 0;
    }
    catch (const CLI::ParseError &e) {
        return app.exit(e);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
