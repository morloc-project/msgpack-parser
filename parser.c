#include "mlcmpack.h"

int main() {

    Schema* big_schema = int_array_schema();

    size_t N = 1000000;
    ParsedData* test_data = array_int_data_(N);
    for(size_t i = 0; i < N; i++){
      test_data->data.int_arr[i] = (int)i;
    }

    /* // Create a test schema                                 */
    /* Schema* big_schema = array_schema(                      */
    /*     tuple_schema(                                       */
    /*         3,                                              */
    /*         int_array_schema(),                             */
    /*         tuple_schema(                                   */
    /*             2,                                          */
    /*             int_schema(),                               */
    /*             float_schema()                              */
    /*         ),                                              */
    /*         map_schema(                                     */
    /*             2,                                          */
    /*             kvp_schema("a", int_schema()),              */
    /*             kvp_schema("b", bool_schema())              */
    /*         )                                               */
    /*     )                                                   */
    /* );                                                      */
    /*                                                         */
    /* // Create test data                                     */
    /* ParsedData* test_data = array_data_(2);                 */
    /* for (int i = 0; i < 2; i++) {                           */
    /*     ParsedData* tuple = tuple_data_(3);                 */
    /*                                                         */
    /*     // Sint array                                       */
    /*     int int_arr[] = {1, -2, 3};                         */
    /*     ParsedData* int_array = array_int_data(int_arr, 3); */
    /*     tuple->data.obj_arr[0] = int_array;                 */
    /*                                                         */
    /*     // Inner tuple (int, float)                         */
    /*     ParsedData* inner_tuple = tuple_data_(2);           */
    /*     inner_tuple->data.obj_arr[0] = int_data(42);        */
    /*     inner_tuple->data.obj_arr[1] = float_data(3.14);    */
    /*     tuple->data.obj_arr[1] = inner_tuple;               */
    /*                                                         */
    /*     // Map                                              */
    /*     ParsedData* map = map_data_(2);                     */
    /*     set_map_element(map, 0, "yolo", int_data(123));     */
    /*     set_map_element(map, 1, "c", bool_data(true));      */
    /*     tuple->data.obj_arr[2] = map;                       */
    /*                                                         */
    /*     test_data->data.obj_arr[i] = tuple;                 */
    /* }                                                       */

    // Pack the data
    char* packed_data;
    size_t packed_size;
    int pack_result = pack(test_data, big_schema, &packed_data, &packed_size);
    if (pack_result != 0) {
        printf("Packing failed with error code: %d\n", pack_result);
        free_parsed_data(test_data);
        free_schema(big_schema);
        return 1;
    }

    size_t buf_remaining = packed_size;
    const char* buf_ptr = packed_data;

    /* printf("Packed data size: %zu bytes\n", packed_size);                                */
    /* print_hex(packed_data, packed_size);                                                 */
    /*                                                                                      */
    /* // 92 93 93 01 fe 03 92 2a cb 40 09 1e b8 51 eb 85 1f 82 a2 61 70 7b a1 63 c3        */
    /* //    93 93 01 fe 03 92 2a cb 40 09 1e b8 51 eb 85 1f 82 a2 61 70 7b a1 63 c3        */
    /* //                                                      |     k  |v |  k  | v        */

    // Unpack the data
    const char* unpack_ptr = packed_data;
    size_t remaining = packed_size;
    ParsedData* unpacked_data = unpack_with_schema(&unpack_ptr, &remaining, big_schema);

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

    /* Schema* big_schema = binary_schema();    */
    /*                                          */
    /* size_t N = 4104;                         */
    /* ParsedData* test_data = binary_data_(N); */
    /* for(size_t i = 0; i < N; i++){           */
    /*   test_data->data.char_arr[i] = 'x';     */
    /* }                                        */
