#include "cppmpack.hpp"

// ANSI color codes
const char* GREEN = "\033[32m"; // Green
const char* RED = "\033[31m";   // Red
const char* RESET = "\033[0m";  // Reset to default

template<typename T>
void generic_test(const std::string& description, const std::string& schema_str, const T& data) {
    try {
        std::vector<char> msgpack_data = mpk_pack(data, schema_str);
        T data_ret = mpk_unpack<T>(msgpack_data, schema_str);
        fprintf(stderr, "%s: ... %spass%s\n", description.c_str(), GREEN, RESET);
    } catch (const std::exception& e) {
        fprintf(stderr, "%s: ... %sfail: %s%s\n", description.c_str(), RED, e.what(), RESET);
    }
}

int main() {
    generic_test<bool>("Test boolean", "b", true);
    generic_test("Test float", "f", 3.14);
    generic_test("Test integer", "i4", 14);
    generic_test("Test string", "s", std::string("Hello"));
    generic_test<>("Test raw binary", "r", std::vector<char>{0x01, 0x02, 0x03});
    generic_test("Test array of integers", "ai4", std::vector<int>{1, 2, 3, 4, 5});
    generic_test("Test array of floats", "af8", std::vector<double>{1.0, 2.0, 3.0});
    generic_test("Test array of booleans", "ab", std::vector<bool>{true,false,true});
    generic_test("Test array of arrays of booleans", "aab", std::vector<std::vector<bool>>{std::vector<bool>{true,false,true}, std::vector<bool>{false,true}});
    generic_test("Test tuple of int and array of floats", "t2i4af8", std::make_tuple(42, std::vector<double>{1.1, 2.2, 3.3}));

    return 0;
}
