#ifndef __MLCMPACK_H__
#define __MLCMPACK_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "mpack.h"

typedef enum {
  MORLOC_NIL           =  0,
  MORLOC_BOOL          =  1,
  MORLOC_SINT          =  2,
  MORLOC_UINT          =  3,
  MORLOC_FLOAT         =  4,
  MORLOC_STRING        =  5,
  MORLOC_BINARY        =  6,
  MORLOC_ARRAY         =  7,
  MORLOC_MAP           =  8,
  MORLOC_TUPLE         =  9,
  MORLOC_BOOL_ARRAY    =  10,
  MORLOC_SINT_ARRAY    =  11,
  MORLOC_UINT_ARRAY    =  12,
  MORLOC_FLOAT_ARRAY   =  13,
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
    size_t size; // number of parameters
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
        char* string_val;

        // an array of boxed objects or a tuple
        struct {
          size_t size;
          struct ParsedData** elements;
        } array_val;

        // primitive arrays
        struct {
          size_t size;
          char* elements;
        } binary_val;

        struct {
          size_t size;
          bool* elements;
        } array_bool_val;

        struct {
          size_t size;
          int* elements;
        } array_sint_val;

        struct {
          size_t size;
          unsigned int* elements;
        } array_uint_val;

        struct {
          size_t size;
          double* elements;
        } array_float_val;

        // a map
        struct {
          size_t size;
          char** keys;
          struct ParsedData** elements;
        } map_val;
    } data;
} ParsedData;

// Prototypes

Schema* create_schema(morloc_serial_type type, size_t size, Schema** parameters, char** keys);
void free_schema(Schema* schema);

void free_parsed_data(ParsedData* data);

void print_parsed_data(const ParsedData* data);
void print_schema(const Schema* schema);

// Main unpack function for reading morloc-encoded MessagePack data
ParsedData* unpack_with_schema(const char** buf_ptr, size_t* buf_remaining, const Schema* schema);

// Main pack function for creating morloc-encoded MessagePack data
int pack_with_schema(const ParsedData* data, const Schema* schema, char** packet, size_t* packet_size);


// Helper structure for key-value pairs
typedef struct {
    char* key;
    Schema* value;
} SchemaKeyValuePair;

// schema constructors
Schema* nil_schema();
Schema* bool_schema();
Schema* sint_schema();
Schema* uint_schema();
Schema* float_schema();
Schema* bool_array_schema();
Schema* sint_array_schema();
Schema* uint_array_schema();
Schema* float_array_schema();
Schema* string_schema();
Schema* binary_schema();
Schema* tuple_schema(size_t size, ...);
Schema* array_schema(Schema* array_type);
SchemaKeyValuePair* kvp_schema(const char* key, Schema* value);
Schema* map_schema(size_t size, ...);


// create atomic values
ParsedData* nil_data();
ParsedData* bool_data(bool value);
ParsedData* sint_data(int value);
ParsedData* uint_data(unsigned int value);
ParsedData* float_data(double value);
// strings and binary
ParsedData* string_data(const char* value);
ParsedData* binary_data(const char* value, size_t size);
// unboxed arrays
ParsedData* array_bool_data(const bool* values, size_t size);
ParsedData* array_sint_data(const int* values, size_t size);
ParsedData* array_uint_data(const unsigned int* values, size_t size);
ParsedData* array_float_data(const double* values, size_t size);
// unboxed uninitialized arrays
ParsedData* array_bool_data_(size_t size);
ParsedData* array_sint_data_(size_t size);
ParsedData* array_uint_data_(size_t size);
ParsedData* array_float_data_(size_t size);
// containers, set elements individually
ParsedData* array_data(size_t size);
ParsedData* tuple_data(size_t size);
ParsedData* map_data(size_t size);
// helper for setting map elements
int set_map_element(ParsedData* map, const char* key, ParsedData* value);

#endif
