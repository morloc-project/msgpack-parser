#include "cppmpack.hpp"
#include <tuple>



typedef struct Person{
  std::string name;
  uint32_t age;
} Person;

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
        std::vector<char> msgpack_data = mpk_pack(data, schema_str);
        T data_ret = mpk_unpack<T>(msgpack_data, schema_str);
        fprintf(stderr, "%s: ... %spass%s\n", description.c_str(), GREEN, RESET);
    } catch (const std::exception& e) {
        fprintf(stderr, "%s: ... %sfail: %s%s\n", description.c_str(), RED, e.what(), RESET);
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

    generic_test<bool>("Test boolean", "b", true);
    generic_test("Test float64", "f8", 3.14);
    generic_test("Test float32", "f4", 3.14);
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
    generic_test("Test array of booleans", "ab", std::vector<bool>{true,false,true});
    generic_test("Test array of integers", "ai4", std::vector<int32_t>{1, 2, 3, 4, 5});
    generic_test("Test array of float", "af4", std::vector<float>{1.0, 2.0, 3.0});
    generic_test("Test array of doubles", "af8", std::vector<double>{1.0, 2.0, 3.0});
    generic_test("Test array of arrays of booleans", "aab", std::vector<std::vector<bool>>{std::vector<bool>{true,false,true}, std::vector<bool>{false,true}});
    generic_test("Test array of arrays of int32", "aai4", std::vector<std::vector<int32_t>>{std::vector<int32_t>{99,-42}, std::vector<int32_t>{12,-4}});

    generic_test("Test tuple of int and array of floats", "t2i4af8", std::make_tuple(42, std::vector<double>{1.1, 2.2, 3.3}));
    generic_test("Test tuple 1a", "t2au2au1", std::make_tuple(std::vector<uint16_t>{1,300,3}, std::vector<uint8_t>{1,2,3}));
    generic_test("Test tuple 1b", "t3au2au1au2", std::make_tuple(std::vector<uint16_t>{1,300,3}, std::vector<uint8_t>{1,2,3}, std::vector<uint16_t>{1,300,3}));
    generic_test("Test tuple 2a", "t4bi4f8au1", std::make_tuple(true, 44, 42.7, std::vector<uint8_t>{1,2,3}));
    generic_test("Test tuple 2b", "t4i4bf8au1", std::make_tuple(44, true, 42.7, std::vector<uint8_t>{1,2,3}));
    generic_test("Test tuple 2c", "t4i4f8au1b", std::make_tuple(44, 42.7, std::vector<uint8_t>{1,2,3}, true));
    generic_test("Test tuple 3a", "t3au2au1b", std::make_tuple(std::vector<uint16_t>{1,2,3}, std::vector<uint8_t>{1,2,3,4,5}, true));
    generic_test("Test tuple 3b", "t3au2bau1", std::make_tuple(std::vector<uint16_t>{1,2,3}, true, std::vector<uint8_t>{1,2,3,4,5}));
    generic_test("Test tuple 3c", "t3bau2au1", std::make_tuple(true, std::vector<uint16_t>{1,2,3}, std::vector<uint8_t>{1,2,3,4,5}));

    generic_test("Test tuple weighted", "t4i4bf8s", std::make_tuple(44, true, 42.7, "Bob"));
    generic_test("Test tuple weighted", "t3su4u4", std::make_tuple("Bob", (uint32_t)42, (uint32_t)56));

    generic_test("Test Alice", "m24names3ageu4", alice);
    generic_test("Test Bob weighted", "m34names3ageu46weightu4", bob);
    generic_test("Test Alice generic", "m34names3agei44infof8", alice2);

    return 0;
}
