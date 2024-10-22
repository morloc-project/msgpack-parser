#include "mlcmpack.h"

// utility ####

void print_hex(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02x", (unsigned char)data[i]);
        if (i < size - 1) {
            printf(" ");
        }
    }
}

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

Schema* int_schema() {
    return create_schema_with_params(MORLOC_INT, 0, NULL, NULL);
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

Schema* int_array_schema() {
    Schema** params = (Schema**)malloc(sizeof(Schema*));
    params[0] = int_schema();
    return create_schema_with_params(MORLOC_INT_ARRAY, 1, params, NULL);
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

Schema* array_schema(Schema* array_type) {
    switch(array_type->type){
      case MORLOC_BOOL:
        return bool_array_schema();
      case MORLOC_INT:
        return int_array_schema();
      case MORLOC_FLOAT:
        return float_array_schema();
      default:
        Schema** params = malloc(sizeof(Schema*));
        if (!params) return NULL;
        
        params[0] = array_type;
        
        return create_schema_with_params(MORLOC_ARRAY, 1, params, NULL);
    }
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
            if(schema->parameters[i]){
              free_schema(schema->parameters[i]);
            }
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
ParsedData* create_parsed_data(morloc_serial_type type, size_t size) {
    ParsedData* data = (ParsedData*)malloc(sizeof(ParsedData));
    if (!data) return NULL;
    data->type = type;
    data->size = size;
    data->key = NULL;
    return data;
}

// Helper functions for primitive types
ParsedData* nil_data() {
    ParsedData* data = create_parsed_data(MORLOC_NIL, 0);
    // I won't actually use this value, so I keep the msgpck value
    if (data) data->data.nil_val = 0xc0;
    return data;
}

ParsedData* bool_data(bool value) {
    ParsedData* data = create_parsed_data(MORLOC_BOOL, 0);
    if (data) data->data.bool_val = value;
    return data;
}

ParsedData* int_data(int value) {
    ParsedData* data = create_parsed_data(MORLOC_INT, 0);
    if (data) data->data.int_val = value;
    return data;
}

ParsedData* float_data(double value) {
    ParsedData* data = create_parsed_data(MORLOC_FLOAT, 0);
    if (data) data->data.double_val = value;
    return data;
}


ParsedData* char_data_(size_t size, morloc_serial_type mtype) {
    ParsedData* data = create_parsed_data(mtype, size);
    if (data) {
        data->data.char_arr = (char*)calloc(size, sizeof(char));
        if (!data->data.char_arr) {
            free(data);
            return NULL;
        }
    }
    return data;
}

ParsedData* string_data_(size_t size) {
    return char_data_(size, MORLOC_STRING);
}

ParsedData* binary_data_(size_t size) {
    return char_data_(size, MORLOC_BINARY);
}

ParsedData* ext_data_(size_t size, int ext_type) {
    // the ext_type is ignored since no support is currently offered
    return char_data_(size, MORLOC_EXT);
}


ParsedData* obj_array_data_(size_t size, morloc_serial_type mtype) {
    ParsedData* data = create_parsed_data(mtype, size);
    if (data) {
        data->data.obj_arr = (ParsedData**)calloc(size, sizeof(ParsedData*));
        if (!data->data.obj_arr) {
            free(data);
            return NULL;
        }
    }
    return data;
}

// Helper function for arrays
ParsedData* array_data_(size_t size) {
    return obj_array_data_(size, MORLOC_ARRAY);
}

// Helper function for tuples
ParsedData* tuple_data_(size_t size) {
    return obj_array_data_(size, MORLOC_TUPLE);
}

// Helper function for maps
ParsedData* map_data_(size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_MAP, size);
    if (data) {
        data->data.obj_arr = (ParsedData**)calloc(size, sizeof(ParsedData*));
        if (!data->data.obj_arr) {
            free(data->data.obj_arr);
            free(data);
            return NULL;
        }
    }
    return data;
}

// Helper function to set a key-value pair in a map
void set_map_element(ParsedData* map, size_t pos, const char* key, ParsedData* value) {
    value->key = strdup(key);
    map->data.obj_arr[pos] = value;
}

ParsedData* array_bool_data_(size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_BOOL_ARRAY, size);
    data->data.bool_arr = malloc(size * sizeof(bool));
    if (!data->data.bool_arr) {
        free(data);
        return NULL;
    }
    return data;
}

ParsedData* array_int_data_(size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_INT_ARRAY, size);
    data->data.int_arr = malloc(size * sizeof(int));
    if (!data->data.int_arr) {
        free(data);
        return NULL;
    }

    return data;
}

ParsedData* array_float_data_(size_t size) {
    ParsedData* data = create_parsed_data(MORLOC_FLOAT_ARRAY, size);
    data->data.float_arr = malloc(size * sizeof(double));
    if (!data->data.float_arr) {
        free(data);
        return NULL;
    }

    return data;
}

// Helper function for array of booleans
ParsedData* array_bool_data(const bool* values, size_t size) {
    ParsedData* data = array_bool_data_(size);
    if (data) {
        memcpy(data->data.bool_arr, values, size * sizeof(bool));
    }
    return data;
}

// Helper function for array of signed integers
ParsedData* array_int_data(const int* values, size_t size) {
    ParsedData* data = array_int_data_(size);
    if (data) {
        memcpy(data->data.int_arr, values, size * sizeof(int));
    }
    return data;
}

// Helper function for array of floats (doubles)
ParsedData* array_float_data(const double* values, size_t size) {
    ParsedData* data = array_float_data_(size);
    if (data) {
        memcpy(data->data.float_arr, values, size * sizeof(double));
    }
    return data;
}

ParsedData* string_data(const char* value, size_t size) {
    ParsedData* data = string_data_(size);
    if (data) {
        memcpy(data->data.char_arr, value, size);
    }
    return data;
}

ParsedData* binary_data(const char* value, size_t size) {
    ParsedData* data = binary_data_(size);
    if (data) {
        memcpy(data->data.char_arr, value, size);
    }
    return data;
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
        case MORLOC_INT:
        case MORLOC_FLOAT:
            // For single primitives, no additional freeing needed
            break;
        case MORLOC_BINARY:
        case MORLOC_STRING:
            free(data->data.char_arr);
            break;
        case MORLOC_MAP:
        case MORLOC_ARRAY:
            if (data->data.obj_arr) {
                for (size_t i = 0; i < data->size; i++) {
                    free_parsed_data(data->data.obj_arr[i]);
                }
                free(data->data.obj_arr);
            }
            break;
        case MORLOC_BOOL_ARRAY:
            free(data->data.bool_arr);
            break;
        case MORLOC_INT_ARRAY:
            free(data->data.int_arr);
            break;
        case MORLOC_FLOAT_ARRAY:
            free(data->data.float_arr);
            break;
        default:
            // Unknown type, do nothing
            break;
    }

    free(data);
}

// packing ####

void upsize(
  char** data,            // data that will be resized
  char** data_ptr,        // pointer that will be updated to preserve offset
  size_t* remaining_size, // remaining data size
  size_t added_size       // the number of bytes that need to be added
){
    // check if any action is needed
    if (added_size <= *remaining_size) {
        return;
    }

    size_t used_size = *data_ptr - *data;
    size_t buffer_size = used_size + *remaining_size;

    // find an appropriate size for the new data
    while (added_size < *remaining_size) {
        if (buffer_size > SIZE_MAX / 2) {
            buffer_size += BUFFER_SIZE;
        } else {
            buffer_size *= 2;
        }
        *remaining_size = buffer_size - used_size;
    }

    // allocate memory for the new data
    *data = (char*)realloc(*data, buffer_size);

    // point old pointer to the same offset in the new data
    *data_ptr = *data + used_size;
}

void write_to_packet(
  void* src,                // source data
  char** packet,            // destination
  char** packet_ptr,        // location in the destination that will be written to
  size_t* packet_remaining, // remaining data size
  size_t size               // the number of bytes to write

){
    upsize(packet, packet_ptr, packet_remaining, size);
    memcpy(*packet_ptr, src, size);
    *packet_ptr += size;
    *packet_remaining -= size;
}

//  The main function for writing MessagePack
int pack_data(
  const ParsedData* data,    // input data structure
  const Schema* schema,      // input data schema
  char** packet,             // a pointer to the messagepack data
  char** packet_ptr,         // the current position in the buffer
  size_t* packet_remaining,  // bytes from current position to the packet end
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
        case MORLOC_INT:
            token = mpack_pack_sint(data->data.int_val);
            break;
        case MORLOC_FLOAT:
            token = mpack_pack_float(data->data.double_val);
            break;
        case MORLOC_STRING:
            token = mpack_pack_str(data->size);
            break;
        case MORLOC_BINARY:
            token = mpack_pack_bin(data->size);
            break;
        case MORLOC_BOOL_ARRAY:
        case MORLOC_INT_ARRAY:
        case MORLOC_FLOAT_ARRAY:
        case MORLOC_ARRAY:
        case MORLOC_TUPLE:
            // these all start with the same array bytes
            token = mpack_pack_array(data->size);
            break;
        case MORLOC_MAP:
            token = mpack_pack_map(data->size);
            break;
    }

    mpack_write(tokbuf, packet_ptr, packet_remaining, &token);
    if (result == MPACK_EOF || *packet_remaining == 0) {
        upsize(packet, packet_ptr, packet_remaining, token.length);
        if (result == MPACK_EOF) {
            mpack_write(tokbuf, packet_ptr, packet_remaining, &token);
        }
    }

    // Write additional data for strings, binaries, arrays, maps, and tuples
    if (schema->type == MORLOC_BINARY) {
        write_to_packet((void*)data->data.char_arr, packet, packet_ptr, packet_remaining, data->size);
    } if (schema->type == MORLOC_STRING) {
        write_to_packet((void*)data->data.char_arr, packet, packet_ptr, packet_remaining, data->size);
    } else if ( schema->type == MORLOC_ARRAY) {
        for (size_t i = 0; i < data->size; i++) {
            pack_data(
              data->data.obj_arr[i],
              schema->parameters[0],
              packet,
              packet_ptr,
              packet_remaining,
              tokbuf
            );
        }
    } else if ( schema->type == MORLOC_BOOL_ARRAY ||
                schema->type == MORLOC_INT_ARRAY ||
                schema->type == MORLOC_FLOAT_ARRAY ) {

        mpack_token_t token;

        for (size_t i = 0; i < data->size; i++){

            switch(schema->type){
              case MORLOC_BOOL_ARRAY:
                token = mpack_pack_boolean(data->data.bool_arr[i]);
                break;
              case MORLOC_INT_ARRAY:
                token = mpack_pack_sint(data->data.int_arr[i]);
                break;
              case MORLOC_FLOAT_ARRAY:
                token = mpack_pack_float(data->data.float_arr[i]);
                break;
              default:
                break;
            }

            result = mpack_write(tokbuf, packet_ptr, packet_remaining, &token);
            if (result == MPACK_EOF || *packet_remaining == 0) {
                upsize(packet, packet_ptr, packet_remaining, token.length);
                if (result == MPACK_EOF) {
                    mpack_write(tokbuf, packet_ptr, packet_remaining, &token);
                }
            }
        }
    } else if (schema->type == MORLOC_TUPLE) {
        for (size_t i = 0; i < schema->size; i++) {
            pack_data(
              data->data.obj_arr[i],
              schema->parameters[i],
              packet,
              packet_ptr,
              packet_remaining,
              tokbuf
            );
        }
    } else if (schema->type == MORLOC_MAP) {
        for (size_t i = 0; i < data->size; i++) {

            char* key = data->data.obj_arr[i]->key;
            size_t key_len = strlen(key);

            // write key string token
            token = mpack_pack_str(key_len);
            result = mpack_write(tokbuf, packet_ptr, packet_remaining, &token);
            if (result == MPACK_EOF || *packet_remaining == 0) {
                upsize(packet, packet_ptr, packet_remaining, token.length);
                if (result == MPACK_EOF) {
                    mpack_write(tokbuf, packet_ptr, packet_remaining, &token);
                }
            }

            // write string bytes
            write_to_packet((void*)key, packet, packet_ptr, packet_remaining, key_len);

            // write value
            pack_data(data->data.obj_arr[i], schema->parameters[i], packet, packet_ptr, packet_remaining, tokbuf);
        }
    }

    return 0;
}



int pack_with_schema(const ParsedData* data, const Schema* schema, char** packet, size_t* packet_size) {
    *packet_size = 0;
    *packet = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (*packet == NULL) return 1;
    size_t packet_remaining = BUFFER_SIZE;
    char* packet_ptr = *packet;

    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);

    int pack_result = pack_data(data, schema, packet, &packet_ptr, &packet_remaining, &tokbuf);

    // mutate packet_size (will be used outside)
    *packet_size = packet_ptr - *packet;

    // Trim the output buffer to the exact size needed
    if (packet_remaining > 0) {
        *packet = (char*)realloc(*packet, *packet_size);
    }

    return pack_result;
}

void write_token(mpack_token_t token){
    switch(token.type){
        case MPACK_TOKEN_NIL:
            printf("NIL");
            break;
        case MPACK_TOKEN_BOOLEAN:
            printf("BOOLEAN");
            break;
        case MPACK_TOKEN_SINT:
            printf("SINT(%zu)", token.length);
            break;
        case MPACK_TOKEN_UINT:
            printf("UINT(%zu)", token.length);
            break;
        case MPACK_TOKEN_FLOAT:
            printf("FLOAT");
            break;
        case MPACK_TOKEN_CHUNK:
            printf("CHUNK(%zu): ", token.length);
            if(token.data.chunk_ptr)
                print_hex(token.data.chunk_ptr, token.length);
            break;
        case MPACK_TOKEN_ARRAY:
            printf("ARRAY(%zu)", token.length);
            break;
        case MPACK_TOKEN_MAP:
            printf("MAP(%zu)", token.length);
            break;
        case MPACK_TOKEN_BIN:
            printf("BIN(%zu)", token.length);
            break;
        case MPACK_TOKEN_STR:
            printf("STR(%zu)", token.length);
            break;
        case MPACK_TOKEN_EXT:
            printf("EXT(%zu)", token.length);
            break;
      default:
        break;
    }
}

void write_schema(const Schema* schema){
    if(!schema){
      printf("NULL");
      return;
    }
    switch(schema->type){
      case MORLOC_NIL: 
          printf("NIL");
          break;
      case MORLOC_BOOL: 
          printf("BOOL");
          break;
      case MORLOC_INT: 
          printf("INT");
          break;
      case MORLOC_FLOAT: 
          printf("FLOAT");
          break;
      case MORLOC_STRING: 
          printf("STRING");
          break;
      case MORLOC_BINARY: 
          printf("BINARY");
          break;
      case MORLOC_ARRAY: 
          printf("ARRAY<");
          write_schema(schema->parameters[0]);
          printf(">");
          break;
      case MORLOC_MAP: 
          printf("MAP{");
          for(size_t i = 0; i < schema->size; i++){
            printf("%s: ", schema->keys[i]);
            write_schema(schema->parameters[i]);
            if(i != schema->size - 1){
                printf(", ");
            }
          }
          printf("}");
          break;
      case MORLOC_TUPLE: 
          printf("TUPLE<");
          for(size_t i = 0; i < schema->size; i++){
            write_schema(schema->parameters[i]);
            if(i != schema->size - 1){
                printf(", ");
            }
          }
          printf(">");
          break;
      case MORLOC_BOOL_ARRAY: 
          printf("BOOL_ARRAY");
          break;
      case MORLOC_INT_ARRAY: 
          printf("INT_ARRAY");
          break;
      case MORLOC_FLOAT_ARRAY: 
          printf("FLOAT_ARRAY");
          break;
      default:
          break;
    }

}


// terminal parsers
ParsedData* parse_binary(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_bool(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_bool_array(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_float(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_float_array(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_int(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_int_array(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_nil(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_string(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);

// nested parsers
ParsedData* parse_array(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_map(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_tuple(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
ParsedData* parse_obj(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);

ParsedData* parse_nil(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    return nil_data();
}

ParsedData* parse_bool(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    return bool_data(mpack_unpack_boolean(*token));
}

ParsedData* parse_int(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    int result = -999;
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    printf("token.length=%zu\n", token->length);
    switch(token->type){
      case MPACK_TOKEN_UINT:
        printf("UINT(%zu)\n", token->length);
        result = (int)mpack_unpack_uint(*token);
        break;
      case MPACK_TOKEN_SINT:
        printf("SINT(%zu)\n", token->length);
        result = (int)mpack_unpack_sint(*token);
        break;
      case MPACK_TOKEN_FLOAT:
        printf("FLOAT(%zu)\n", token->length);
        result = (int)(mpack_unpack_float(*token));
        break;
      default:
        printf("Bad token %d\n", token->type);
        break;
    }
    printf("writing %d\n", result);

    return int_data(result);
}

ParsedData* parse_float(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    return float_data((double)mpack_unpack_float(*token));
}


char* parse_key(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){

    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    char* key = (char*)calloc(token->length + 1, sizeof(char));
    
    size_t str_idx = 0;

    while(size - str_idx > 0){
        mpack_read(tokbuf, buf_ptr, buf_remaining, token);
        memcpy(
          key + str_idx,
          token->data.chunk_ptr,
          token->length * sizeof(char)
        );
        str_idx += token->length;
    }

    return key;
}

ParsedData* parse_string(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){

    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = string_data_(size);
    size_t str_idx = 0;

    while(size - str_idx > 0){
        mpack_read(tokbuf, buf_ptr, buf_remaining, token);
        memcpy(
          result->data.char_arr + str_idx,
          token->data.chunk_ptr,
          token->length * sizeof(char)
        );
        str_idx += token->length;
    }

    return result;
}

ParsedData* parse_binary(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){

    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = binary_data_(size);
    size_t str_idx = 0;

    while(size - str_idx > 0){
        mpack_read(tokbuf, buf_ptr, buf_remaining, token);
        memcpy(
          result->data.char_arr + str_idx,
          token->data.chunk_ptr,
          token->length * sizeof(char)
        );
        str_idx += token->length;
    }

    return result;
}

ParsedData* parse_bool_array(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = array_bool_data_(size);
    for(size_t i; i < size; i++){
      mpack_read(tokbuf, buf_ptr, buf_remaining, token);
      result->data.bool_arr[i] = mpack_unpack_boolean(*token);
    }
    return result;
}

ParsedData* parse_int_array(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = array_int_data_(size);
    for(size_t i = 0; i < size; i++){
      mpack_read(tokbuf, buf_ptr, buf_remaining, token);
      switch(token->type){
        case MPACK_TOKEN_UINT:
          printf("UINT(%zu)\n", token->length);
          printf("lo=%d hi=%d\n", token->data.value.lo, token->data.value.hi);
          result->data.int_arr[i] = (int)mpack_unpack_uint(*token);
          break;
        case MPACK_TOKEN_SINT:
          printf("SINT(%zu)\n", token->length);
          result->data.int_arr[i] = (int)mpack_unpack_sint(*token);
          break;
        case MPACK_TOKEN_FLOAT:
          printf("FLOAT(%zu)\n", token->length);
          result->data.int_arr[i] = (int)(mpack_unpack_float(*token));
          break;
        default:
          printf("Bad token %d\n", token->type);
          break;
      }
      printf("writing %d\n", result->data.int_arr[i]);
    }
    return result;
}

ParsedData* parse_float_array(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = array_float_data_(size);
    for(size_t i = 0; i < size; i++){
      mpack_read(tokbuf, buf_ptr, buf_remaining, token);
      result->data.float_arr[i] = (double)mpack_unpack_float(*token);
    }
    return result;
}

ParsedData* parse_array(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = array_data_(size);

    for(size_t i = 0; i < size; i++){
        result->data.obj_arr[i] = parse_obj(schema, tokbuf, buf_ptr, buf_remaining, token);
    }

    return result;
}

ParsedData* parse_tuple(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = tuple_data_(size);

    for(size_t i = 0; i < size; i++){
        result->data.obj_arr[i] = parse_obj(schema->parameters[i], tokbuf, buf_ptr, buf_remaining, token);
    }

    return result;
}

ParsedData* parse_map(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    ParsedData* result = map_data_(size);
    char* key;

    for(size_t i = 0; i < size; i++){
        key = parse_key(tokbuf, buf_ptr, buf_remaining, token);;
        result->data.obj_arr[i] = parse_obj(schema->parameters[i], tokbuf, buf_ptr, buf_remaining, token);
        result->data.obj_arr[i]->key = key;
    }

    return result;
}

ParsedData* parse_obj(const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    switch(schema->type){
      case MORLOC_NIL:
        return parse_nil(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_BOOL:
        return parse_bool(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_INT:
        return parse_int(tokbuf, buf_ptr, buf_remaining, token);
        break;
      case MORLOC_FLOAT:
        return parse_float(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_STRING:
        return parse_string(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_BINARY:
        return parse_binary(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_ARRAY:
        return parse_array(schema->parameters[0], tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_MAP:
        return parse_map(schema, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_TUPLE:
        return parse_tuple(schema, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_BOOL_ARRAY:
        return parse_bool_array(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_INT_ARRAY:
        return parse_int_array(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_FLOAT_ARRAY:
        return parse_float_array(tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_EXT:
        // no exensions are used yet
        return NULL;
      default:
        break;
    }
}

ParsedData* unpack_with_schema(const char** buf_ptr, size_t* buf_remaining, const Schema* schema) {
    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);
    mpack_token_t token;
    return(parse_obj(schema, &tokbuf, buf_ptr, buf_remaining, &token));
}


void write_tokens(const char** buf_ptr, size_t* buf_remaining){
    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);
    mpack_token_t token;

    int read_result;

    do {
      read_result = mpack_read(&tokbuf, buf_ptr, buf_remaining, &token);
      write_token(token);
      printf("\n");
    } while(read_result == MPACK_OK);
}


Packet pack(const ParsedData* data, const Schema* schema){

    /* print_parsed_data(data, 0); */
    /* print_schema(schema, 0);    */

    char* packet = NULL;
    size_t packet_size = 0;
    int result = pack_with_schema(data, schema, &packet, &packet_size);

    const char* buf = packet;
    size_t size = packet_size;
    print_hex(buf, size);
    printf("\n");

    /* const char* buf = packet;         */
    /* write_tokens(&buf, &packet_size); */

    /* printf("result_code=%d\n", result); */
    /* print_hex(packet, packet_size);     */
    /* printf("\n");                       */

    Packet packet_obj;
    packet_obj.data = packet;
    packet_obj.size = packet_size;
    return packet_obj;
}

ParsedData* unpack(Packet packet, const Schema* schema){
    const char* buf = packet.data;
    size_t buf_remaining = packet.size;
    ParsedData* data = unpack_with_schema(&buf, &buf_remaining, schema);

    /* print_parsed_data(data, 0); */
    /* print_schema(schema, 0);    */

/* 94 2a 45 cd 01 a4 ce 00 01 0f 2c */
/* {                                */
/*   "type": "INT_ARRAY",           */
/*   "size": 4,                     */
/*   "data": [42, 69, 420, 0]       */
/* }{                               */
/*   "type": "INT_ARRAY",           */
/*   "size": 0                      */
/* Original:  [42, 69, 420, 69420]  */
/* Roundtrip: [42, 69, 420, 0]      */

    return data;
}


// Function to get the string representation of morloc_serial_type
const char* get_type_string(morloc_serial_type type) {
    switch (type) {
        case MORLOC_NIL: return "NIL";
        case MORLOC_BOOL: return "BOOL";
        case MORLOC_INT: return "INT";
        case MORLOC_FLOAT: return "FLOAT";
        case MORLOC_STRING: return "STRING";
        case MORLOC_BINARY: return "BINARY";
        case MORLOC_ARRAY: return "ARRAY";
        case MORLOC_MAP: return "MAP";
        case MORLOC_TUPLE: return "TUPLE";
        case MORLOC_BOOL_ARRAY: return "BOOL_ARRAY";
        case MORLOC_INT_ARRAY: return "INT_ARRAY";
        case MORLOC_FLOAT_ARRAY: return "FLOAT_ARRAY";
        case MORLOC_EXT: return "EXT";
        default: return "UNKNOWN";
    }
}

// Function to print Schema
void print_schema(const Schema* schema, int indent) {
    if (schema == NULL) {
        printf("null");
        return;
    }

    printf("{\n");
    printf("%*s\"type\": \"%s\",\n", indent + 2, "", get_type_string(schema->type));
    printf("%*s\"size\": %zu", indent + 2, "", schema->size);

    if (schema->size > 0) {
        printf(",\n%*s\"parameters\": [\n", indent + 2, "");
        for (size_t i = 0; i < schema->size; i++) {
            printf("%*s", indent + 4, "");
            print_schema(schema->parameters[i], indent + 4);
            if (i < schema->size - 1) printf(",");
            printf("\n");
        }
        printf("%*s]", indent + 2, "");

        if (schema->keys != NULL) {
            printf(",\n%*s\"keys\": [", indent + 2, "");
            for (size_t i = 0; i < schema->size; i++) {
                printf("\"%s\"", schema->keys[i]);
                if (i < schema->size - 1) printf(", ");
            }
            printf("]");
        }
    }

    printf("\n%*s}", indent, "");
}

// Function to print ParsedData
void print_parsed_data(const ParsedData* data, int indent) {
    if (data == NULL) {
        printf("null");
        return;
    }

    printf("{\n");
    printf("%*s\"type\": \"%s\",\n", indent + 2, "", get_type_string(data->type));
    printf("%*s\"size\": %zu,\n", indent + 2, "", data->size);
    
    if (data->key != NULL) {
        printf("%*s\"key\": \"%s\",\n", indent + 2, "", data->key);
    }

    printf("%*s\"data\": ", indent + 2, "");

    switch (data->type) {
        case MORLOC_NIL:
            printf("null");
            break;
        case MORLOC_BOOL:
            printf("%s", data->data.bool_val ? "true" : "false");
            break;
        case MORLOC_INT:
            printf("%d", data->data.int_val);
            break;
        case MORLOC_FLOAT:
            printf("%f", data->data.double_val);
            break;
        case MORLOC_STRING:
            printf("\"%s\"", data->data.char_arr);
            break;
        case MORLOC_BINARY:
            printf("\"<binary data>\"");
            break;
        case MORLOC_ARRAY:
        case MORLOC_MAP:
        case MORLOC_TUPLE:
            printf("[\n");
            for (size_t i = 0; i < data->size; i++) {
                printf("%*s", indent + 4, "");
                print_parsed_data(data->data.obj_arr[i], indent + 4);
                if (i < data->size - 1) printf(",");
                printf("\n");
            }
            printf("%*s]", indent + 2, "");
            break;
        case MORLOC_BOOL_ARRAY:
            printf("[");
            for (size_t i = 0; i < data->size; i++) {
                printf("%s", data->data.bool_arr[i] ? "true" : "false");
                if (i < data->size - 1) printf(", ");
            }
            printf("]");
            break;
        case MORLOC_INT_ARRAY:
            printf("[");
            for (size_t i = 0; i < data->size; i++) {
                printf("%d", data->data.int_arr[i]);
                if (i < data->size - 1) printf(", ");
            }
            printf("]");
            break;
        case MORLOC_FLOAT_ARRAY:
            printf("[");
            for (size_t i = 0; i < data->size; i++) {
                printf("%f", data->data.float_arr[i]);
                if (i < data->size - 1) printf(", ");
            }
            printf("]");
            break;
        case MORLOC_EXT:
            printf("\"<extension data>\"");
            break;
    }

    printf("\n%*s}", indent, "");
}
