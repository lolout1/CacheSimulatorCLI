
#include <vector>
#include <string>
struct HomeworkTestCase {
    std::string name;
    std::vector<std::string> addresses;
    int N;
    int B;
    int I;
    int ways;
    ReplacementPolicy policy;
};
HomeworkTestCase hw1_test = {
    "Problem 1 - Direct Mapping",
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
HomeworkTestCase hw2_test = {
    "Problem 2 - Cache Analysis",
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
HomeworkTestCase hw3_test = {
    "Problem 3 - Set Associative",
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
