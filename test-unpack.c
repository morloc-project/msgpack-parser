#include <assert.h>
#include "mlcmpack.h"
#include <math.h>

typedef struct {
    const char* packed_data;
    size_t data_size;
    Schema* (*create_schema)();
    void (*verify_data)(ParsedData*);
    const char* description;
} UnpackTestCase;

// Schema creation functions
Schema* create_nil_schema() { return nil_schema(); }
Schema* create_bool_schema() { return bool_schema(); }
Schema* create_int_schema() { return int_schema(); }
Schema* create_float_schema() { return float_schema(); }
Schema* create_string_schema() { return string_schema(); }
Schema* create_binary_schema() { return binary_schema(); }
Schema* create_array_schema() { return array_schema(int_schema()); }
Schema* create_empty_array_schema() { return array_schema(int_schema()); }
Schema* create_map_schema() {
    SchemaKeyValuePair* kvp1 = kvp_schema("key1", int_schema());
    SchemaKeyValuePair* kvp2 = kvp_schema("key2", string_schema());
    return map_schema(2, kvp1, kvp2);
}
Schema* create_empty_map_schema() { return map_schema(0); }

// Verification functions
void verify_nil(ParsedData* data) {
    assert(data->type == MORLOC_NIL);
}

void verify_bool(ParsedData* data) {
    assert(data->type == MORLOC_BOOL);
    assert(data->data.bool_val == true);
}

void verify_sint(ParsedData* data) {
    assert(data->type == MORLOC_INT);
    assert(data->data.int_val == 42);
}

void verify_uint(ParsedData* data) {
    assert(data->type == MORLOC_INT);
    assert(data->data.int_val == 300);
}

void verify_float(ParsedData* data) {
    assert(data->type == MORLOC_FLOAT);
    assert(fabs(data->data.double_val - 3.14) < 0.0001);
}

void verify_string(ParsedData* data) {
    assert(data->type == MORLOC_STRING);
    assert(memcmp(data->data.char_arr, "hello", 5) == 0);
}

void verify_binary(ParsedData* data) {
    assert(data->type == MORLOC_BINARY);
    assert(data->size == 3);
    assert(memcmp(data->data.char_arr, "\x01\x02\x03", 3) == 0);
}

void verify_array(ParsedData* data) {
    assert(data->type == MORLOC_INT_ARRAY);
    assert(data->size == 3);
    assert(data->data.int_arr[0] == 1);
    assert(data->data.int_arr[1] == 2);
    assert(data->data.int_arr[2] == 3);
}

void verify_empty_array(ParsedData* data) {
    assert(data->type == MORLOC_INT_ARRAY);
    assert(data->size == 0);
}

void verify_map(ParsedData* data) {
    assert(data->type == MORLOC_MAP);
    assert(data->size == 2);
    assert(strcmp(data->data.obj_arr[0]->key, "key1") == 0);
    assert(data->data.obj_arr[0]->type == MORLOC_INT);
    assert(data->data.obj_arr[0]->data.int_val == 42);
    assert(strcmp(data->data.obj_arr[1]->key, "key2") == 0);
    assert(data->data.obj_arr[1]->type == MORLOC_STRING);
    assert(memcmp(data->data.obj_arr[1]->data.char_arr, "value", 5) == 0);
}

void verify_empty_map(ParsedData* data) {
    assert(data->type == MORLOC_MAP);
    assert(data->size == 0);
}

UnpackTestCase unpack_test_cases[] = {
    {"\xc0", 1, create_nil_schema, verify_nil, "Nil"},
    {"\xc3", 1, create_bool_schema, verify_bool, "Boolean (true)"},
    {"\x2a", 1, create_int_schema, verify_sint, "Signed int (42)"},
    {"\xcd\x01\x2c", 3, create_int_schema, verify_uint, "Unsigned int (300)"},
    {"\xcb\x40\x09\x1e\xb8\x51\xeb\x85\x1f", 9, create_float_schema, verify_float, "Float (3.14)"},
    {"\xa5hello", 6, create_string_schema, verify_string, "String"},
    {"\xc4\x03\x01\x02\x03", 5, create_binary_schema, verify_binary, "Binary"},
    {"\x90", 1, create_empty_array_schema, verify_empty_array, "Empty array"},
    {"\x93\x01\x02\x03", 4, create_array_schema, verify_array, "Array"},
    {"\x80", 1, create_empty_map_schema, verify_empty_map, "Empty map"},
    {"\x82\xa4key1\x2a\xa4key2\xa5value", 18, create_map_schema, verify_map, "Map"}
};

void run_unpack_tests() {

    for (size_t i = 0; i < sizeof(unpack_test_cases) / sizeof(UnpackTestCase); i++) {
        UnpackTestCase tc = unpack_test_cases[i];
        printf("Testing unpacking %s ... ", tc.description);

        Schema* schema = tc.create_schema();
        const char* buf = tc.packed_data;
        size_t buf_remaining = tc.data_size;

        ParsedData* data = unpack_with_schema(&buf, &buf_remaining, schema);
        
        assert(data != NULL);
        assert(buf_remaining == 0);

        tc.verify_data(data);

        free_parsed_data(data);
        free_schema(schema);

        printf("\033[0;32mpassed\033[0m\n", tc.description);
    }
}

int main() {
    run_unpack_tests();
    printf("All unpack tests passed!\n");
    return 0;
}
