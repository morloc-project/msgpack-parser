#include "mlcmpack.h"

// Helper function to create a schema with parameters
Schema* create_schema_with_params(morloc_serial_type type, int size, Schema** params, char** keys) {
    Schema* schema = malloc(sizeof(Schema));
    if (!schema) return NULL;

    schema->type = type;
    schema->size = size;
    schema->parameters = params;
    schema->keys = keys;

    return schema;
}

Schema* nil_schema() {
    return create_schema_with_params(MORLOC_NIL, 0, NULL, NULL);
}

Schema* bool_schema() {
    return create_schema_with_params(MORLOC_BOOL, 0, NULL, NULL);
}

Schema* sint_schema() {
    return create_schema_with_params(MORLOC_SINT, 0, NULL, NULL);
}

Schema* uint_schema() {
    return create_schema_with_params(MORLOC_UINT, 0, NULL, NULL);
}

Schema* float_schema() {
    return create_schema_with_params(MORLOC_FLOAT, 0, NULL, NULL);
}

Schema* string_schema() {
    return create_schema_with_params(MORLOC_STRING, 0, NULL, NULL);
}

Schema* binary_schema() {
    return create_schema_with_params(MORLOC_BINARY, 0, NULL, NULL);
}

Schema* bool_array_schema() {
    Schema** params = (Schema**)malloc(sizeof(Schema*));
    params[0] = bool_schema();
    return create_schema_with_params(MORLOC_BOOL_ARRAY, 1, params, NULL);
}

Schema* sint_array_schema() {
    Schema** params = (Schema**)malloc(sizeof(Schema*));
    params[0] = sint_schema();
    return create_schema_with_params(MORLOC_SINT_ARRAY, 1, params, NULL);
}

Schema* uint_array_schema() {
    Schema** params = (Schema**)malloc(sizeof(Schema*));
    params[0] = uint_schema();
    return create_schema_with_params(MORLOC_UINT_ARRAY, 1, params, NULL);
}

Schema* float_array_schema() {
    Schema** params = (Schema**)malloc(sizeof(Schema*));
    params[0] = float_schema();
    return create_schema_with_params(MORLOC_FLOAT_ARRAY, 1, params, NULL);
}


// Helper function to create a key-value pair
SchemaKeyValuePair* kvp_schema(const char* key, Schema* value) {
    SchemaKeyValuePair* kvp = malloc(sizeof(SchemaKeyValuePair));
    if (!kvp) return NULL;
    
    // this is a NEW pointer, so need to free this later
    kvp->key = strdup(key);
    if (!kvp->key) {
        free(kvp);
        return NULL;
    }
    
    kvp->value = value;
    return kvp;
}

// Implementation of tuple function
Schema* tuple_schema(size_t size, ...) {
    va_list args;
    va_start(args, size);

    Schema** params = malloc(size * sizeof(Schema*));
    if (!params) {
        va_end(args);
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        params[i] = va_arg(args, Schema*);
    }

    va_end(args);

    return create_schema_with_params(MORLOC_TUPLE, size, params, NULL);
}

// Implementation of array function
Schema* array_schema(Schema* array_type) {
    Schema** params = malloc(sizeof(Schema*));
    if (!params) return NULL;

    params[0] = array_type;

    return create_schema_with_params(MORLOC_ARRAY, 1, params, NULL);
}

// Implementation of map function
// This function eats its keys
Schema* map_schema(size_t size, ...) {
    va_list args;
    va_start(args, size);

    Schema** params = malloc(size * sizeof(Schema*));
    char** keys = malloc(size * sizeof(char*));

    if (!params || !keys) {
        free(params);
        free(keys);
        va_end(args);
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        SchemaKeyValuePair* kvp = va_arg(args, SchemaKeyValuePair*);
        // here I am passing ownership, the schema will free these
        keys[i] = kvp->key;
        params[i] = kvp->value;
        // only free kvp
        free(kvp);
    }

    va_end(args);

    return create_schema_with_params(MORLOC_MAP, size, params, keys);
}

// Free a Schema and its contents
void free_schema(Schema* schema) {
    if (!schema) return;

    if (schema->parameters) {
        for (int i = 0; i < schema->size; i++) {
            free_schema(schema->parameters[i]);
        }
        free(schema->parameters);
    }

    if (schema->keys) {
        for (int i = 0; i < schema->size; i++) {
            if (schema->keys[i]) {
                free(schema->keys[i]);
            }
        }
        free(schema->keys);
    }

    free(schema);
}



// Helper function to create a ParsedData with allocated memory
ParsedData* create_parsed_data(morloc_serial_type type) {
    ParsedData* data = (ParsedData*)malloc(sizeof(ParsedData));
    if (!data) return NULL;
    data->type = type;
    return data;
}

// Helper functions for primitive types
ParsedData* nil_data() {
    ParsedData* data = create_parsed_data(MORLOC_NIL);
    if (data) data->data.nil_val = 0x00;
    return data;
}

ParsedData* bool_data(bool value) {
    ParsedData* data = create_parsed_data(MORLOC_BOOL);
    if (data) data->data.bool_val = value;
    return data;
}

ParsedData* sint_data(int value) {
    ParsedData* data = create_parsed_data(MORLOC_SINT);
    if (data) data->data.sint_val = value;
    return data;
}

ParsedData* uint_data(unsigned int value) {
    ParsedData* data = create_parsed_data(MORLOC_UINT);
    if (data) data->data.uint_val = value;
    return data;
}

ParsedData* float_data(double value) {
    ParsedData* data = create_parsed_data(MORLOC_FLOAT);
    if (data) data->data.double_val = value;
    return data;
}

ParsedData* string_data(const char* value) {
    ParsedData* data = create_parsed_data(MORLOC_STRING);
    if (data) {
        data->data.string_val = strdup(value);
        if (!data->data.string_val) {
            free(data);
            return NULL;
        }
    }
    return data;
}

ParsedData* binary_data(const char* value, size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_BINARY);
    if (data) {
        data->data.binary_val.size = size;
        data->data.binary_val.elements = (char*)malloc(size);
        if (!data->data.binary_val.elements) {
            free(data);
            return NULL;
        }
        memcpy(data->data.binary_val.elements, value, size);
    }
    return data;
}

// Helper function for arrays
ParsedData* array_data(size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_ARRAY);
    if (data) {
        data->data.array_val.size = size;
        data->data.array_val.elements = (ParsedData**)calloc(size, sizeof(ParsedData*));
        if (!data->data.array_val.elements) {
            free(data);
            return NULL;
        }
    }
    return data;
}

// Helper function for tuples
ParsedData* tuple_data(size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_TUPLE);
    if (data) {
        data->data.array_val.size = size;
        data->data.array_val.elements = (ParsedData**)calloc(size, sizeof(ParsedData*));
        if (!data->data.array_val.elements) {
            free(data);
            return NULL;
        }
    }
    return data;
}

// Helper function for maps
ParsedData* map_data(size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_MAP);
    if (data) {
        data->data.map_val.size = size;
        data->data.map_val.keys = (char**)calloc(size, sizeof(char*));
        data->data.map_val.elements = (ParsedData**)calloc(size, sizeof(ParsedData*));
        if (!data->data.map_val.keys || !data->data.map_val.elements) {
            free(data->data.map_val.keys);
            free(data->data.map_val.elements);
            free(data);
            return NULL;
        }
    }
    return data;
}


ParsedData* array_bool_data_(size_t size) {
    ParsedData* data = malloc(sizeof(ParsedData));
    if (!data) return NULL;

    data->type = MORLOC_BOOL_ARRAY;
    data->data.array_bool_val.size = size;
    data->data.array_bool_val.elements = malloc(size * sizeof(bool));
    if (!data->data.array_bool_val.elements) {
        free(data);
        return NULL;
    }

    return data;
}

ParsedData* array_sint_data_(size_t size) {
    ParsedData* data = malloc(sizeof(ParsedData));
    if (!data) return NULL;

    data->type = MORLOC_SINT_ARRAY;
    data->data.array_sint_val.size = size;
    data->data.array_sint_val.elements = malloc(size * sizeof(int));
    if (!data->data.array_sint_val.elements) {
        free(data);
        return NULL;
    }

    return data;
}

ParsedData* array_uint_data_(size_t size) {
    ParsedData* data = malloc(sizeof(ParsedData));
    if (!data) return NULL;

    data->type = MORLOC_UINT_ARRAY;
    data->data.array_uint_val.size = size;
    data->data.array_uint_val.elements = malloc(size * sizeof(unsigned int));
    if (!data->data.array_uint_val.elements) {
        free(data);
        return NULL;
    }

    return data;
}

ParsedData* array_float_data_(size_t size) {
    ParsedData* data = malloc(sizeof(ParsedData));
    if (!data) return NULL;

    data->type = MORLOC_FLOAT_ARRAY;
    data->data.array_float_val.size = size;
    data->data.array_float_val.elements = malloc(size * sizeof(double));
    if (!data->data.array_float_val.elements) {
        free(data);
        return NULL;
    }

    return data;
}

// Helper function for array of booleans
ParsedData* array_bool_data(const bool* values, size_t size) {
    ParsedData* data = array_bool_data_(size);
    if (data) {
        memcpy(data->data.array_bool_val.elements, values, size * sizeof(bool));
    }
    return data;
}

// Helper function for array of signed integers
ParsedData* array_sint_data(const int* values, size_t size) {
    ParsedData* data = array_sint_data_(size);
    if (data) {
        memcpy(data->data.array_sint_val.elements, values, size * sizeof(int));
    }
    return data;
}

// Helper function for array of unsigned integers
ParsedData* array_uint_data(const unsigned int* values, size_t size) {
    ParsedData* data = array_uint_data_(size);
    if (data) {
        memcpy(data->data.array_uint_val.elements, values, size * sizeof(unsigned int));
    }
    return data;
}

// Helper function for array of floats (doubles)
ParsedData* array_float_data(const double* values, size_t size) {
    ParsedData* data = array_float_data_(size);
    if (data) {
        memcpy(data->data.array_float_val.elements, values, size * sizeof(double));
    }
    return data;
}




// Helper function to set a key-value pair in a map
int set_map_element(ParsedData* map, const char* key, ParsedData* value) {
    if (!map || map->type != MORLOC_MAP) {
        return 0;  // Error
    }
    for (size_t i = 0; i < map->data.map_val.size; i++) {
        if (!map->data.map_val.keys[i]) {
            // map copies the key
            map->data.map_val.keys[i] = strdup(key);
            // map takes ownership of value and wil free later
            map->data.map_val.elements[i] = value;
            return 1;  // Success
        }
    }
    return 0;  // Map is full
}

void free_parsed_data(ParsedData* data) {
    if (!data) return;

    switch (data->type) {
        case MORLOC_NIL:
            // No additional freeing needed
            break;
        case MORLOC_BOOL:
            // For a single boolean, no additional freeing needed
            break;
        case MORLOC_SINT:
        case MORLOC_UINT:
        case MORLOC_FLOAT:
            // For single primitives, no additional freeing needed
            break;
        case MORLOC_STRING:
            free(data->data.string_val);
            break;
        case MORLOC_BINARY:
            free(data->data.binary_val.elements);
            break;
        case MORLOC_ARRAY:
            if (data->data.array_val.elements) {
                for (size_t i = 0; i < data->data.array_val.size; i++) {
                    free_parsed_data(data->data.array_val.elements[i]);
                }
                free(data->data.array_val.elements);
            }
            break;
        case MORLOC_BOOL_ARRAY:
            free(data->data.array_bool_val.elements);
            break;
        case MORLOC_SINT_ARRAY:
            free(data->data.array_sint_val.elements);
            break;
        case MORLOC_UINT_ARRAY:
            free(data->data.array_uint_val.elements);
            break;
        case MORLOC_FLOAT_ARRAY:
            free(data->data.array_float_val.elements);
            break;
        case MORLOC_MAP:
            if (data->data.map_val.keys) {
                for (size_t i = 0; i < data->data.map_val.size; i++) {
                    free(data->data.map_val.keys[i]);
                }
                free(data->data.map_val.keys);
            }
            if (data->data.map_val.elements) {
                for (size_t i = 0; i < data->data.map_val.size; i++) {
                    free_parsed_data(data->data.map_val.elements[i]);
                }
                free(data->data.map_val.elements);
            }
            break;
        default:
            // Unknown type, do nothing
            break;
    }

    free(data);
}


#define INITIAL_BUFFER_SIZE 4096

// Helper function to resize the packet buffer
char* resize_buffer(char* buf, size_t* current_size, size_t needed_size) {
    if (needed_size <= *current_size) {
        return buf;
    }
    while (*current_size < needed_size) {
        if (*current_size > SIZE_MAX / 2) {
            *current_size += INITIAL_BUFFER_SIZE;
        } else {
            *current_size *= 2;
        }
    }
    char* new_buf = (char*)realloc(buf, *current_size);
    if (new_buf == NULL) {
        free(buf);
        return NULL;
    }
    return new_buf;
}

// Helper function to write data to the packet
void write_to_packet(char** packet, size_t* packet_size, size_t* current_buffer_size, const char* data, size_t size) {
    *packet = resize_buffer(*packet, current_buffer_size, *packet_size + size);
    if (*packet == NULL) return;
    memcpy(*packet + *packet_size, data, size);
    *packet_size += size;
}

// Forward declaration of pack_data function
int pack_data(
  const ParsedData* data,
  const Schema* schema,
  char** packet,
  size_t* packet_size,
  size_t* current_buffer_size,
  char* buffer,
  char** buf_ptr,
  size_t* buf_remaining,
  mpack_tokbuf_t* tokbuf
) {
    mpack_token_t token;
    int result;

    switch (schema->type) {
        case MORLOC_NIL:
            token = mpack_pack_nil();
            break;
        case MORLOC_BOOL:
            token = mpack_pack_boolean(data->data.bool_val);
            break;
        case MORLOC_SINT:
            token = mpack_pack_sint(data->data.sint_val);
            break;
        case MORLOC_UINT:
            token = mpack_pack_uint(data->data.uint_val);
            break;
        case MORLOC_FLOAT:
            token = mpack_pack_float(data->data.double_val);
            break;
        case MORLOC_STRING:
            token = mpack_pack_str(strlen(data->data.string_val));
            break;
        case MORLOC_BINARY:
            token = mpack_pack_bin(data->data.binary_val.size);
            break;
        case MORLOC_BOOL_ARRAY:
        case MORLOC_SINT_ARRAY:
        case MORLOC_UINT_ARRAY:
        case MORLOC_FLOAT_ARRAY:
        case MORLOC_ARRAY:
            token = mpack_pack_array(data->data.array_val.size);
            break;
        case MORLOC_MAP:
            token = mpack_pack_map(data->data.map_val.size);
            break;
        case MORLOC_TUPLE:
            token = mpack_pack_ext(TUPLE_EXT_TYPE, schema->size);
            break;
    }

    result = mpack_write(tokbuf, buf_ptr, buf_remaining, &token);
    if (result == MPACK_EOF || *buf_remaining == 0) {
        write_to_packet(packet, packet_size, current_buffer_size, buffer, INITIAL_BUFFER_SIZE - *buf_remaining);
        *buf_ptr = buffer;
        *buf_remaining = INITIAL_BUFFER_SIZE;
        if (result == MPACK_EOF) {
            mpack_write(tokbuf, buf_ptr, buf_remaining, &token);
        }
    }

    // Write additional data for strings, binaries, arrays, maps, and tuples
    if (schema->type == MORLOC_STRING) {
        size_t len = strlen(data->data.string_val);
        const char* bytes = data->data.string_val;
        while (len > 0) {
            size_t chunk_size = (len < *buf_remaining) ? len : *buf_remaining;
            memcpy(*buf_ptr, bytes, chunk_size);
            *buf_ptr += chunk_size;
            *buf_remaining -= chunk_size;
            bytes += chunk_size;
            len -= chunk_size;
            if (*buf_remaining == 0) {
                write_to_packet(packet, packet_size, current_buffer_size, buffer, INITIAL_BUFFER_SIZE);
                *buf_ptr = buffer;
                *buf_remaining = INITIAL_BUFFER_SIZE;
            }
        }
    } else if (schema->type == MORLOC_BINARY) {
        size_t len = data->data.binary_val.size;
        const char* bytes = data->data.binary_val.elements;
        while (len > 0) {
            size_t chunk_size = (len < *buf_remaining) ? len : *buf_remaining;
            memcpy(*buf_ptr, bytes, chunk_size);
            *buf_ptr += chunk_size;
            *buf_remaining -= chunk_size;
            bytes += chunk_size;
            len -= chunk_size;
            if (*buf_remaining == 0) {
                write_to_packet(packet, packet_size, current_buffer_size, buffer, INITIAL_BUFFER_SIZE);
                *buf_ptr = buffer;
                *buf_remaining = INITIAL_BUFFER_SIZE;
            }
        }
    } else if ( schema->type == MORLOC_ARRAY) {
        for (size_t i = 0; i < data->data.array_val.size; i++) {
            pack_data(
              data->data.array_val.elements[i],
              schema->parameters[0],
              packet,
              packet_size,
              current_buffer_size,
              buffer,
              buf_ptr,
              buf_remaining,
              tokbuf
            );
        }
    } else if ( schema->type == MORLOC_BOOL_ARRAY ||
                schema->type == MORLOC_SINT_ARRAY ||
                schema->type == MORLOC_UINT_ARRAY ||
                schema->type == MORLOC_FLOAT_ARRAY ) {

        mpack_token_t token;

        size_t size;

        switch(schema->type){
          case MORLOC_BOOL_ARRAY:
            size = data->data.array_bool_val.size;
            break;
          case MORLOC_SINT_ARRAY:
            size = data->data.array_sint_val.size;
            break;
          case MORLOC_UINT_ARRAY:
            size = data->data.array_uint_val.size;
            break;
          case MORLOC_FLOAT_ARRAY:
            size = data->data.array_float_val.size;
            break;
          default:
            break;
        }

        for (size_t i = 0; i < size; i++){

            switch(schema->type){
              case MORLOC_BOOL_ARRAY:
                token = mpack_pack_boolean(data->data.array_bool_val.elements[i]);
                break;
              case MORLOC_SINT_ARRAY:
                token = mpack_pack_sint(data->data.array_sint_val.elements[i]);
                break;
              case MORLOC_UINT_ARRAY:
                token = mpack_pack_uint(data->data.array_uint_val.elements[i]);
                break;
              case MORLOC_FLOAT_ARRAY:
                token = mpack_pack_float(data->data.array_float_val.elements[i]);
                break;
              default:
                break;
            }

            result = mpack_write(tokbuf, buf_ptr, buf_remaining, &token);
            if (result == MPACK_EOF || *buf_remaining == 0) {
                write_to_packet(packet, packet_size, current_buffer_size, buffer, INITIAL_BUFFER_SIZE - *buf_remaining);
                *buf_ptr = buffer;
                *buf_remaining = INITIAL_BUFFER_SIZE;
                if (result == MPACK_EOF) {
                    mpack_write(tokbuf, buf_ptr, buf_remaining, &token);
                }
            }
        }
    } else if (schema->type == MORLOC_TUPLE) {
        for (size_t i = 0; i < schema->size; i++) {
            pack_data(
              data->data.array_val.elements[i],
              schema->parameters[i],
              packet,
              packet_size,
              current_buffer_size,
              buffer,
              buf_ptr,
              buf_remaining,
              tokbuf
            );
        }
    } else if (schema->type == MORLOC_MAP) {
        for (size_t i = 0; i < data->data.map_val.size; i++) {
            // Pack key
            token = mpack_pack_str(strlen(data->data.map_val.keys[i]));
            mpack_write(tokbuf, buf_ptr, buf_remaining, &token);
            const char* key = data->data.map_val.keys[i];
            size_t key_len = strlen(key);
            while (key_len > 0) {
                size_t chunk_size = (key_len < *buf_remaining) ? key_len : *buf_remaining;
                memcpy(*buf_ptr, key, chunk_size);
                *buf_ptr += chunk_size;
                *buf_remaining -= chunk_size;
                key += chunk_size;
                key_len -= chunk_size;
                if (*buf_remaining == 0) {
                    write_to_packet(packet, packet_size, current_buffer_size, buffer, INITIAL_BUFFER_SIZE);
                    *buf_ptr = buffer;
                    *buf_remaining = INITIAL_BUFFER_SIZE;
                }
            }
            // Pack value
            pack_data(data->data.map_val.elements[i], schema->parameters[0], packet, packet_size, current_buffer_size, buffer, buf_ptr, buf_remaining, tokbuf);
        }
    }

    return 0;
}

int pack_with_schema(const ParsedData* data, const Schema* schema, char** packet, size_t* packet_size) {
    *packet_size = 0;
    size_t current_buffer_size = INITIAL_BUFFER_SIZE;
    *packet = (char*)malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    if (*packet == NULL) return 1;

    char buffer[INITIAL_BUFFER_SIZE];
    char *buf_ptr = buffer;
    size_t buf_remaining = INITIAL_BUFFER_SIZE;
    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);

    int pack_result = pack_data(data, schema, packet, packet_size, &current_buffer_size, buffer, &buf_ptr, &buf_remaining, &tokbuf);
    if (pack_result != 0){
      return pack_result;
    }

    // Write any remaining data
    if (buf_remaining < INITIAL_BUFFER_SIZE) {
        write_to_packet(packet, packet_size, &current_buffer_size, buffer, INITIAL_BUFFER_SIZE - buf_remaining);
    }

    // Trim the output buffer to the exact size needed
    if (current_buffer_size > *packet_size) {
        char* new_packet = (char*)realloc(*packet, *packet_size);
        if (new_packet != NULL) {
            *packet = new_packet;
        }
    }

    return 0;
}


ParsedData* unpack_with_schema(const char** buf_ptr, size_t* buf_remaining, const Schema* schema) {
    if (!buf_ptr || !buf_remaining || !schema || *buf_remaining == 0) return NULL;

    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);
    mpack_token_t token;

    int read_result = mpack_read(&tokbuf, buf_ptr, buf_remaining, &token);
    if (read_result != MPACK_OK) {
        return NULL;
    }

    ParsedData* result = NULL;

    switch (schema->type) {
        case MORLOC_NIL:
            if (token.type != MPACK_TOKEN_NIL) return NULL;
            result = nil_data();
            break;
        case MORLOC_BOOL:
            if (token.type != MPACK_TOKEN_BOOLEAN) return NULL;
            result = bool_data(mpack_unpack_boolean(token));
            break;
        case MORLOC_SINT:
            // mpack won't always get this right since there is overlap
            if (!(token.type == MPACK_TOKEN_SINT || token.type == MPACK_TOKEN_UINT)) return NULL;
            result = sint_data(mpack_unpack_number(token));
            break;
        case MORLOC_UINT:
            if (!(token.type == MPACK_TOKEN_SINT || token.type == MPACK_TOKEN_UINT)) return NULL;
            result = uint_data(mpack_unpack_number(token));
            break;
        case MORLOC_FLOAT:
            if (token.type != MPACK_TOKEN_FLOAT) return NULL;
            result = float_data(mpack_unpack_float(token));
            break;
        case MORLOC_STRING:
            if (token.type != MPACK_TOKEN_STR) return NULL;
            // here I use binary_data since MessagePack strings
            // are not null terminated. The string_data fucntion
            // is appropriate when passing a C string in construction.
            result = binary_data(*buf_ptr, token.length);
            *buf_ptr += token.length;
            *buf_remaining -= token.length;
            break;
        case MORLOC_BINARY:
            if (token.type != MPACK_TOKEN_BIN) return NULL;
            result = binary_data(*buf_ptr, token.length);
            *buf_ptr += token.length;
            *buf_remaining -= token.length;
            break;
        case MORLOC_MAP:
            if (token.type != MPACK_TOKEN_MAP) return NULL;
            result = map_data(token.length);
            if (!result) return NULL;
            for (size_t i = 0; i < token.length; i++) {
                // Read key
                mpack_read(&tokbuf, buf_ptr, buf_remaining, &token);
                if (token.type != MPACK_TOKEN_STR) {
                    free_parsed_data(result);
                    return NULL;
                }
                char* key = malloc(token.length + 1);
                if (!key) {
                    free_parsed_data(result);
                    return NULL;
                }
                memcpy(key, buf_ptr, token.length);
                key[token.length] = '\0';
                *buf_ptr += token.length;
                *buf_remaining -= token.length;

                // Read value
                ParsedData* value = unpack_with_schema(buf_ptr, buf_remaining, schema->parameters[0]);
                if (!value) {
                    free(key);
                    free_parsed_data(result);
                    return NULL;
                }
                if (!set_map_element(result, key, value)) {
                    free(key);
                    free_parsed_data(value);
                    free_parsed_data(result);
                    return NULL;
                }
                free(key);
            }
            break;
        case MORLOC_ARRAY:
            if (token.type != MPACK_TOKEN_ARRAY) return NULL;
            result = array_data(token.length);
            if (!result) return NULL;
            for (size_t i = 0; i < token.length; i++) {
                ParsedData* element = unpack_with_schema(buf_ptr, buf_remaining, schema->parameters[0]);
                if (!element) {
                    free_parsed_data(result);
                    return NULL;
                }
                result->data.array_val.elements[i] = element;
            }
            break;
        case MORLOC_TUPLE:
            if (token.type != MPACK_TOKEN_ARRAY && token.type != MPACK_TOKEN_EXT) return NULL;
            result = tuple_data(schema->size);
            if (!result) return NULL;
            for (size_t i = 0; i < schema->size; i++) {
                ParsedData* element = unpack_with_schema(buf_ptr, buf_remaining, schema->parameters[i]);
                if (!element) {
                    free_parsed_data(result);
                    return NULL;
                }
                result->data.array_val.elements[i] = element;
            }
            break;
        case MORLOC_BOOL_ARRAY:
            if (token.type != MPACK_TOKEN_ARRAY) return NULL;
            result = array_bool_data_(token.length);
            if (!result) return NULL;
            for (size_t i = 0; i < token.length; i++) {
                mpack_read(&tokbuf, buf_ptr, buf_remaining, &token);
                result->data.array_bool_val.elements[i] = mpack_unpack_boolean(token);
            }
            break;
        case MORLOC_SINT_ARRAY:
            if (token.type != MPACK_TOKEN_ARRAY) return NULL;
            result = array_sint_data_(token.length);
            if (!result) return NULL;
            for (size_t i = 0; i < token.length; i++) {
                mpack_read(&tokbuf, buf_ptr, buf_remaining, &token);
                result->data.array_sint_val.elements[i] = mpack_unpack_sint(token);
            }
            break;
        case MORLOC_UINT_ARRAY:
            if (token.type != MPACK_TOKEN_ARRAY) return NULL;
            result = array_uint_data_(token.length);
            if (!result) return NULL;
            for (size_t i = 0; i < token.length; i++) {
                mpack_read(&tokbuf, buf_ptr, buf_remaining, &token);
                result->data.array_uint_val.elements[i] = mpack_unpack_uint(token);
            }
            break;
        case MORLOC_FLOAT_ARRAY:
            if (token.type != MPACK_TOKEN_ARRAY) return NULL;
            result = array_float_data_(token.length);
            if (!result) return NULL;
            for (size_t i = 0; i < token.length; i++) {
                mpack_read(&tokbuf, buf_ptr, buf_remaining, &token);
                result->data.array_float_val.elements[i] = mpack_unpack_float(token);
            }
            break;
        default:
            return NULL;
    }

    return result;
}
