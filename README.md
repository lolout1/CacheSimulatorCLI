# Cache Simulator CLI - Abheek Pradhan CS 3339 Fall 2024 

A flexible and feature-rich cache simulator designed for computer architecture analysis and education. This simulator supports various cache configurations, replacement policies, and provides detailed analysis of cache behavior.

## Features

- Configurable cache parameters (size, block size, associativity)
- Multiple replacement policies (LRU, MRU, Optimal)
- Detailed access analysis and statistics
- Support for trace file input
- Dynamic visualization of cache behavior

## Installation

### Prerequisites
- C++ compiler with C++11 support
- CMake (version 3.10 or higher)
- Make

### Building from Source
```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Basic Command Format
```bash
./cache_simulator -N <address_bits> -B <block_size> -I <index_bits> -w <ways> -p <policy> -f <trace_file>
```

### Parameters

- `-N`: Address space size in bits (e.g., 32 for MIPS)
- `-B`: Block size in bytes (will be rounded up to power of 2)
- `-I`: Number of index bits (determines number of sets)
- `-w`: Number of ways (associativity level)
- `-p`: Replacement policy (lru/mru/opt)
- `-f`: Input trace file path

### Example Commands
```bash
# 4-way set associative cache with LRU replacement
./cache_simulator -N 32 -B 4 -I 8 -w 4 -p lru -f ../examples/trace1.txt

# Direct-mapped cache
./cache_simulator -N 32 -B 4 -I 8 -w 1 -p lru -f ../examples/trace1.txt

# 2-way set associative with MRU replacement
./cache_simulator -N 32 -B 4 -I 8 -w 2 -p mru -f ../examples/trace1.txt
```

## Input File Format

The trace file should contain one memory address per line in hexadecimal format:
```
x03
xB4
x2B
x02
xBE
```

## Output Format

The simulator provides detailed output for each memory access:

```
Cache Configuration:
Association: 2-way
Replacement Policy: LRU (Least Recently Used)

Original Address    Tag     Index    Offset         Hit/Miss  Replacement Info
----------------   ------   ------   ------------   --------  ----------------
0x03               0x0     0x0      0x00000011     M        Replaced way 0
0xB4               0xB     0x1      0x00000000     H        Updated way 1
```

### Output Fields
- Original Address: Complete memory address
- Tag: High-order bits for cache line identification
- Index: Set index bits
- Offset: Block offset bits
- Hit/Miss: Cache hit (H) or miss (M)
- Replacement Info: Details about cache line updates/replacements

## Project Structure

```
CacheSimulatorCLI/
├── src/
│   ├── main.cpp           # Main program entry
│   ├── cache.cpp          # Cache implementation
│   ├── cache_set.cpp      # Cache set management
│   ├── address_parser.cpp # Address parsing logic
│   └── visualization.cpp  # Output formatting
├── include/
│   ├── cache.hpp         # Cache class definition
│   ├── cache_set.hpp     # Set associative logic
│   ├── cache_entry.hpp   # Cache line structure
│   └── address_parser.hpp # Address parsing
├── examples/             # Sample trace files
└── CMakeLists.txt       # Build configuration
```

## Cache Organization and Associativity

### Cache Structure
The cache is organized into:
- **Sets**: Groups of cache lines/ways
- **Ways**: Individual storage locations within a set
- **Blocks**: Fixed-size units of data stored in each way

### Associativity Types
1. **Direct-Mapped (1-way)**
   - Each memory address maps to exactly one cache location
   - Simplest to implement but prone to conflict misses
   - No replacement policy needed

2. **N-Way Set Associative**
   - Memory addresses map to a set of N possible cache locations
   - Balances flexibility and implementation complexity
   - Requires replacement policy to choose victim when set is full

3. **Fully Associative**
   - Memory blocks can be placed in any cache location
   - Most flexible but expensive to implement
   - Requires searching entire cache for hits

## Replacement Policies

### Least Recently Used (LRU)
- **Strategy**: Evicts the cache line that hasn't been accessed for the longest time
- **Implementation**: 
  - Maintains access history for each cache line
  - Updates timestamps on every access
- **Advantages**:
  - Good temporal locality exploitation
  - Predictable behavior
- **Disadvantages**:
  - Overhead of maintaining access history
  - May not be optimal for some access patterns

### Most Recently Used (MRU)
- **Strategy**: Evicts the most recently used cache line
- **Implementation**:
  - Similar to LRU but selects most recent instead of least recent
- **Advantages**:
  - Better for cyclic access patterns
  - Simple to implement
- **Disadvantages**:
  - Poor performance for general-purpose caching
  - Counter-intuitive behavior

### First-In-First-Out (FIFO)
- **Strategy**: Evicts the oldest cache line based on insertion time
- **Implementation**:
  - Maintains queue of cache line insertion order
  - No updates on cache hits
- **Advantages**:
  - Simple to implement
  - Lower overhead than LRU
- **Disadvantages**:
  - Doesn't consider access frequency
  - May evict frequently used lines

### Random
- **Strategy**: Randomly selects a cache line for eviction
- **Implementation**:
  - Uses pseudo-random number generator
  - No state maintenance required
- **Advantages**:
  - Extremely simple to implement
  - No overhead
  - Can prevent pathological cases
- **Disadvantages**:
  - Unpredictable performance
  - No exploitation of locality

### OPTIMAL (MIN/OPT/Lookahead)
- **Strategy**: Evicts the cache line that will be used furthest in the future
- **Implementation**:
  - Requires knowledge of future memory accesses
  - Maintains future access positions for each address
  - Uses binary search to find next access
- **Advantages**:
  - Theoretically optimal hit rate
  - Useful as performance baseline
- **Disadvantages**:
  - Not implementable in real systems (requires future knowledge)
  - High implementation complexity
  - Memory overhead for storing future accesses

## Statistics

The simulator provides comprehensive statistics:
- Total accesses
- Cache hits and misses
- Cold misses
- Conflict misses
- Hit rate percentage

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for any improvements.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
