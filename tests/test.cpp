#include "cppmorloc.hpp"
#include <tuple>
#include <vector>
#include <algorithm>

std::vector<int32_t> generate_integers() {
    // Maximum and minimum 32-bit signed integers
    int64_t max_int = 0x000000007FFFFFFF;
    int64_t min_int = 0xFFFFFFFF80000000;

    // Generate 1024 evenly spaced values, powers of 2, and sums of powers of 2
    std::vector<int64_t> all_values;
    for (int64_t i = max_int; i >= min_int; i -= (max_int - min_int) / 1023) {
        all_values.push_back(i);
    }

    int64_t current_sum = 0;
    for (int i = 0; i < 32; ++i) {
        const int64_t base = static_cast<int64_t>(1 << i);
        current_sum += base;

        all_values.push_back(base - 1);
        all_values.push_back(base);
        all_values.push_back(base + 1);
        all_values.push_back(-base + 1);
        all_values.push_back(-base);
        all_values.push_back(-base - 1);

        all_values.push_back(current_sum - 1);
        all_values.push_back(current_sum);
        all_values.push_back(current_sum + 1);
        all_values.push_back(-current_sum + 1);
        all_values.push_back(-current_sum);
        all_values.push_back(-current_sum - 1);

        if (i > 0) {
            const int64_t intermediate_sum = base + (1 << (i - 1));
            all_values.push_back(intermediate_sum - 1);
            all_values.push_back(intermediate_sum);
            all_values.push_back(intermediate_sum + 1);
            all_values.push_back(-intermediate_sum + 1);
            all_values.push_back(-intermediate_sum);
            all_values.push_back(-intermediate_sum - 1);
        }
    }

    // Convert to int32_t and remove values outside the 32-bit range
    std::vector<int32_t> result;
    for (int64_t value : all_values) {
        if (value >= min_int && value <= max_int) {
            result.push_back(static_cast<int32_t>(value));
        }
    }
    return result;
}

template <typename T>
std::vector<T> range(T start, size_t by, size_t n_values){
  std::vector<T> result;
  for(size_t i = 0; i < n_values; i++){
    result.push_back(start + (T)(i * by));
  }
  return result;
}


typedef struct Person{
  std::string name;
  uint32_t age;
} Person;

bool operator==(const Person& lhs, const Person& rhs) {
    return (lhs.name == rhs.name) && (lhs.age == rhs.age);
}

Person fromAnything(const Schema* schema, const void* anything, Person* dummy = nullptr)
{
    Person person;

    std::string* elemental_dumby_0 = nullptr;
    person.name = fromAnything(schema->parameters[0], (char*)anything + schema->offsets[0], elemental_dumby_0);

    uint32_t* elemental_dumby_1 = nullptr;
    person.age = fromAnything(schema->parameters[1], (char*)anything + schema->offsets[1], elemental_dumby_1);

    return person;
}

void* toAnything(void* dest, const Schema* schema, const Person& obj)
{
    return toAnything(dest, schema, std::make_tuple(obj.name, obj.age));
}



typedef struct Person2{
  std::string name;
  uint32_t age;
  uint32_t weight;
} Person2;

bool operator==(const Person2& lhs, const Person2& rhs) {
    return (lhs.name == rhs.name) && (lhs.age == rhs.age) && (lhs.weight == rhs.weight);
}


Person2 fromAnything(const Schema* schema, const void* anything, Person2* dummy = nullptr)
{
    Person2 person;

    std::string* elemental_dumby_0 = nullptr;
    person.name = fromAnything(schema->parameters[0], (char*)anything + schema->offsets[0], elemental_dumby_0);

    uint32_t* elemental_dumby_1 = nullptr;
    person.age = fromAnything(schema->parameters[1], (char*)anything + schema->offsets[1], elemental_dumby_1);

    uint32_t* elemental_dumby_2 = nullptr;
    person.weight = fromAnything(schema->parameters[2], (char*)anything + schema->offsets[2], elemental_dumby_2);

    return person;
}

void* toAnything(void* dest, const Schema* schema, const Person2& obj)
{
    return toAnything(dest, schema, std::make_tuple(obj.name, obj.age, obj.weight));
}




template<typename T>
struct PersonPlus {
    std::string name;
    int age;
    T info;
};

template<typename T>
bool operator==(const PersonPlus<T>& lhs, const PersonPlus<T>& rhs) {
    return (lhs.name == rhs.name) && (lhs.age == rhs.age) && (lhs.info == rhs.info);
}

template<typename T>
PersonPlus<T> fromAnything(const Schema* schema, const void* anything, PersonPlus<T>* dummy = nullptr)
{
    PersonPlus<T> person;

    std::string* elemental_dumby_0 = nullptr;
    person.name = fromAnything(schema->parameters[0], (char*)anything + schema->offsets[0], elemental_dumby_0);

    int* elemental_dumby_1 = nullptr;
    person.age = fromAnything(schema->parameters[1], (char*)anything + schema->offsets[1], elemental_dumby_1);

    T* elemental_dumby_2 = nullptr;
    person.info = fromAnything(schema->parameters[2], (char*)anything + schema->offsets[2], elemental_dumby_2);

    return person;
}

template<typename T>
void* toAnything(void* dest, const Schema* schema, const PersonPlus<T>& obj)
{
    return toAnything(dest, schema, std::make_tuple(obj.name, obj.age, obj.info));
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
        void* voidstar_in = toAnything(NULL, schema, data);

        // convert voidstar to MessagePack
        char* mesgpack_ptr;
        size_t mesgpack_size;
        int pack_result = pack_with_schema(voidstar_in, schema, &mesgpack_ptr, &mesgpack_size);

        // convert MessagePack back to voidstar
        void* voidstar_out;
        unpack_with_schema(mesgpack_ptr, mesgpack_size, schema, &voidstar_out);

        // convert voidstar to C++ data
        T* dumby = nullptr;
        T return_data = fromAnything(schema, voidstar_out, dumby);

        if(return_data == data){
            printf("%s: ... %spass%s\n", description.c_str(), GREEN, RESET);
        } else {
            printf("%s: ... %svalue fail%s\n", description.c_str(), RED, RESET);
        }
    } catch (const std::exception& e) {
        printf("%s: ... %serror: %s%s\n", description.c_str(), RED, e.what(), RESET);
    }
}

int main() {

    Person alice;
    alice.name = "Alice";
    alice.age = 42;

    Person2 bob;
    bob.name = "Bob";
    bob.age = 42;
    bob.weight = 45;

    PersonPlus<double> alice2;
    alice2.name = "Alice";
    alice2.age = 42;
    alice2.info = 6.9;

    generic_test<uint8_t>("Test boolean", "b", true);
    generic_test("Test float64", "f8", (double)3.14);
    generic_test("Test float32", "f4", (float)3.14);
    generic_test("Test int8",  "i1", (int8_t)14);
    generic_test("Test int16", "i2", (int16_t)14);
    generic_test("Test int32", "i4", (int32_t)14);
    generic_test("Test int64", "i8", (int64_t)14);
    generic_test("Test uint8",  "u1", (uint8_t)14);
    generic_test("Test uint16", "u2", (uint16_t)14);
    generic_test("Test uint32", "u4", (uint32_t)14);
    generic_test("Test uint32 max", "u4", (uint32_t)0xffffffff);
    generic_test("Test uint64", "u8", (uint64_t)14);
    generic_test("Test uint64 max", "u8", (uint64_t)0xffffffffffffffff);

    generic_test("Test string", "s", std::string("Helloooo"));
    generic_test("Test raw binary", "au1", std::vector<uint8_t>{0x01, 0x02, 0x03});
    generic_test("Test null susan", "au1", std::vector<uint8_t>{0x00, 0x00, 0x73, 0x75, 0x73, 0x61, 0x6E});

    generic_test("Test array of booleans", "ab", std::vector<uint8_t>{true,false,true});
    generic_test("Test array of integers", "ai4", std::vector<int32_t>{1, 2, 3, 4, 5});
    generic_test("Test array of float", "af4", std::vector<float>{1.0, 2.0, 3.0});
    generic_test("Test array of doubles", "af8", std::vector<double>{1.0, 2.0, 3.0});
    generic_test("Test array of arrays of booleans", "aab", std::vector<std::vector<uint8_t>>{std::vector<uint8_t>{true,false,true}, std::vector<uint8_t>{false,true}});
    generic_test("Test array of arrays of int32", "aai4", std::vector<std::vector<int32_t>>{std::vector<int32_t>{99,-42}, std::vector<int32_t>{12,-4}});

    generic_test("Test tuple of int and array of floats", "t2i4af8", std::make_tuple(42, std::vector<double>{1.1, 2.2, 3.3}));
    generic_test("tuple 1a", "t2au2au1", std::make_tuple(std::vector<uint16_t>{1,300,3}, std::vector<uint8_t>{1,2,3}));
    generic_test("tuple 1b", "t3au2au1au2", std::make_tuple(std::vector<uint16_t>{1,300,3}, std::vector<uint8_t>{1,2,3}, std::vector<uint16_t>{1,300,3}));
    generic_test("tuple 2a", "t4bi4f8au1", std::make_tuple(true, 44, 42.7, std::vector<uint8_t>{1,2,3}));
    generic_test("tuple 2b", "t4i4bf8au1", std::make_tuple(44, true, 42.7, std::vector<uint8_t>{1,2,3}));
    generic_test("tuple 2c", "t4i4f8au1b", std::make_tuple(44, 42.7, std::vector<uint8_t>{1,2,3}, true));
    generic_test("tuple 3a", "t3au2au1b", std::make_tuple(std::vector<uint16_t>{1,2,3}, std::vector<uint8_t>{1,2,3,4,5}, true));
    generic_test("tuple 3b", "t3au2bau1", std::make_tuple(std::vector<uint16_t>{1,2,3}, true, std::vector<uint8_t>{1,2,3,4,5}));
    generic_test("tuple 3c", "t3bau2au1", std::make_tuple(true, std::vector<uint16_t>{1,2,3}, std::vector<uint8_t>{1,2,3,4,5}));

    generic_test("tuple 4a", "t2bs", std::make_tuple(true, std::string("Bob")));
    generic_test("tuple 4b", "t2sb", std::make_tuple(std::string("Bob"), true));
    generic_test("tuple 4c", "t2u4s", std::make_tuple((uint32_t)42, std::string("Bob")));
    generic_test("tuple 4d", "t2su4", std::make_tuple(std::string("Bob"), (uint32_t)42));
    generic_test("tuple 4e", "t4i4bf8s", std::make_tuple(44, true, 42.7, std::string("Bob")));
    generic_test("tuple 4f", "t3su4u4", std::make_tuple(std::string("Bob"), (uint32_t)42, (uint32_t)56));

    generic_test("tuple 5", "at2abb", std::vector<std::tuple<std::vector<uint8_t>,uint8_t>>{std::make_tuple(std::vector<uint8_t>{true, false}, true)});

    generic_test("exhaustive edge cases for i32", "ai4", generate_integers());
    generic_test("range(1500) au1", "au1", range<uint8_t>(  0, 1, 1500));
    generic_test("range(1500) au2", "au2", range<uint16_t>( 0, 1, 1500));
    generic_test("range(1500) au4", "au4", range<uint32_t>( 0, 1, 1500));
    generic_test("range(1500) au8", "au8", range<uint64_t>( 0, 1, 1500));
    generic_test("range(1500) ai1", "ai1", range<int8_t>(   0, 1, 1500));
    generic_test("range(1500) ai2", "ai2", range<int16_t>(  0, 1, 1500));
    generic_test("range(1500) ai4", "ai4", range<int32_t>(  0, 1, 1500));
    generic_test("range(1500) ai8", "ai8", range<int64_t>(  0, 1, 1500));

    generic_test("Test Alice", "m24names3ageu4", alice);
    generic_test("Test Bob weighted", "m34names3ageu46weightu4", bob);
    generic_test("Test Alice generic", "m34names3agei44infof8", alice2);

    return 0;
}
