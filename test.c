#include "mlcmpack.h"

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { printf("Running test_%s...\n", #name); test_##name(); printf("test_%s passed!\n\n", #name); } while(0)

// Helper function to compare schemas
int compare_schemas(const Schema* s1, const Schema* s2) {
    if (s1->type != s2->type || s1->size != s2->size) return 0;
    for (size_t i = 0; i < s1->size; i++) {
        if (!compare_schemas(s1->parameters[i], s2->parameters[i])) return 0;
        if (s1->keys && s2->keys && strcmp(s1->keys[i], s2->keys[i]) != 0) return 0;
    }
    return 1;
}

TEST(nil_schema) {
    Schema* schema = nil_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_NIL);
    assert(schema->size == 0);
    free_schema(schema);
}

TEST(bool_schema) {
    Schema* schema = bool_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_BOOL);
    assert(schema->size == 0);
    free_schema(schema);
}

TEST(sint_schema) {
    Schema* schema = sint_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_SINT);
    assert(schema->size == 0);
    free_schema(schema);
}

TEST(uint_schema) {
    Schema* schema = uint_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_UINT);
    assert(schema->size == 0);
    free_schema(schema);
}

TEST(float_schema) {
    Schema* schema = float_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_FLOAT);
    assert(schema->size == 0);
    free_schema(schema);
}

TEST(bool_array_schema) {
    Schema* schema = bool_array_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_BOOL_ARRAY);
    assert(schema->size == 1);
    free_schema(schema);
}

TEST(sint_array_schema) {
    Schema* schema = sint_array_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_SINT_ARRAY);
    assert(schema->size == 1);
    free_schema(schema);
}

TEST(uint_array_schema) {
    Schema* schema = uint_array_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_UINT_ARRAY);
    assert(schema->size == 1);
    free_schema(schema);
}

TEST(float_array_schema) {
    Schema* schema = float_array_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_FLOAT_ARRAY);
    assert(schema->size == 1);
    free_schema(schema);
}

TEST(string_schema) {
    Schema* schema = string_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_STRING);
    assert(schema->size == 0);
    free_schema(schema);
}

TEST(binary_schema) {
    Schema* schema = binary_schema();
    assert(schema != NULL);
    assert(schema->type == MORLOC_BINARY);
    assert(schema->size == 0);
    free_schema(schema);
}

TEST(tuple_schema) {
    Schema* schema = tuple_schema(2, sint_schema(), float_schema());
    assert(schema != NULL);
    assert(schema->type == MORLOC_TUPLE);
    assert(schema->size == 2);
    assert(schema->parameters[0]->type == MORLOC_SINT);
    assert(schema->parameters[1]->type == MORLOC_FLOAT);
    free_schema(schema);
}

TEST(array_schema) {
    Schema* element_schema = sint_schema();
    Schema* schema = array_schema(element_schema);
    assert(schema != NULL);
    assert(schema->type == MORLOC_ARRAY);
    assert(schema->size == 1);
    assert(schema->parameters[0]->type == MORLOC_SINT);
    free_schema(schema);
}

TEST(map_schema) {
    SchemaKeyValuePair* kvp1 = kvp_schema("key1", sint_schema());
    SchemaKeyValuePair* kvp2 = kvp_schema("key2", float_schema());
    Schema* schema = map_schema(2, kvp1, kvp2);
    assert(schema != NULL);
    assert(schema->type == MORLOC_MAP);
    assert(schema->size == 2);
    assert(strcmp(schema->keys[0], "key1") == 0);
    assert(schema->parameters[0]->type == MORLOC_SINT);
    assert(strcmp(schema->keys[1], "key2") == 0);
    assert(schema->parameters[1]->type == MORLOC_FLOAT);
    free_schema(schema);
}

TEST(nil_data) {
    ParsedData* data = nil_data();
    assert(data != NULL);
    assert(data->type == MORLOC_NIL);
    free_parsed_data(data);
}

TEST(bool_data) {
    ParsedData* data = bool_data(true);
    assert(data != NULL);
    assert(data->type == MORLOC_BOOL);
    assert(data->data.bool_val == true);
    free_parsed_data(data);
}

TEST(sint_data) {
    ParsedData* data = sint_data(-42);
    assert(data != NULL);
    assert(data->type == MORLOC_SINT);
    assert(data->data.sint_val == -42);
    free_parsed_data(data);
}

TEST(uint_data) {
    ParsedData* data = uint_data(42);
    assert(data != NULL);
    assert(data->type == MORLOC_UINT);
    assert(data->data.uint_val == 42);
    free_parsed_data(data);
}

TEST(float_data) {
    ParsedData* data = float_data(3.14);
    assert(data != NULL);
    assert(data->type == MORLOC_FLOAT);
    assert(data->data.double_val == 3.14);
    free_parsed_data(data);
}

TEST(string_data) {
    ParsedData* data = string_data("test");
    assert(data != NULL);
    assert(data->type == MORLOC_STRING);
    assert(strcmp(data->data.string_val, "test") == 0);
    free_parsed_data(data);
}

TEST(binary_data) {
    const char* binary = "\x01\x02\x03\x04";
    ParsedData* data = binary_data(binary, 4);
    assert(data != NULL);
    assert(data->type == MORLOC_BINARY);
    assert(data->data.binary_val.size == 4);
    assert(memcmp(data->data.binary_val.elements, binary, 4) == 0);
    free_parsed_data(data);
}

TEST(array_bool_data) {
    bool values[] = {true, false, true};
    ParsedData* data = array_bool_data(values, 3);
    assert(data != NULL);
    assert(data->type == MORLOC_BOOL_ARRAY);
    assert(data->data.array_bool_val.size == 3);
    assert(data->data.array_bool_val.elements[0] == true);
    assert(data->data.array_bool_val.elements[1] == false);
    assert(data->data.array_bool_val.elements[2] == true);
    free_parsed_data(data);
}

TEST(array_sint_data) {
    int values[] = {1, -2, 3};
    ParsedData* data = array_sint_data(values, 3);
    assert(data != NULL);
    assert(data->type == MORLOC_SINT_ARRAY);
    assert(data->data.array_sint_val.size == 3);
    assert(data->data.array_sint_val.elements[0] == 1);
    assert(data->data.array_sint_val.elements[1] == -2);
    assert(data->data.array_sint_val.elements[2] == 3);
    free_parsed_data(data);
}

TEST(array_uint_data) {
    unsigned int values[] = {1, 2, 3};
    ParsedData* data = array_uint_data(values, 3);
    assert(data != NULL);
    assert(data->type == MORLOC_UINT_ARRAY);
    assert(data->data.array_uint_val.size == 3);
    assert(data->data.array_uint_val.elements[0] == 1);
    assert(data->data.array_uint_val.elements[1] == 2);
    assert(data->data.array_uint_val.elements[2] == 3);
    free_parsed_data(data);
}

TEST(array_float_data) {
    double values[] = {1.1, 2.2, 3.3};
    ParsedData* data = array_float_data(values, 3);
    assert(data != NULL);
    assert(data->type == MORLOC_FLOAT_ARRAY);
    assert(data->data.array_float_val.size == 3);
    assert(data->data.array_float_val.elements[0] == 1.1);
    assert(data->data.array_float_val.elements[1] == 2.2);
    assert(data->data.array_float_val.elements[2] == 3.3);
    free_parsed_data(data);
}

TEST(array_data) {
    ParsedData* data = array_data(2);
    assert(data != NULL);
    assert(data->type == MORLOC_ARRAY);
    assert(data->data.array_val.size == 2);
    data->data.array_val.elements[0] = sint_data(1);
    data->data.array_val.elements[1] = float_data(2.2);
    assert(data->data.array_val.elements[0]->type == MORLOC_SINT);
    assert(data->data.array_val.elements[1]->type == MORLOC_FLOAT);
    free_parsed_data(data);
}

TEST(tuple_data) {
    ParsedData* data = tuple_data(2);
    assert(data != NULL);
    assert(data->type == MORLOC_TUPLE);
    assert(data->data.array_val.size == 2);
    data->data.array_val.elements[0] = sint_data(1);
    data->data.array_val.elements[1] = float_data(2.2);
    assert(data->data.array_val.elements[0]->type == MORLOC_SINT);
    assert(data->data.array_val.elements[1]->type == MORLOC_FLOAT);
    free_parsed_data(data);
}

TEST(map_data) {
    ParsedData* data = map_data(2);
    assert(data != NULL);
    assert(data->type == MORLOC_MAP);
    assert(data->data.map_val.size == 2);
    set_map_element(data, "key1", sint_data(1));
    set_map_element(data, "key2", float_data(2.2));
    assert(strcmp(data->data.map_val.keys[0], "key1") == 0);
    assert(data->data.map_val.elements[0]->type == MORLOC_SINT);
    assert(strcmp(data->data.map_val.keys[1], "key2") == 0);
    assert(data->data.map_val.elements[1]->type == MORLOC_FLOAT);
    free_parsed_data(data);
}

TEST(unpack_with_schema) {
    Schema* schema = uint_schema();
    const char* buf = "\x2A"; // MessagePack encoding for 42
    size_t buf_remaining = 1;
    ParsedData* data = unpack_with_schema(&buf, &buf_remaining, schema);
    assert(data != NULL);
    assert(data->type == MORLOC_UINT);
    assert(data->data.uint_val == 42);
    free_parsed_data(data);
    free_schema(schema);
}

TEST(pack_with_schema) {
    Schema* schema = sint_schema();
    ParsedData* data = sint_data(42);
    char* packet = NULL;
    size_t packet_size = 0;
    int result = pack_with_schema(data, schema, &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 1);
    assert(packet[0] == 0x2A); // MessagePack encoding for 42
    free(packet);
    free_parsed_data(data);
    free_schema(schema);
}


typedef struct {
    const char* packed_data;
    size_t packed_size;
    long long expected_value;
    const char* description;
} IntegerTestCase;

IntegerTestCase signed_integer_tests[] = {
    // Positive fixint
    {"\x00", 1, 0, "Positive fixint: 0"},
    {"\x10", 1, 16, "Positive fixint: 16"},
    {"\x7f", 1, 127, "Positive fixint: 127"},

    // Negative fixint
    {"\xff", 1, -1, "Negative fixint: -1"},
    {"\xe0", 1, -32, "Negative fixint: -32"},

    // int 8
    {"\xd0\x80", 2, -128, "int 8: -128"},
    {"\xd0\x7f", 2, 127, "int 8: 127"},

    // int 16
    {"\xd1\xff\x80", 3, -128, "int 16: -128"},
    {"\xd1\x80\x00", 3, -32768, "int 16: -32768"},
    {"\xd1\x7f\xff", 3, 32767, "int 16: 32767"},

    // int 32
    {"\xd2\xff\xff\xff\x80", 5, -128, "int 32: -128"},
    {"\xd2\x80\x00\x00\x00", 5, -2147483648LL, "int 32: -2147483648"},
    {"\xd2\x7f\xff\xff\xff", 5, 2147483647, "int 32: 2147483647"}

    /* // int 64 (testing full range)                                                                           */
    /* {"\xd3\x80\x00\x00\x00\x00\x00\x00\x00", 9, -9223372036854775807LL - 1, "int 64: -9223372036854775808"}, */
    /* {"\xd3\x7f\xff\xff\xff\xff\xff\xff\xff", 9, 9223372036854775807LL, "int 64: 9223372036854775807"},       */
};

IntegerTestCase unsigned_integer_tests[] = {
    // Positive fixint
    {"\x00", 1, 0, "Positive fixint: 0"},
    {"\x10", 1, 16, "Positive fixint: 16"},
    {"\x7f", 1, 127, "Positive fixint: 127"},

    // uint 8
    {"\xcc\x80", 2, 128, "uint 8: 128"},
    {"\xcc\xff", 2, 255, "uint 8: 255"},

    // uint 16
    {"\xcd\x01\x00", 3, 256, "uint 16: 256"},
    {"\xcd\xff\xff", 3, 65535, "uint 16: 65535"},

    // uint 32
    {"\xce\x00\x01\x00\x00", 5, 65536, "uint 32: 65536"},
    {"\xce\xff\xff\xff\xff", 5, 4294967295ULL, "uint 32: 4294967295"}

    /* // uint 64                                                                                             */
    /* {"\xcf\x00\x00\x00\x01\x00\x00\x00\x00", 9, 4294967296ULL, "uint 64: 4294967296"},                     */
    /* {"\xcf\xff\xff\xff\xff\xff\xff\xff\xff", 9, 18446744073709551615ULL, "uint 64: 18446744073709551615"}, */
};

void test_integer_unpacking(Schema* schema, IntegerTestCase* tests, size_t test_count, const char* test_type) {
    for (size_t i = 0; i < test_count; i++) {
        IntegerTestCase test = tests[i];
        const char* buf = test.packed_data;
        size_t buf_remaining = test.packed_size;
        
        ParsedData* data = unpack_with_schema(&buf, &buf_remaining, schema);
        
        assert(data != NULL);
        if (schema->type == MORLOC_SINT) {
            assert(data->type == MORLOC_SINT);
            assert(data->data.sint_val == (int)test.expected_value);
        } else {
            assert(data->type == MORLOC_UINT);
            assert(data->data.uint_val == (unsigned int)test.expected_value);
        }
        
        printf("Passed: %s - %s\n", test_type, test.description);
        
        free_parsed_data(data);
    }
}



int main() {
    RUN_TEST(nil_schema);
    RUN_TEST(bool_schema);
    RUN_TEST(sint_schema);
    RUN_TEST(uint_schema);
    RUN_TEST(float_schema);
    RUN_TEST(bool_array_schema);
    RUN_TEST(sint_array_schema);
    RUN_TEST(uint_array_schema);
    RUN_TEST(float_array_schema);
    RUN_TEST(string_schema);
    RUN_TEST(binary_schema);
    RUN_TEST(tuple_schema);
    RUN_TEST(array_schema);
    RUN_TEST(map_schema);
    RUN_TEST(nil_data);
    RUN_TEST(bool_data);
    RUN_TEST(sint_data);
    RUN_TEST(uint_data);
    RUN_TEST(float_data);
    RUN_TEST(string_data);
    RUN_TEST(binary_data);
    RUN_TEST(array_bool_data);
    RUN_TEST(array_sint_data);
    RUN_TEST(array_uint_data);
    RUN_TEST(array_float_data);
    RUN_TEST(array_data);
    RUN_TEST(tuple_data);
    RUN_TEST(map_data);
    RUN_TEST(unpack_with_schema);
    RUN_TEST(pack_with_schema);

    // integer testing
    Schema* sint_schema_obj = sint_schema();
    Schema* uint_schema_obj = uint_schema();

    printf("Testing signed integers:\n");
    test_integer_unpacking(sint_schema_obj, signed_integer_tests, 
                           sizeof(signed_integer_tests) / sizeof(IntegerTestCase), 
                           "Signed");

    printf("\nTesting unsigned integers:\n");
    test_integer_unpacking(uint_schema_obj, unsigned_integer_tests, 
                           sizeof(unsigned_integer_tests) / sizeof(IntegerTestCase), 
                           "Unsigned");

    free_schema(sint_schema_obj);
    free_schema(uint_schema_obj);

    printf("\nAll integer unpacking tests passed!\n");
    return 0;
}
