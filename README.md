# Cache Simulator - CS3339 Extra Credit 3

## Project Structure
```
sww35_EC3/ 
├── src/              # Source code
├── include/          # Header files
├── test/             # Test cases
├── examples/         # Example input files
├── CMakeLists.txt   # Build configuration
└── README.md        # This file
```

## Compilation Instructions
```bash
mkdir build
cd build
cmake ..
make
```

## Basic Usage
```bash
./cache_simulator -N 32 -B 4 -I 4 -f ../examples/addresses.txt
```

## Required Parameters
- N: Address space size (2^N bytes)
- B: Block size (rounded to power of 2)
- I: Number of blocks (rounded to power of 2)
- filename.txt: Input file with addresses

## Optional Parameters
- w, --ways: Association level (default: 1)
- p, --policy: Replacement policy (lru/mru/arc)

## Replacement Policies
1. LRU (Least Recently Used)
   - Removes the least recently accessed block
   - Optimal for temporal locality

2. MRU (Most Recently Used)
   - Removes the most recently accessed block
   - Better for scanning workloads

3. ARC (Adaptive Replacement Cache)
   - Advanced policy that adapts to access patterns
   - Combines recency and frequency
   - Self-tuning for optimal performance

## Example Input File Format
```
x0000
x1000
x2000
xFFFF
```

## Example Output
```
Index   Tag     Hit/Miss
x0      x1      M
x1      x2      H
x2      x1      M
```

## Test Cases
See test/test_cases.md for detailed test scenarios.
