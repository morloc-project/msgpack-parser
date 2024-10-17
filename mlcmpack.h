#ifndef __MLCMPACK_H__
#define __MLCMPACK_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "mpack.h"

typedef enum {
  MORLOC_NIL     = 0,
  MORLOC_BOOL    = 1,
  MORLOC_SINT    = 2,
  MORLOC_UINT    = 3,
  MORLOC_FLOAT   = 4,
  MORLOC_STRING  = 5,
  MORLOC_BINARY  = 6,
  MORLOC_ARRAY   = 7,
  MORLOC_MAP     = 8,
  MORLOC_TUPLE   = 9,
} morloc_serial_type;

#define TUPLE_EXT_TYPE 0
#define INITIAL_BUFFER_SIZE 4096

// Forward declarations
struct Schema;
struct ParsedData;

// Schema definition
//  * Primitives have no parameters
//  * Arrays have one
//  * Tuples and records have one or more
typedef struct Schema {
    morloc_serial_type type;
    int size; // number of parameters
    struct Schema** parameters;
    char** keys; // field names, used only for records
} Schema;

// Parsed data structure
typedef struct ParsedData {
    morloc_serial_type type;
    union {
        char nil_val; // set to 0x00, but the actual value will not be used
        bool bool_val;
        int sint_val;
        unsigned int uint_val;
        double double_val;
        char* bytes_val; // string or bytes
        struct {
          int size;
          struct ParsedData* elements;
        } array_val; // tuple or array
        struct {
          int size;
          char* keys;
          struct ParsedData* elements;
        } map_val;
    } data;
} ParsedData;

// Prototypes

Schema* create_schema(morloc_serial_type type, int size, Schema** parameters, char** keys);
void free_schema(Schema* schema);

ParsedData* create_parsed_data(morloc_serial_type type);
void free_parsed_data(ParsedData* data);

void print_parsed_data(const ParsedData* data);
void print_schema(const Schema* schema);

// Function to check if parsed data matches the schema
bool check_data(const ParsedData* data, const Schema* schema);

// Main unpack function for reading morloc-encoded MessagePack data
ParsedData* unpack_with_schema(const char* data, size_t size, const Schema* schema);

// Main pack function for creating morloc-encoded MessagePack data
void pack_with_schema(const ParsedData* data, const Schema* schema, char** packet, size_t* packet_size);

#endif
