#include <chrono>
#include <tuple>
#include <vector>

#include "cppmorloc.hpp"

std::string make_test_string(int n) {
    std::string result(1024 * 1024 * n, 'x');
    return result;
}

// ANSI color codes
const char* GREEN = "\033[32m"; // Green
const char* RED = "\033[31m";   // Red
const char* RESET = "\033[0m";  // Reset to default

template<typename T>
void generic_test(const std::string& description, const std::string& schema_str, const T& data) {
    try {

        // parse schema
        const char* schema_ptr = schema_str.c_str();
        const Schema* schema = parse_schema(&schema_ptr);

        // convert C++ data to morloc voidstar
        auto start_tovoid = std::chrono::high_resolution_clock::now();
        void* voidstar_in = toAnything(schema, data);
        auto end_tovoid = std::chrono::high_resolution_clock::now();
        auto duration_tovoid = std::chrono::duration_cast<std::chrono::nanoseconds>(end_tovoid - start_tovoid);

        // convert voidstar to MessagePack
        auto start_pack = std::chrono::high_resolution_clock::now();
        char* mesgpack_ptr;
        size_t mesgpack_size;
        int pack_result = pack_with_schema(voidstar_in, schema, &mesgpack_ptr, &mesgpack_size);
        auto end_pack = std::chrono::high_resolution_clock::now();
        auto duration_pack = std::chrono::duration_cast<std::chrono::nanoseconds>(end_pack - start_pack);

        // convert MessagePack back to voidstar
        auto start_unpack = std::chrono::high_resolution_clock::now();
        void* voidstar_out;
        unpack_with_schema(mesgpack_ptr, mesgpack_size, schema, &voidstar_out);
        auto end_unpack = std::chrono::high_resolution_clock::now();
        auto duration_unpack = std::chrono::duration_cast<std::chrono::nanoseconds>(end_unpack - start_unpack);

        // convert voidstar to C++ data
        auto start_fromvoid = std::chrono::high_resolution_clock::now();
        T* dumby = nullptr;
        T return_data = fromAnything(schema, voidstar_out, dumby);
        auto end_fromvoid = std::chrono::high_resolution_clock::now();
        auto duration_fromvoid = std::chrono::duration_cast<std::chrono::nanoseconds>(end_fromvoid - start_fromvoid);

        double tovoid_us = duration_tovoid.count() / 1000.0;
        double pack_us = duration_pack.count() / 1000.0;
        double unpack_us = duration_unpack.count() / 1000.0;
        double fromvoid_us = duration_fromvoid.count() / 1000.0;
        double total = tovoid_us + pack_us + unpack_us + fromvoid_us;

        if(return_data == data){
            printf("%s: ... %spass%s (%.2f(%.2f), %.2f(%.2f), %.2f(%.2f), %.2f(%.2f))\n", 
                   description.c_str(), GREEN, RESET, 
                   tovoid_us, tovoid_us / total,
                   pack_us, pack_us / total,
                   unpack_us, unpack_us / total,
                   fromvoid_us, fromvoid_us / total);
        } else {
            printf("%s: ... %svalue fail%s\n", 
                   description.c_str(), RED, RESET);
        }
    } catch (const std::exception& e) {
        printf("%s: ... %serror: %s%s\n", description.c_str(), RED, e.what(), RESET);
    }
}

int main() {

    shinit("morloc-cpptest", 0, 0x100);

    generic_test("Test long string (1M)", "s", make_test_string(1));
    generic_test("Test long string (2M)", "s", make_test_string(2));
    generic_test("Test long string (4M)", "s", make_test_string(4));
    generic_test("Test long string (8M)", "s", make_test_string(8));
    generic_test("Test long string (16M)", "s", make_test_string(16));
    generic_test("Test long string (32M)", "s", make_test_string(32));
    generic_test("Test long string (64M)", "s", make_test_string(64));
    generic_test("Test long string (128M)", "s", make_test_string(128));
    generic_test("Test long string (256M)", "s", make_test_string(256));
  
    shclose();

    return 0;
}
