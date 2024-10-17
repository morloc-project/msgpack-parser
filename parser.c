#include "mpack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "mlcmpack.h"

#define BUFFER_SIZE 4096

// Function to handle dynamic array allocation and reallocation
char* resize_buffer(char* buffer, size_t current_size, size_t needed_size, size_t* new_size) {

    if (needed_size <= current_size) {
        *new_size = current_size;
        return buffer;
    }

    size_t target_size = current_size;
    while (target_size < needed_size) {
        if (target_size > SIZE_MAX / 2) {
            // Would overflow, so add a fixed amount instead
            target_size += BUFFER_SIZE;
        } else {
            target_size *= 2;
        }
    }

    char* new_buffer = (char*)realloc(buffer, target_size);
    if (new_buffer == NULL) {
        free(buffer);
        return NULL;
    }

    *new_size = target_size;
    return new_buffer;
}

char* pack(size_t* size) {
    *size = 0;
    size_t current_buffer_size = BUFFER_SIZE;
    char* output = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (output == NULL) return NULL;

    char buffer[BUFFER_SIZE];
    char *buf_ptr = buffer;
    size_t buf_remaining = BUFFER_SIZE;
    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);

    mpack_token_t array_token = mpack_pack_array(10000);
    int result = mpack_write(&tokbuf, &buf_ptr, &buf_remaining, &array_token);
    if (result != MPACK_OK) {
        free(output);
        return NULL;
    }

    for (int i = 0; i < 10000; i++) {
        mpack_token_t int_token = mpack_pack_uint(i);
        result = mpack_write(&tokbuf, &buf_ptr, &buf_remaining, &int_token);

        if (result == MPACK_EOF || buf_remaining == 0) {
            size_t written = BUFFER_SIZE - buf_remaining;
            *size += written;

            output = resize_buffer(output, current_buffer_size, *size + BUFFER_SIZE, &current_buffer_size);
            if (output == NULL) return NULL;

            memcpy(output + *size - written, buffer, written);

            buf_ptr = buffer;
            buf_remaining = BUFFER_SIZE;

            if (result == MPACK_EOF) {
                result = mpack_write(&tokbuf, &buf_ptr, &buf_remaining, &int_token);
                if (result != MPACK_OK && result != MPACK_EOF) {
                    free(output);
                    return NULL;
                }
            }
        } else if (result != MPACK_OK) {
            free(output);
            return NULL;
        }
    }

    if (buf_remaining < BUFFER_SIZE) {
        size_t written = BUFFER_SIZE - buf_remaining;
        *size += written;

        output = resize_buffer(output, current_buffer_size, *size, &current_buffer_size);
        if (output == NULL) return NULL;

        memcpy(output + *size - written, buffer, written);
    }

    // Trim the output buffer to the exact size needed
    if (current_buffer_size > *size) {
        char* new_output = (char*)realloc(output, *size);
        if (new_output != NULL) {
            output = new_output;
        }
    }

    return output;
}

void unpack(char* data, size_t data_size) {
    const char *buf_ptr = data;
    size_t buf_remaining = data_size;
    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);

    mpack_token_t token;
    int result;
    int array_size = 0;
    int count = 0;

    while (buf_remaining > 0) {
        result = mpack_read(&tokbuf, &buf_ptr, &buf_remaining, &token);

        if (result == MPACK_OK) {
            switch (token.type) {
                case MPACK_TOKEN_ARRAY:
                    array_size = token.length;
                    printf("Array with %d elements\n", array_size);
                    break;
                case MPACK_TOKEN_UINT:
                    printf("Element %d: %u\n", count++, (unsigned int)mpack_unpack_uint(token));
                    break;
                default:
                    printf("Unexpected token type\n");
                    break;
            }
        } else if (result == MPACK_EOF) {
            // This shouldn't happen as we're reading from a complete buffer
            printf("Unexpected EOF while reading MessagePack data\n");
            break;
        } else {
            printf("Error reading MessagePack data\n");
            break;
        }
    }

    if (count != array_size) {
        printf("Warning: Expected %d elements, but unpacked %d\n", array_size, count);
    }
}


int main() {
    size_t size = 0;
    char* data = pack(&size);
    unpack(data, size);
    return 0;
}



/* int main() {                                                                                         */
/*     // Create a test schema                                                                          */
/*     Schema* schema = create_schema(MORLOC_ARRAY, 3, NULL, NULL);                                     */
/*     schema->parameters = malloc(3 * sizeof(Schema*));                                                */
/*     schema->parameters[0] = create_schema(MORLOC_STRING, 0, NULL, NULL);                             */
/*     schema->parameters[1] = create_schema(MORLOC_TUPLE, 2, NULL, NULL);                              */
/*     schema->parameters[1]->parameters = malloc(2 * sizeof(Schema*));                                 */
/*     schema->parameters[1]->parameters[0] = create_schema(MORLOC_SINT, 0, NULL, NULL);                */
/*     schema->parameters[1]->parameters[1] = create_schema(MORLOC_FLOAT, 0, NULL, NULL);               */
/*     schema->parameters[2] = create_schema(MORLOC_MAP, 1, NULL, NULL);                                */
/*     schema->parameters[2]->parameters = malloc(sizeof(Schema*));                                     */
/*     schema->parameters[2]->parameters[0] = create_schema(MORLOC_UINT, 0, NULL, NULL);                */
/*     schema->parameters[2]->keys = malloc(sizeof(char*));                                             */
/*     schema->parameters[2]->keys[0] = strdup("key");                                                  */
/*                                                                                                      */
/*     // Create test data                                                                              */
/*     ParsedData* data = create_parsed_data(MORLOC_ARRAY);                                             */
/*     data->data.array_val.size = 3;                                                                   */
/*     data->data.array_val.elements = malloc(3 * sizeof(ParsedData));                                  */
/*                                                                                                      */
/*     data->data.array_val.elements[0] = *create_parsed_data(MORLOC_STRING);                           */
/*     data->data.array_val.elements[0].data.bytes_val = strdup("Hello, MessagePack!");                 */
/*                                                                                                      */
/*     data->data.array_val.elements[1] = *create_parsed_data(MORLOC_TUPLE);                            */
/*     data->data.array_val.elements[1].data.array_val.size = 2;                                        */
/*     data->data.array_val.elements[1].data.array_val.elements = malloc(2 * sizeof(ParsedData));       */
/*     data->data.array_val.elements[1].data.array_val.elements[0] = *create_parsed_data(MORLOC_SINT);  */
/*     data->data.array_val.elements[1].data.array_val.elements[0].data.sint_val = -42;                 */
/*     data->data.array_val.elements[1].data.array_val.elements[1] = *create_parsed_data(MORLOC_FLOAT); */
/*     data->data.array_val.elements[1].data.array_val.elements[1].data.double_val = 3.14;              */
/*                                                                                                      */
/*     data->data.array_val.elements[2] = *create_parsed_data(MORLOC_MAP);                              */
/*     data->data.array_val.elements[2].data.map_val.size = 1;                                          */
/*     data->data.array_val.elements[2].data.map_val.keys = malloc(50 * sizeof(char));                  */
/*     strcpy(data->data.array_val.elements[2].data.map_val.keys, "key");                               */
/*     data->data.array_val.elements[2].data.map_val.elements = malloc(sizeof(ParsedData));             */
/*     data->data.array_val.elements[2].data.map_val.elements[0] = *create_parsed_data(MORLOC_UINT);    */
/*     data->data.array_val.elements[2].data.map_val.elements[0].data.uint_val = 100;                   */
/*                                                                                                      */
/*                                                                                                      */
/*     // Create a test schema                                                                          */
/*     Schema* schema = create_schema(MORLOC_SINT, 1, NULL, NULL);                                      */
/*                                                                                                      */
/*     // Create test data                                                                              */
/*     ParsedData* data = create_parsed_data(MORLOC_SINT);                                              */
/*     data->data.sint_val = 42;                                                                        */
/*                                                                                                      */
/*     // Pack the data                                                                                 */
/*     char* packet;                                                                                    */
/*     size_t packet_size;                                                                              */
/*     pack_with_schema(data, schema, &packet, &packet_size);                                           */
/*     if (packet == NULL || packet_size == 0) {                                                        */
/*         printf("Packing failed\n");                                                                  */
/*         // Handle error                                                                              */
/*     }                                                                                                */
/*                                                                                                      */
/*                                                                                                      */
/*     printf("Packed data size: %zu bytes\n", packet_size);                                            */
/*     printf("Packed data: ");                                                                         */
/*     for (size_t i = 0; i < packet_size; i++) {                                                       */
/*         printf("%02x ", (unsigned char)packet[i]);                                                   */
/*     }                                                                                                */
/*     printf("\n\n");                                                                                  */
/*                                                                                                      */
/*     // Unpack the data                                                                               */
/*     ParsedData* unpacked_data = unpack_with_schema(packet, packet_size, schema);                     */
/*     if (unpacked_data == NULL) {                                                                     */
/*         printf("Unpacking failed\n");                                                                */
/*         // Handle error                                                                              */
/*     }                                                                                                */
/*                                                                                                      */
/*     // Print the unpacked data                                                                       */
/*     printf("Unpacked data:\n");                                                                      */
/*     print_parsed_data(unpacked_data);                                                                */
/*                                                                                                      */
/*                                                                                                      */
/*     // Clean up                                                                                      */
/*     free_parsed_data(data);                                                                          */
/*     free_parsed_data(unpacked_data);                                                                 */
/*     free_schema(schema);                                                                             */
/*     free(packet);                                                                                    */
/*                                                                                                      */
/*     return 0;                                                                                        */
/* }                                                                                                    */
