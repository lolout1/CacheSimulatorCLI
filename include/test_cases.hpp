#pragma once
#include <vector>
#include <string>
#include "cache_set.hpp"
struct TestCase {
    std::string name;
    std::vector<std::string> addresses;
    int N;
    int B;
    int I;
    int ways;
    ReplacementPolicy policy;
};
namespace TestCases {
    inline TestCase getHW1Test() {
        return TestCase{
            "HW1 - Direct Mapping",
            {
                "x00", "xFD", "x01", "xB4", "x2B", "xB5", 
                "x02", "xBC", "xBE", "x03", "x58", "xBF", "x2C"
            },
            32,    
            8,     
            16,    
            1,     
            ReplacementPolicy::LRU
        };
    }
    inline TestCase getHW2Test() {
        return TestCase{
            "HW2 - Cache Analysis",
            {
                "x00", "x04", "x10", "x08", "x84", "xE8", 
                "xA0", "x400", "x14", "x8C", "xC1C", "xB4", "x884"
            },
            32,    
            32,    
            16,    
            1,     
            ReplacementPolicy::LRU
        };
    }
    inline TestCase getHW3Test() {
        return TestCase{
            "HW3 - Set Associative",
            {
                "x03", "xB4", "x2B", "x02", "xBE", "x58", 
                "xBF", "x0E", "x1F", "xB5", "xBF", "xBA", "x2E", "xCE"
            },
            32,    
            8,     
            16,    
            3,     
            ReplacementPolicy::LRU
        };
    }
}
