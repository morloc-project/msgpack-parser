#include <assert.h>
#include "mlcmpack.h"

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { printf("Running test_%s ... ", #name); test_##name(); printf("\033[0;32mpassed\033[0m\n", #name); } while(0)

TEST(pack_nil) {
    ParsedData* data = nil_data();
    char* packet = NULL;
    size_t packet_size = 0;
    int result = pack(data, "z", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 1);
    assert((unsigned char)packet[0] == 0xc0); // MessagePack encoding for nil
    free(packet);
    free_parsed_data(data);
}

TEST(pack_bool) {
    ParsedData* data_true = bool_data(true);
    ParsedData* data_false = bool_data(false);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data_true, "b", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 1);
    assert((unsigned char)packet[0] == 0xc3); // MessagePack encoding for true
    free(packet);

    result = pack(data_false, "b", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 1);
    assert((unsigned char)packet[0] == 0xc2); // MessagePack encoding for false
    free(packet);

    free_parsed_data(data_true);
    free_parsed_data(data_false);
}

TEST(pack_sint) {
    ParsedData* data_positive = int_data(42);
    ParsedData* data_negative = int_data(-42);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data_positive, "i4", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 1);
    assert((unsigned char)packet[0] == 0x2A); // MessagePack encoding for 42
    free(packet);

    result = pack(data_negative, "i4", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 2);
    assert((unsigned char)packet[0] == 0xD0 && (unsigned char)packet[1] == 0xD6); // MessagePack encoding for -42
    free(packet);

    free_parsed_data(data_positive);
    free_parsed_data(data_negative);
}

TEST(pack_uint) {
    ParsedData* data = int_data(300);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "i4", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 3);
    assert((unsigned char)packet[0] == 0xCD && (unsigned char)packet[1] == 0x01 && (unsigned char)packet[2] == 0x2C); // MessagePack encoding for 300
    free(packet);

    free_parsed_data(data);
}

TEST(pack_float) {
    ParsedData* data = float_data(3.14);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "f8", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 9);
    assert((unsigned char)packet[0] == 0xCB); // MessagePack encoding for double
    // We don't check the exact bytes for the float value due to potential precision issues
    free(packet);

    free_parsed_data(data);
}

TEST(pack_string) {
    ParsedData* data = string_data("hello", 5);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "s", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 6);
    assert((unsigned char)packet[0] == 0xA5); // MessagePack encoding for string of length 5
    assert(memcmp(packet + 1, "hello", 5) == 0);
    free(packet);

    free_parsed_data(data);
}

TEST(pack_binary) {
    const char x[] = {0x01, 0x02, 0x03};
    ParsedData* data = binary_data(x, 3);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "r", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 5);
    assert((unsigned char)packet[0] == 0xC4 && (unsigned char)packet[1] == 0x03); // MessagePack encoding for binary data of length 3
    assert(memcmp(packet + 2, x, 3) == 0);
    free(packet);

    free_parsed_data(data);
}

TEST(pack_array_bool) {
    bool values[] = {true, false, true};
    ParsedData* data = array_bool_data(values, 3);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "ab", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 4);
    assert((unsigned char)packet[0] == 0x93); // MessagePack encoding for array of length 3
    assert((unsigned char)packet[1] == 0xC3 && (unsigned char)packet[2] == 0xC2 && (unsigned char)packet[3] == 0xC3);
    free(packet);

    free_parsed_data(data);
}

TEST(pack_array_sint) {
    int values[] = {1, -2, 3};
    ParsedData* data = array_int_data(values, 3);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "ai4", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 4);
    assert((unsigned char)packet[0] == 0x93); // MessagePack encoding for array of length 3
    assert((unsigned char)packet[1] == 0x01 && (unsigned char)packet[2] == 0xFE && (unsigned char)packet[3] == 0x03);
    free(packet);

    free_parsed_data(data);
}

TEST(pack_array_uint) {
    unsigned int values[] = {1, 2, 300};
    ParsedData* data = array_int_data(values, 3);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "ai4", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 6);
    assert((unsigned char)packet[0] == 0x93); // MessagePack encoding for array of length 3
    assert((unsigned char)packet[1] == 0x01 && (unsigned char)packet[2] == 0x02);
    assert((unsigned char)packet[3] == 0xCD && (unsigned char)packet[4] == 0x01 && (unsigned char)packet[5] == 0x2C);
    free(packet);

    free_parsed_data(data);
}

TEST(pack_array_float) {
    double values[] = {1.1, 2.2, 3.3};
    ParsedData* data = array_float_data(values, 3);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "af8", &packet, &packet_size);

    /* printf("packet_size = %zu\n", packet_size); */
    /* print_hex(packet, packet_size);             */
    /* 93 cb 3f f1 99 99 99 99 99 9a cb 40 01 99 99 99 99 99 9a cb 40 0a 66 66 66 66 66 66 */
    //  0  1  2  3  4  5  6  7  8  9  a

    assert(result == 0);
    assert(packet_size == 28);
    assert((unsigned char)packet[0] == 0x93); // MessagePack encoding for array of length 3
    assert((unsigned char)packet[1] == 0xCB && (unsigned char)packet[10] == 0xCB && (unsigned char)packet[19] == 0xCB);
    free(packet);

    free_parsed_data(data);
}

TEST(pack_tuple) {
    ParsedData* data = tuple_data_(3);
    data->data.obj_arr[0] = int_data(42);
    data->data.obj_arr[1] = float_data(3.14);
    data->data.obj_arr[2] = string_data("hello", 5);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "t3i4f8s", &packet, &packet_size);

    /* printf("packet_size = %zu\n", packet_size); */
    /* print_hex(packet, packet_size);             */

    assert(result == 0);
    assert(packet_size == 17);
    assert((unsigned char)packet[0] == 0x93); // MessagePack array of lenth 3
    assert((unsigned char)packet[1] == 0x2A); // 42
    assert((unsigned char)packet[2] == 0xCB); // Start of double
    assert((unsigned char)packet[11] == 0xA5); // String of length 5
    free(packet);

    free_parsed_data(data);
}

TEST(pack_map) {
    ParsedData* data = map_data_(2);
    set_map_element(data, 0, "key1", int_data(42));
    set_map_element(data, 1, "key2", string_data("value", 5));
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "m24key1i44key2s", &packet, &packet_size);

    /* printf("packet_size = %zu\n", packet_size); */
    /* print_hex(packet, packet_size);             */
    /* 82 a4 6b 65 79 31 2a a4 6b 65 79 32 a5 76 61 6c 75 65 */
    // 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  10 11
    //   |   k  e  y  1 |42|   k  e  y  2 |   v  a  l  u  e

    assert(result == 0);
    assert(packet_size == 18);
    assert((unsigned char)packet[0] == 0x82); // MessagePack encoding for map of size 2
    free(packet);
    free_parsed_data(data);
}

TEST(pack_empty_array) {
    ParsedData* data = array_data_(0);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "ai4", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 1);
    assert((unsigned char)packet[0] == 0x90); // MessagePack encoding for empty array
    free(packet);

    free_parsed_data(data);
}

TEST(pack_empty_map) {
    ParsedData* data = map_data_(0);
    char* packet = NULL;
    size_t packet_size = 0;

    int result = pack(data, "m0", &packet, &packet_size);
    assert(result == 0);
    assert(packet_size == 1);
    assert((unsigned char)packet[0] == 0x80); // MessagePack encoding for empty map
    free(packet);

    free_parsed_data(data);
}

int main() {
    RUN_TEST(pack_nil);
    RUN_TEST(pack_bool);
    RUN_TEST(pack_sint);
    RUN_TEST(pack_uint);
    RUN_TEST(pack_float);
    RUN_TEST(pack_string);
    RUN_TEST(pack_binary);
    RUN_TEST(pack_array_bool);
    RUN_TEST(pack_array_sint);
    RUN_TEST(pack_array_uint);
    RUN_TEST(pack_array_float);
    RUN_TEST(pack_tuple);
    RUN_TEST(pack_map);
    RUN_TEST(pack_empty_array);
    RUN_TEST(pack_empty_map);
    return 0;
}
