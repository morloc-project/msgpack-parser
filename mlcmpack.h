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
  MORLOC_INT           =  2,
  MORLOC_FLOAT         =  3,
  MORLOC_STRING        =  4,
  MORLOC_BINARY        =  5,
  MORLOC_ARRAY         =  6,
  MORLOC_MAP           =  7,
  MORLOC_TUPLE         =  8,
  MORLOC_BOOL_ARRAY    =  9,
  MORLOC_INT_ARRAY     =  10,
  MORLOC_FLOAT_ARRAY   =  11,
  MORLOC_EXT           =  12
} morloc_serial_type;

#define BUFFER_SIZE 4096

// The maximum nesting depth of a data structure. This should be deep enough for
// any non-recursive datastructure. For recursive structures, trees and such, I
// will make a dedicated function that does not depend on this limit. This
// function would use a linked list for the stack instead of an array. The
// downside of the linked list is that it must live on the heap and will be
// slower.
#define MAX_DEPTH 128

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

// A data structure that stores anything representable by MessagePack
// The morloc pools will need to transform their data to/from this form
typedef struct ParsedData {
    morloc_serial_type type;
    size_t size; // 0 for primitives, array length for containers
    char* key; // NULL terminated string, used for names of elements in maps
    union {
        char nil_val; // set to 0x00, but the actual value will not be used
        bool bool_val;
        int int_val;
        double double_val;
        char* char_arr; // bytes, strings, and extensions
        bool* bool_arr;
        int* int_arr;
        double* float_arr;
        struct ParsedData** obj_arr; // general arrays, tuples, and maps
    } data;
} ParsedData;

// Prototypes

Schema* create_schema(morloc_serial_type type, size_t size, Schema** parameters, char** keys);
void free_schema(Schema* schema);

void free_parsed_data(ParsedData* data);

// void print_parsed_data(const ParsedData* data, int indent);
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
Schema* int_schema();
Schema* float_schema();
Schema* bool_array_schema();
Schema* int_array_schema();
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
ParsedData* int_data(int value);
ParsedData* float_data(double value);
// strings and binary
ParsedData* string_data(const char* value, size_t size);
ParsedData* binary_data(const char* value, size_t size);
// unboxed arrays
ParsedData* array_bool_data(const bool* values, size_t size);
ParsedData* array_int_data(const int* values, size_t size);
ParsedData* array_float_data(const double* values, size_t size);
// unboxed uninitialized arrays
ParsedData* array_bool_data_(size_t size);
ParsedData* array_int_data_(size_t size);
ParsedData* array_float_data_(size_t size);
// containers, set elements individually
ParsedData* array_data_(size_t size);
ParsedData* tuple_data_(size_t size);
ParsedData* map_data_(size_t size);
// helper for setting map elements
void set_map_element(ParsedData* map, size_t pos, const char* key, ParsedData* value);

void print_hex(const char* data, size_t size);
void write_tokens(const char** buf_ptr, size_t* buf_remaining);

#endif
