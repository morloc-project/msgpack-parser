#include "mlcmpack.h"


// Allocate memory for a new Schema
Schema* create_schema(morloc_serial_type type, int size, Schema** parameters, char** keys) {
    Schema* schema = (Schema*)malloc(sizeof(Schema));
    if (!schema) return NULL;

    schema->type = type;
    schema->size = size;
    schema->parameters = parameters;
    schema->keys = keys;

    return schema;
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

// Create a new ParsedData
ParsedData* create_parsed_data(morloc_serial_type type) {
    ParsedData* data = (ParsedData*)malloc(sizeof(ParsedData));
    if (!data) return NULL;

    data->type = type;
    memset(&data->data, 0, sizeof(data->data));  // Initialize union to zeros

    return data;
}

// Free a ParsedData and its contents
void free_parsed_data_contents(ParsedData* data) {
    if (!data) return;
    free_parsed_data(data);
    // Don't free 'data' itself, as it might be part of an array
}
void free_parsed_data(ParsedData* data) {
    if (!data) return;

    switch (data->type) {
        case MORLOC_STRING:
        case MORLOC_BINARY:
            free(data->data.bytes_val);
            break;
        case MORLOC_ARRAY:
        case MORLOC_TUPLE:
            if (data->data.array_val.elements) {
                for (int i = 0; i < data->data.array_val.size; i++) {
                    free_parsed_data_contents(&data->data.array_val.elements[i]);
                }
                free(data->data.array_val.elements);
            }
            break;
        case MORLOC_MAP:
            if (data->data.map_val.elements) {
                for (int i = 0; i < data->data.map_val.size; i++) {
                    free_parsed_data_contents(&data->data.map_val.elements[i]);
                }
                free(data->data.map_val.elements);
            }
            free(data->data.map_val.keys);
            break;
        default:
            // No additional freeing needed for other types
            break;
    }
}


void print_parsed_data_indent(const ParsedData* data, int indent) {
    if (!data) return;

    char indent_str[100] = "";
    for (int i = 0; i < indent; i++) strcat(indent_str, "  ");

    switch (data->type) {
        case MORLOC_NIL:
            printf("%snil\n", indent_str);
            break;
        case MORLOC_BOOL:
            printf("%s%s\n", indent_str, data->data.bool_val ? "true" : "false");
            break;
        case MORLOC_SINT:
            printf("%s%d\n", indent_str, data->data.sint_val);
            break;
        case MORLOC_UINT:
            printf("%s%u\n", indent_str, data->data.uint_val);
            break;
        case MORLOC_FLOAT:
            printf("%s%f\n", indent_str, data->data.double_val);
            break;
        case MORLOC_STRING:
            printf("%s\"%s\"\n", indent_str, data->data.bytes_val);
            break;
        case MORLOC_BINARY:
            printf("%s", indent_str);
            for (int i = 0; data->data.bytes_val[i] != '\0'; i++) {
                printf("%02x", (unsigned char)data->data.bytes_val[i]);
            }
            printf("\n");
            break;
        case MORLOC_ARRAY:
            printf("%s[\n", indent_str);
            for (int i = 0; i < data->data.array_val.size; i++) {
                print_parsed_data_indent(&data->data.array_val.elements[i], indent + 1);
                if (i < data->data.array_val.size - 1) printf("%s,\n", indent_str);
            }
            printf("%s]\n", indent_str);
            break;
        case MORLOC_TUPLE:
            printf("%s(\n", indent_str);
            for (int i = 0; i < data->data.array_val.size; i++) {
                print_parsed_data_indent(&data->data.array_val.elements[i], indent + 1);
                if (i < data->data.array_val.size - 1) printf("%s,\n", indent_str);
            }
            printf("%s)\n", indent_str);
            break;
        case MORLOC_MAP:
            printf("%s{\n", indent_str);
            for (int i = 0; i < data->data.map_val.size; i++) {
                printf("%s  %s: ", indent_str, &data->data.map_val.keys[i * 50]); // Assuming max key length of 50
                print_parsed_data_indent(&data->data.map_val.elements[i], indent + 1);
                if (i < data->data.map_val.size - 1) printf("%s;\n", indent_str);
            }
            printf("%s}\n", indent_str);
            break;
        default:
            printf("%sUnknown type\n", indent_str);
    }
}

// Function to print a schema
void print_schema(const Schema* schema) {
    if (!schema) {
        printf("NULL schema\n");
        return;
    }

    const char* type_names[] = {
        "NIL", "BOOL", "SINT", "UINT", "FLOAT", "STRING",
        "BINARY", "ARRAY", "MAP", "TUPLE"
    };

    printf("Schema Type: %s\n", type_names[schema->type]);
    printf("Size: %d\n", schema->size);

    if (schema->type == MORLOC_ARRAY || schema->type == MORLOC_TUPLE) {
        printf("Parameters:\n");
        for (int i = 0; i < schema->size; i++) {
            printf("  Parameter %d:\n", i);
            print_schema(schema->parameters[i]);
        }
    } else if (schema->type == MORLOC_MAP) {
        printf("Keys and Parameters:\n");
        for (int i = 0; i < schema->size; i++) {
            printf("  Key: %s\n", schema->keys[i]);
            printf("  Value Schema:\n");
            print_schema(schema->parameters[i]);
        }
    }
}

bool check_data(const ParsedData* data, const Schema* schema) {
    if (!data || !schema) {
        return false;
    }

    if (data->type != schema->type) {
        return false;
    }

    switch (schema->type) {
        case MORLOC_NIL:
        case MORLOC_BOOL:
        case MORLOC_SINT:
        case MORLOC_UINT:
        case MORLOC_FLOAT:
        case MORLOC_STRING:
        case MORLOC_BINARY:
            return true;  // These types don't need further checking

        case MORLOC_ARRAY:
        case MORLOC_TUPLE:
            if (data->data.array_val.size != schema->size) {
                return false;
            }
            for (int i = 0; i < schema->size; i++) {
                if (!check_data(&data->data.array_val.elements[i], schema->parameters[i])) {
                    return false;
                }
            }
            return true;

        case MORLOC_MAP:
            if (data->data.map_val.size != schema->size) {
                return false;
            }
            for (int i = 0; i < schema->size; i++) {
                // Check if the key exists and matches
                bool key_found = false;
                for (int j = 0; j < data->data.map_val.size; j++) {
                    if (strcmp(&data->data.map_val.keys[j * 50], schema->keys[i]) == 0) {
                        key_found = true;
                        if (!check_data(&data->data.map_val.elements[j], schema->parameters[i])) {
                            return false;
                        }
                        break;
                    }
                }
                if (!key_found) {
                    return false;
                }
            }
            return true;

        default:
            return false;  // Unknown type
    }
}

ParsedData* unpack_with_schema(const char* data, size_t size, const Schema* schema) {
    if (!data || !schema) return NULL;

    mpack_parser_t parser;
    mpack_parser_init(&parser, MPACK_MAX_OBJECT_DEPTH);
    
    ParsedData* result = create_parsed_data(schema->type);
    if (!result) return NULL;

    parser.data.p = result;

    const char* current_data = data;
    size_t remaining_size = size;

    while (remaining_size > 0) {
        mpack_token_t token;
        int rc = mpack_read(&parser.tokbuf, &current_data, &remaining_size, &token);
        
        if (rc != MPACK_OK) {
            if (rc == MPACK_EOF) break;
            free_parsed_data(result);
            return NULL;
        }

        switch (token.type) {
            case MPACK_TOKEN_NIL:
                result->type = MORLOC_NIL;
                break;
            case MPACK_TOKEN_BOOLEAN:
                result->type = MORLOC_BOOL;
                result->data.bool_val = mpack_unpack_boolean(token);
                break;
            case MPACK_TOKEN_UINT:
                result->type = MORLOC_UINT;
                result->data.uint_val = (unsigned int)mpack_unpack_sint(token);
                break;
            case MPACK_TOKEN_SINT:
                result->type = MORLOC_SINT;
                result->data.sint_val = (int)mpack_unpack_sint(token);
                break;
            case MPACK_TOKEN_FLOAT:
                result->type = MORLOC_FLOAT;
                result->data.double_val = mpack_unpack_float(token);
                break;
            case MPACK_TOKEN_STR:
                result->type = MORLOC_STRING;
                result->data.bytes_val = malloc(token.length + 1);
                memcpy(result->data.bytes_val, token.data.chunk_ptr, token.length);
                result->data.bytes_val[token.length] = '\0';
                break;
            case MPACK_TOKEN_BIN:
                result->type = MORLOC_BINARY;
                result->data.bytes_val = malloc(token.length);
                memcpy(result->data.bytes_val, token.data.chunk_ptr, token.length);
                break;
            case MPACK_TOKEN_ARRAY:
                result->type = MORLOC_ARRAY;
                result->data.array_val.size = token.length;
                result->data.array_val.elements = malloc(sizeof(ParsedData) * token.length);
                for (int i = 0; i < token.length; i++) {
                    result->data.array_val.elements[i] = *unpack_with_schema(current_data, remaining_size, schema->parameters[i]);
                }
                break;
            case MPACK_TOKEN_MAP:
                result->type = MORLOC_MAP;
                result->data.map_val.size = token.length;
                result->data.map_val.elements = malloc(sizeof(ParsedData) * token.length);
                result->data.map_val.keys = malloc(50 * token.length); // Assuming max key length of 50
                for (int i = 0; i < token.length; i++) {
                    // Read key
                    mpack_token_t key_token;
                    mpack_read(&parser.tokbuf, &current_data, &remaining_size, &key_token);
                    memcpy(&result->data.map_val.keys[i * 50], key_token.data.chunk_ptr, key_token.length);
                    result->data.map_val.keys[i * 50 + key_token.length] = '\0';
                    
                    // Read value
                    result->data.map_val.elements[i] = *unpack_with_schema(current_data, remaining_size, schema->parameters[i]);
                }
                break;
            default:
                free_parsed_data(result);
                return NULL;
        }

        if (result->type != schema->type) {
            free_parsed_data(result);
            return NULL;
        }
    }

    return result;
}



/* void pack_with_schema(const ParsedData* data, const Schema* schema, char** packet, size_t* packet_size) {             */
/*     mpack_tokbuf_t tb;                                                                                                */
/*     mpack_tokbuf_init(&tb);                                                                                           */
/*     size_t buffer_size = INITIAL_BUFFER_SIZE;                                                                         */
/*     char* buffer = (char*)malloc(buffer_size * sizeof(char));                                                         */
/*     if (!buffer) {                                                                                                    */
/*         *packet = NULL;                                                                                               */
/*         *packet_size = 0;                                                                                             */
/*         return;                                                                                                       */
/*     }                                                                                                                 */
/*                                                                                                                       */
/*     size_t bytes_written = 0;                                                                                         */
/*     mpack_token_t token;                                                                                              */
/*     int rc;                                                                                                           */
/*                                                                                                                       */
/*     switch (schema->type) {                                                                                           */
/*         case MORLOC_NIL:                                                                                              */
/*             token = mpack_pack_nil();                                                                                 */
/*             break;                                                                                                    */
/*         case MORLOC_BOOL:                                                                                             */
/*             token = mpack_pack_boolean(data->data.bool_val);                                                          */
/*             break;                                                                                                    */
/*         case MORLOC_SINT:                                                                                             */
/*             token = mpack_pack_sint(data->data.sint_val);                                                             */
/*             break;                                                                                                    */
/*         case MORLOC_UINT:                                                                                             */
/*             token = mpack_pack_uint(data->data.uint_val);                                                             */
/*             break;                                                                                                    */
/*         case MORLOC_FLOAT:                                                                                            */
/*             token = mpack_pack_float(data->data.double_val);                                                          */
/*             break;                                                                                                    */
/*         case MORLOC_STRING:                                                                                           */
/*             token = mpack_pack_str(strlen(data->data.bytes_val));                                                     */
/*             rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                     */
/*             if (rc != MPACK_OK) goto error;                                                                           */
/*             bytes_written += buffer_size;                                                                             */
/*             token = mpack_pack_chunk(data->data.bytes_val, strlen(data->data.bytes_val));                             */
/*             break;                                                                                                    */
/*         case MORLOC_BINARY:                                                                                           */
/*             token = mpack_pack_bin(data->data.array_val.size);                                                        */
/*             rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                     */
/*             if (rc != MPACK_OK) goto error;                                                                           */
/*             bytes_written += buffer_size;                                                                             */
/*             token = mpack_pack_chunk(data->data.bytes_val, data->data.array_val.size);                                */
/*             break;                                                                                                    */
/*         case MORLOC_ARRAY:                                                                                            */
/*             token = mpack_pack_array(data->data.array_val.size);                                                      */
/*             rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                     */
/*             if (rc != MPACK_OK) goto error;                                                                           */
/*             bytes_written += buffer_size;                                                                             */
/*             for (int i = 0; i < data->data.array_val.size; i++) {                                                     */
/*                 pack_with_schema(&data->data.array_val.elements[i], schema->parameters[i], &buffer, &buffer_size);    */
/*                 bytes_written += buffer_size;                                                                         */
/*             }                                                                                                         */
/*             break;                                                                                                    */
/*         case MORLOC_MAP:                                                                                              */
/*             token = mpack_pack_map(data->data.map_val.size);                                                          */
/*             rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                     */
/*             if (rc != MPACK_OK) goto error;                                                                           */
/*             bytes_written += buffer_size;                                                                             */
/*             for (int i = 0; i < data->data.map_val.size; i++) {                                                       */
/*                 token = mpack_pack_str(strlen(&data->data.map_val.keys[i * 50]));                                     */
/*                 rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                 */
/*                 if (rc != MPACK_OK) goto error;                                                                       */
/*                 bytes_written += buffer_size;                                                                         */
/*                 token = mpack_pack_chunk(&data->data.map_val.keys[i * 50], strlen(&data->data.map_val.keys[i * 50])); */
/*                 rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                 */
/*                 if (rc != MPACK_OK) goto error;                                                                       */
/*                 bytes_written += buffer_size;                                                                         */
/*                 pack_with_schema(&data->data.map_val.elements[i], schema->parameters[i], &buffer, &buffer_size);      */
/*                 bytes_written += buffer_size;                                                                         */
/*             }                                                                                                         */
/*             break;                                                                                                    */
/*         case MORLOC_TUPLE:                                                                                            */
/*             token = mpack_pack_ext(TUPLE_EXT_TYPE, schema->size);                                                     */
/*             rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                     */
/*             if (rc != MPACK_OK) goto error;                                                                           */
/*             bytes_written += buffer_size;                                                                             */
/*             token = mpack_pack_array(schema->size);                                                                   */
/*             rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                     */
/*             if (rc != MPACK_OK) goto error;                                                                           */
/*             bytes_written += buffer_size;                                                                             */
/*             for (int i = 0; i < schema->size; i++) {                                                                  */
/*                 pack_with_schema(&data->data.array_val.elements[i], schema->parameters[i], &buffer, &buffer_size);    */
/*                 bytes_written += buffer_size;                                                                         */
/*             }                                                                                                         */
/*             break;                                                                                                    */
/*         default:                                                                                                      */
/*             goto error;                                                                                               */
/*     }                                                                                                                 */
/*                                                                                                                       */
/*     rc = mpack_write(&tb, &buffer, &buffer_size, &token);                                                             */
/*     if (rc != MPACK_OK) goto error;                                                                                   */
/*     bytes_written += buffer_size;                                                                                     */
/*                                                                                                                       */
/*     *packet = buffer;                                                                                                 */
/*     *packet_size = bytes_written;                                                                                     */
/*     return;                                                                                                           */
/*                                                                                                                       */
/* error:                                                                                                                */
/*     free(buffer);                                                                                                     */
/*     *packet = NULL;                                                                                                   */
/*     *packet_size = 0;                                                                                                 */
/* }                                                                                                                     */




void print_parsed_data(const ParsedData* data) {
  print_parsed_data_indent(data, 0);
}
