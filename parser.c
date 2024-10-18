#include "mlcmpack.h"

int main() {
    // Create a test schema
    Schema* big_schema = array_schema(
        tuple_schema(
            3,
            sint_array_schema(),
            tuple_schema(
                2,
                sint_schema(),
                float_schema()
            ),
            map_schema(
                2,
                kvp_schema("a", uint_schema()),
                kvp_schema("b", bool_schema())
            )
        )
    );

    // Create test data
    ParsedData* test_data = array_data(2);
    for (int i = 0; i < 2; i++) {
        ParsedData* tuple = tuple_data(3);

        // Sint array
        int sint_arr[] = {1, -2, 3};
        ParsedData* sint_array = array_sint_data(sint_arr, 3);
        tuple->data.array_val.elements[0] = sint_array;

        // Inner tuple (sint, float)
        ParsedData* inner_tuple = tuple_data(2);
        inner_tuple->data.array_val.elements[0] = sint_data(42);
        inner_tuple->data.array_val.elements[1] = float_data(3.14);
        tuple->data.array_val.elements[1] = inner_tuple;

        // Map
        ParsedData* map = map_data(2);
        set_map_element(map, "ap", uint_data(123));
        set_map_element(map, "c", bool_data(true));
        tuple->data.array_val.elements[2] = map;

// 92 93
//       93 01 fe 03
//       92 2a cb 40 09 1e b8 51 eb 85 1f
//       82 a2 61 70 7b
//          a1 63 c3
//    93 93 01 fe 03 92 2a cb 40 09 1e b8 51 eb 85 1f 82 a2 61 70 7b a1 63 c3


        test_data->data.array_val.elements[i] = tuple;
    }

    // Pack the data
    char* packed_data;
    size_t packed_size;
    int pack_result = pack_with_schema(test_data, big_schema, &packed_data, &packed_size);
    if (pack_result != 0) {
        printf("Packing failed with error code: %d\n", pack_result);
        free_parsed_data(test_data);
        free_schema(big_schema);
        return 1;
    }

    printf("Packed data size: %zu bytes\n", packed_size);
    print_hex(packed_data, packed_size);

    // 92 93 93 01 fe 03 92 2a cb 40 09 1e b8 51 eb 85 1f 82 a2 61 70 7b a1 63 c3
    //    93 93 01 fe 03 92 2a cb 40 09 1e b8 51 eb 85 1f 82 a2 61 70 7b a1 63 c3
    //                                                      |     k  |v |  k  | v


    // 92 93 93 01 fe 03 92 2a cb 40 09 1e b8 51 eb 85 1f 82 a1 61 7b a1 62 c3
    //    93 93 01 fe 03 92 2a cb 40 09 1e b8 51 eb 85 1f 82 a1 61 7b a1 62 c3

    // Unpack the data
    const char* unpack_ptr = packed_data;
    size_t remaining = packed_size;
    ParsedData* unpacked_data = unpack_with_schema(&unpack_ptr, &remaining, big_schema);
    print_parsed_data(unpacked_data, 0);

    if (!unpacked_data) {
        printf("Unpacking failed\n");
        free(packed_data);
        free_parsed_data(test_data);
        free_schema(big_schema);
        return 1;
    }

    // Clean up
    free(packed_data);
    free_parsed_data(test_data);
    free_parsed_data(unpacked_data);
    free_schema(big_schema);

    return 0;
}
