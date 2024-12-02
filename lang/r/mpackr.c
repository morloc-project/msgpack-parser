#include <R.h>
#include <Rinternals.h>
#include "mlcmpack.h"

#include <Rdefines.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <R_ext/Arith.h>


SEXP to_mesgpack(SEXP r_obj, SEXP r_schema_str);
SEXP from_mesgpack(SEXP r_packed, SEXP r_schema_str);
void* to_voidstar(void* dest, SEXP obj, const Schema* schema);
SEXP from_voidstar(const void* data, const Schema* schema);


// Anything* r_to_anything_int(Anything* result, SEXP obj, const Schema* schema) {
//   result->size = 0;
//   if (TYPEOF(obj) == INTSXP) {
//     result->data.int_val = INTEGER(obj)[0];
//   } else if (TYPEOF(obj) == REALSXP) {
//     double val = REAL(obj)[0];
//     if (val > INT_MAX || val < INT_MIN) {
//       free(result);
//       error("Double value %f out of range for integer", val);
//     }
//     result->data.int_val = (int)val;
//   } else {
//     free(result);
//     error("Expected integer or double, got %s", type2char(TYPEOF(obj)));
//   }
//   return result;
// }
// 
// Anything* r_to_anything_float(Anything* result, SEXP obj, const Schema* schema) {
//   if (TYPEOF(obj) != REALSXP) {
//     free(result);
//     error("Expected double, got %s", type2char(TYPEOF(obj)));
//   }
//   result->size = 0;
//   result->data.double_val = REAL(obj)[0];
//   return result;
// }
// 
// Anything* r_to_anything_binary(Anything* result, SEXP obj, const Schema* schema) {
//   if (TYPEOF(obj) != RAWSXP) {
//     free(result);
//     error("Expected raw vector, got %s", type2char(TYPEOF(obj)));
//   }
//   result->size = LENGTH(obj);
//   result->data.char_arr = (char*)malloc(result->size);
//   if (!result->data.char_arr) {
//     free(result);
//     error("Failed to allocate memory for binary data");
//   }
//   memcpy(result->data.char_arr, RAW(obj), result->size);
//   return result;
// }
// 
// Anything* r_to_anything_string(Anything* result, SEXP obj, const Schema* schema) {
// 
//     if (TYPEOF(obj) != STRSXP || LENGTH(obj) != 1) {
//         free(result);
//         error("Expected a single-element character vector, got %s with length %d", 
//               type2char(TYPEOF(obj)), LENGTH(obj));
//     }
// 
//     result->type = MORLOC_STRING;
//     result->data.char_arr = (char*)CHAR(STRING_ELT(obj, 0));
//     result->size = strlen(result->data.char_arr);
// 
//     return result;
// }
// 
// 
// Anything* r_to_anything_string_list(Anything* result, SEXP obj, const Schema* schema) {
//     for (size_t i = 0; i < result->size; i++) {
//         SEXP str = VECTOR_ELT(obj, i);
//         if (str == NA_STRING) {
//             error("NA strings are not supported");
//         } else {
//           result->data.obj_arr[i] = (Anything*)malloc(sizeof(Anything));
//           result->data.obj_arr[i] = r_to_anything_string(
//             result->data.obj_arr[i],
//             str,
//             schema
//           );
//        }
//     }
//     return result;
// }
// 
// 
// Anything* r_to_anything_string_vector(Anything* result, SEXP obj, const Schema* schema) {
//     for (size_t i = 0; i < result->size; i++) {
//         SEXP str = STRING_ELT(obj, i);
//         if (str == NA_STRING) {
//             error("NA strings are not supported");
//         } else {
//             const char* c_str = CHAR(str);
//             result->data.obj_arr[i] = (Anything*)malloc(sizeof(Anything));
//             result->data.obj_arr[i]->type = MORLOC_STRING;
//             result->data.obj_arr[i]->size = strlen(c_str);
//             result->data.obj_arr[i]->data.char_arr = strdup(c_str);
//         }
//     }
//     return result;
// }
// 
// Anything* r_to_anything_array(Anything* result, SEXP obj, const Schema* schema) {
// 
//    if (! (TYPEOF(obj) == VECSXP || TYPEOF(obj) == STRSXP)) {
//      free(result);
//      error("Expected list, got %s", type2char(TYPEOF(obj)));
//    }
// 
//   const Schema* element_schema = schema->parameters[0];
// 
//   result->type = MORLOC_ARRAY;
//   result->size = LENGTH(obj);
//   result->data.obj_arr = (Anything**)malloc(result->size * sizeof(Anything*));
//   if (!result->data.obj_arr) {
//     free(result);
//     error("Failed to allocate memory for array");
//   }
// 
//   if (element_schema->type == MORLOC_STRING){
//     if (TYPEOF(obj) == VECSXP) {
//       return r_to_anything_string_list(result, obj, schema);
//     } else if (TYPEOF(obj) == STRSXP) {
//       return r_to_anything_string_vector(result, obj, schema);
//     }
//   }
// 
//   for (size_t i = 0; i < result->size; i++) {
//     result->data.obj_arr[i] = r_to_anything(VECTOR_ELT(obj, i), element_schema);
//   }
//   return result;
// }
// 
// Anything* r_to_anything_tuple(Anything* result, SEXP obj, const Schema* schema) {
//   if (TYPEOF(obj) != VECSXP) {
//     free(result);
//     error("Expected list, got %s", type2char(TYPEOF(obj)));
//   }
//   result->size = LENGTH(obj);
//   if (result->size != schema->size) {
//     free(result);
//     error("Tuple size mismatch: expected %zu, got %zu", schema->size, result->size);
//   }
//   result->data.obj_arr = (Anything**)malloc(result->size * sizeof(Anything*));
//   if (!result->data.obj_arr) {
//     free(result);
//     error("Failed to allocate memory for tuple");
//   }
//   for (size_t i = 0; i < result->size; i++) {
//     result->data.obj_arr[i] = r_to_anything(VECTOR_ELT(obj, i), schema->parameters[i]);
//   }
//   return result;
// }
// 
// Anything* r_to_anything_map(Anything* result, SEXP obj, const Schema* schema) {
//   if (TYPEOF(obj) != VECSXP) {
//     free(result);
//     error("Expected list, got %s", type2char(TYPEOF(obj)));
//   }
//   
//   result->size = LENGTH(obj);
//   result->data.obj_arr = (Anything**)malloc(result->size * sizeof(Anything*));
//   if (!result->data.obj_arr) {
//     free(result);
//     error("Failed to allocate memory for map");
//   }
//   SEXP names = getAttrib(obj, R_NamesSymbol);
//   if (names == R_NilValue) {
//     free(result->data.obj_arr);
//     free(result);
//     error("Expected named list for map");
//   }
//   for (size_t i = 0; i < result->size; i++) {
//     result->data.obj_arr[i] = r_to_anything(VECTOR_ELT(obj, i), schema->parameters[i]);
//     result->data.obj_arr[i]->key = strdup(CHAR(STRING_ELT(names, i)));
//     if (!result->data.obj_arr[i]->key) {
//       // Clean up previously allocated memory
//       for (size_t j = 0; j < i; j++) {
//         free(result->data.obj_arr[j]->key);
//         free(result->data.obj_arr[j]);
//       }
//       free(result->data.obj_arr);
//       free(result);
//       error("Failed to allocate memory for map key");
//     }
//   }
//   return result;
// }
// 
// 
// Anything* r_to_anything_bool_array(Anything* result, SEXP obj, const Schema* schema) {
//     result->size = 0;
//     result->data.bool_arr = NULL;
// 
//     if (TYPEOF(obj) == LGLSXP) {
//         // Handle logical vector
//         result->size = LENGTH(obj);
//         result->data.bool_arr = (bool*)malloc(result->size * sizeof(bool));
//         if (!result->data.bool_arr) {
//             free(result);
//             error("Failed to allocate memory for boolean array");
//         }
//         for (size_t i = 0; i < result->size; i++) {
//             result->data.bool_arr[i] = LOGICAL(obj)[i] == NA_LOGICAL ? false : LOGICAL(obj)[i];
//         }
//     } else if (TYPEOF(obj) == VECSXP) {
//         // Handle list of logical values
//         result->size = LENGTH(obj);
//         result->data.bool_arr = (bool*)malloc(result->size * sizeof(bool));
//         if (!result->data.bool_arr) {
//             free(result);
//             error("Failed to allocate memory for boolean array");
//         }
//         for (size_t i = 0; i < result->size; i++) {
//             SEXP elem = VECTOR_ELT(obj, i);
//             if (TYPEOF(elem) != LGLSXP || LENGTH(elem) != 1) {
//                 free(result->data.bool_arr);
//                 free(result);
//                 error("Element %zu in list is not a single logical value", i + 1);
//             }
//             result->data.bool_arr[i] = LOGICAL(elem)[0] == NA_LOGICAL ? false : LOGICAL(elem)[0];
//         }
//     } else {
//         free(result);
//         error("Expected logical vector or list of logical values, got %s", type2char(TYPEOF(obj)));
//     }
// 
//     return result;
// }
// 
// 
// Anything* r_to_anything_int_array(Anything* result, SEXP obj, const Schema* schema) {
//     result->size = 0;
//     result->data.int_arr = NULL;
// 
//     if (TYPEOF(obj) == INTSXP || TYPEOF(obj) == REALSXP) {
//         // Handle integer or double vector
//         result->size = LENGTH(obj);
//         result->data.int_arr = (int*)malloc(result->size * sizeof(int));
//         if (!result->data.int_arr) {
//             free(result);
//             error("Failed to allocate memory for integer array");
//         }
//         if (TYPEOF(obj) == INTSXP) {
//             memcpy(result->data.int_arr, INTEGER(obj), result->size * sizeof(int));
//         } else {
//             for (size_t i = 0; i < result->size; i++) {
//                 double val = REAL(obj)[i];
//                 if (val > INT_MAX || val < INT_MIN) {
//                     free(result->data.int_arr);
//                     free(result);
//                     error("Double value %f at index %zu is out of range for integer", val, i);
//                 }
//                 result->data.int_arr[i] = (int)val;
//             }
//         }
//     } else if (TYPEOF(obj) == VECSXP) {
//         // Handle list of integers or doubles
//         result->size = LENGTH(obj);
//         result->data.int_arr = (int*)malloc(result->size * sizeof(int));
//         if (!result->data.int_arr) {
//             free(result);
//             error("Failed to allocate memory for integer array");
//         }
//         for (size_t i = 0; i < result->size; i++) {
//             SEXP elem = VECTOR_ELT(obj, i);
//             if (TYPEOF(elem) == INTSXP && LENGTH(elem) == 1) {
//                 result->data.int_arr[i] = INTEGER(elem)[0];
//             } else if (TYPEOF(elem) == REALSXP && LENGTH(elem) == 1) {
//                 double val = REAL(elem)[0];
//                 if (val > INT_MAX || val < INT_MIN) {
//                     free(result->data.int_arr);
//                     free(result);
//                     error("Double value %f at index %zu is out of range for integer", val, i);
//                 }
//                 result->data.int_arr[i] = (int)val;
//             } else {
//                 free(result->data.int_arr);
//                 free(result);
//                 error("Element %zu in list is not a single integer or double", i + 1);
//             }
//         }
//     } else {
//         free(result);
//         error("Expected integer or double vector, or list of integers or doubles, got %s", type2char(TYPEOF(obj)));
//     }
// 
//     return result;
// }
// 
// 
// Anything* r_to_anything_float_array(Anything* result, SEXP obj, const Schema* schema) {
//     result->size = 0;
//     result->data.float_arr = NULL;
// 
//     if (TYPEOF(obj) == REALSXP) {
//         // Handle double vector
//         result->size = LENGTH(obj);
//         result->data.float_arr = (double*)malloc(result->size * sizeof(double));
//         if (!result->data.float_arr) {
//             free(result);
//             error("Failed to allocate memory for float array");
//         }
//         memcpy(result->data.float_arr, REAL(obj), result->size * sizeof(double));
//     } else if (TYPEOF(obj) == VECSXP) {
//         // Handle list of doubles
//         result->size = LENGTH(obj);
//         result->data.float_arr = (double*)malloc(result->size * sizeof(double));
//         if (!result->data.float_arr) {
//             free(result);
//             error("Failed to allocate memory for float array");
//         }
//         for (size_t i = 0; i < result->size; i++) {
//             SEXP elem = VECTOR_ELT(obj, i);
//             if (TYPEOF(elem) != REALSXP || LENGTH(elem) != 1) {
//                 free(result->data.float_arr);
//                 free(result);
//                 error("Element %zu in list is not a single double", i + 1);
//             }
//             result->data.float_arr[i] = REAL(elem)[0];
//         }
//     } else {
//         free(result);
//         error("Expected double vector or list of doubles, got %s", type2char(TYPEOF(obj)));
//     }
// 
//     return result;
// }
// 
// 
// 
// Anything* r_to_anything_ext(Anything* result, SEXP obj, const Schema* schema) {
//   if (TYPEOF(obj) != RAWSXP) {
//     free(result);
//     error("Expected raw vector for EXT type, got %s", type2char(TYPEOF(obj)));
//   }
// 
//   result->size = LENGTH(obj);
//   result->data.char_arr = (char*)malloc(result->size);
//   if (!result->data.char_arr) {
//     free(result);
//     error("Failed to allocate memory for EXT data");
//   }
//   memcpy(result->data.char_arr, RAW(obj), result->size);
// 
//   return result;
// }







#define HANDLE_SINT_TYPE(CTYPE, MIN, MAX) \
    do { \
        if (!(isInteger(obj) || isReal(obj))) { \
            error("Expected integer for %s, but got %s", #CTYPE, type2char(TYPEOF(obj))); \
        } \
        double value = asReal(obj); \
        if (value < MIN || value > MAX) { \
            error("Integer overflow for %s", #CTYPE); \
        } \
        *(CTYPE*)dest = (CTYPE)value; \
    } while(0)

#define HANDLE_UINT_TYPE(CTYPE, MAX) \
    do { \
        if (!(isInteger(obj) || isReal(obj))) { \
            error("Expected integer for %s, but got %s", #CTYPE, type2char(TYPEOF(obj))); \
        } \
        double value = asReal(obj); \
        if (value < 0 || value > MAX) { \
            error("Integer overflow for %s", #CTYPE); \
        } \
        *(CTYPE*)dest = (CTYPE)value; \
    } while(0)

void* to_voidstar(void* dest, SEXP obj, const Schema* schema) {
    if(!dest){
      dest = get_ptr(schema);
    }

    switch (schema->type) {
        case MORLOC_NIL:
            if (obj != R_NilValue) {
                error("Expected NULL for MORLOC_NIL, but got %s", type2char(TYPEOF(obj)));
            }
            *((int8_t*)dest) = (int8_t)0;
            break;
        case MORLOC_BOOL:
            if (!isLogical(obj)) {
                error("Expected logical for MORLOC_BOOL, but got %s", type2char(TYPEOF(obj)));
            }
            *((bool*)dest) = (LOGICAL(obj)[0] == TRUE);
            break;
        case MORLOC_SINT8:
            HANDLE_SINT_TYPE(int8_t, INT8_MIN, INT8_MAX);
            break;
        case MORLOC_SINT16:
            HANDLE_SINT_TYPE(int16_t, INT16_MIN, INT16_MAX);
            break;
        case MORLOC_SINT32:
            HANDLE_SINT_TYPE(int32_t, INT32_MIN, INT32_MAX);
            break;
        case MORLOC_SINT64:
            HANDLE_SINT_TYPE(int64_t, INT64_MIN, INT64_MAX);
            break;
        case MORLOC_UINT8:
            HANDLE_UINT_TYPE(uint8_t, UINT8_MAX);
            break;
        case MORLOC_UINT16:
            HANDLE_UINT_TYPE(uint16_t, UINT16_MAX);
            break;
        case MORLOC_UINT32:
            HANDLE_UINT_TYPE(uint32_t, UINT32_MAX);
            break;
        case MORLOC_UINT64:
            HANDLE_UINT_TYPE(uint64_t, UINT64_MAX);
            break;
        case MORLOC_FLOAT32:
            if (!(isReal(obj) || isInteger(obj))) {
                error("Expected numeric for MORLOC_FLOAT32, but got %s", type2char(TYPEOF(obj)));
            }
            *((float*)dest) = (float)asReal(obj);
            break;

        case MORLOC_FLOAT64:
            if (!(isReal(obj) || isInteger(obj))) {
                error("Expected numeric for MORLOC_FLOAT64, but got %s", type2char(TYPEOF(obj)));
            }
            *((double*)dest) = asReal(obj);
            break;
        case MORLOC_STRING:
        case MORLOC_ARRAY:
            Array* array = array_data(dest, schema->parameters[0]->width, LENGTH(obj));
            int length = LENGTH(obj);
          
            switch (TYPEOF(obj)) {
                case VECSXP:  // This handles lists
                    for (int i = 0; i < length; i++) {
                        SEXP elem = VECTOR_ELT(obj, i);
                        void* element_ptr = (char*)array->data + i * schema->parameters[0]->width;
                        to_voidstar(element_ptr, elem, schema->parameters[0]);
                    }
                    break;
                case LGLSXP:
                    for (int i = 0; i < length; i++) {
                        SEXP elem = PROTECT(ScalarLogical(LOGICAL(obj)[i]));
                        void* element_ptr = (char*)array->data + i * schema->parameters[0]->width;
                        to_voidstar(element_ptr, elem, schema->parameters[0]);
                        UNPROTECT(1);
                    }
                    break;
                case INTSXP:
                    for (int i = 0; i < length; i++) {
                        SEXP elem = PROTECT(ScalarInteger(INTEGER(obj)[i]));
                        void* element_ptr = (char*)array->data + i * schema->parameters[0]->width;
                        to_voidstar(element_ptr, elem, schema->parameters[0]);
                        UNPROTECT(1);
                    }
                    break;
                case REALSXP:
                    for (int i = 0; i < length; i++) {
                        SEXP elem = PROTECT(ScalarReal(REAL(obj)[i]));
                        void* element_ptr = (char*)array->data + i * schema->parameters[0]->width;
                        to_voidstar(element_ptr, elem, schema->parameters[0]);
                        UNPROTECT(1);
                    }
                    break;
                case CHARSXP:
                    const char* str = CHAR(obj);
                    size_t str_len = strlen(str);  // Do not include null terminator
                    array->size = str_len;
                    memcpy(array->data, str, str_len);
                    break;
                case STRSXP:
                    if (length == 1) {
                        SEXP elem = STRING_ELT(obj, 0);
                        const char* str = CHAR(elem);
                        size_t str_len = strlen(str);  // Do not include null terminator
                        array->size = str_len;
                        memcpy(array->data, str, str_len);
                    } else {
                        if(schema->parameters[0]->type == MORLOC_STRING){
                            Schema* element_schema = schema->parameters[0];
                            for(size_t i = 0; i < array->size; i++){
                                SEXP elem = STRING_ELT(obj, i);
                                to_voidstar(array->data + i * element_schema->width, elem, element_schema); 
                            }
                        } else {
                            error("Expected character vector of length 1, but got length %d", length);
                        }
                    }
                    break;
                case RAWSXP:  // Raw vectors
                    for (int i = 0; i < length; i++) {
                        SEXP elem = PROTECT(ScalarRaw(RAW(obj)[i]));
                        void* element_ptr = (char*)array->data + i * schema->parameters[0]->width;
                        to_voidstar(element_ptr, elem, schema->parameters[0]);
                        UNPROTECT(1);
                    }
                    break;
                default:
                    error("Unsupported type in to_voidstar: %s", type2char(TYPEOF(obj)));
            }
            break;



        /* case MORLOC_TUPLE:                                                                                                   */
        /*     if (!isVectorList(obj)) {                                                                                        */
        /*         error("Expected list for MORLOC_TUPLE, but got %s", type2char(TYPEOF(obj)));                                 */
        /*     }                                                                                                                */

        /*     {                                                                                                                */
        /*         R_xlen_t size = xlength(obj);                                                                                */
        /*         if ((size_t)size != schema->size) {                                                                          */
        /*             error("List size mismatch");                                                                             */
        /*         }                                                                                                            */
        /*         for (R_xlen_t i = 0; i < size; ++i) {                                                                        */
        /*             SEXP item = VECTOR_ELT(obj, i);                                                                          */
        /*             r_to_anything(item, schema->parameters[i]);                                                              */
        /*             memcpy((char*)dest + schema->offsets[i], schema->parameters[i]->data, schema->parameters[i]->width);     */
        /*         }                                                                                                            */
        /*     }                                                                                                                */
        /*     break;                                                                                                           */

        /* case MORLOC_MAP:                                                                                                     */
        /*     if (!isEnvironment(obj)) {                                                                                       */
        /*         error("Expected environment for MORLOC_MAP, but got %s", type2char(TYPEOF(obj)));                            */
        /*     }                                                                                                                */

        /*     {                                                                                                                */
        /*         for (size_t i = 0; i < schema->size; ++i) {                                                                  */
        /*             SEXP key = mkString(schema->keys[i]);                                                                    */
        /*             SEXP value = findVar(installChar(key), obj);                                                             */
        /*             if (value != R_UnboundValue) {                                                                           */
        /*                 r_to_anything(value, schema->parameters[i]);                                                         */
        /*                 memcpy((char*)dest + schema->offsets[i], schema->parameters[i]->data, schema->parameters[i]->width); */
        /*             }                                                                                                        */
        /*         }                                                                                                            */
        /*     }                                                                                                                */

        default:
            error("Unhandled schema type");
            break;
    }

    return dest;
}


// // Helper function to convert Anything to R object
// SEXP anything_to_r(const void* data, const Schema* schema) {
//   SEXP result;
//   switch(schema->type){
//     case MORLOC_NIL:
//       return R_NilValue;
//     case MORLOC_BOOL:
//       return ScalarLogical(data->data.bool_val);
//     case MORLOC_INT:
//       return ScalarInteger(data->data.int_val);
//     case MORLOC_FLOAT:
//       return ScalarReal(data->data.double_val);
//     case MORLOC_STRING:
//       return mkString(data->data.char_arr);
//     case MORLOC_BINARY:
//       {
//         result = PROTECT(allocVector(RAWSXP, data->size));
//         memcpy(RAW(result), data->data.char_arr, data->size);
//         UNPROTECT(1);
//       }
//       break;
// 
//     case MORLOC_ARRAY:
//       {
//           switch(schema->parameters[0]->type) {
//               case MORLOC_BOOL:
//               {
//                   result = PROTECT(allocVector(LGLSXP, data->size));
//                   for (size_t i = 0; i < data->size; i++) {
//                       LOGICAL(result)[i] = data->data.obj_arr[i]->data.bool_val ? TRUE : FALSE;
//                   }
//                   UNPROTECT(1);
//               }
//               break;
//     
//               case MORLOC_INT:
//               {
//                   result = PROTECT(allocVector(INTSXP, data->size));
//                   for (size_t i = 0; i < data->size; i++) {
//                       INTEGER(result)[i] = data->data.obj_arr[i]->data.int_val;
//                   }
//                   UNPROTECT(1);
//               }
//               break;
//     
//               case MORLOC_FLOAT:
//               {
//                   result = PROTECT(allocVector(REALSXP, data->size));
//                   for (size_t i = 0; i < data->size; i++) {
//                       REAL(result)[i] = data->data.obj_arr[i]->data.double_val;
//                   }
//                   UNPROTECT(1);
//               }
//               break;
//     
//               case MORLOC_STRING:
//               {
//                   result = PROTECT(allocVector(STRSXP, data->size));
//                   for (size_t i = 0; i < data->size; i++) {
//                       SET_STRING_ELT(result, i, mkChar(data->data.obj_arr[i]->data.char_arr));
//                   }
//                   UNPROTECT(1);
//               }
//               break;
//     
//               default:
//               {
//                   result = PROTECT(allocVector(VECSXP, data->size));
//                   const Schema* element_schema = schema->parameters[0];
//                   for (size_t i = 0; i < data->size; i++) {
//                       SET_VECTOR_ELT(result, i, anything_to_r(data->data.obj_arr[i], element_schema));
//                   }
//                   UNPROTECT(1);
//               }
//               break;
//           }
//       }
//       break;
// 
//     case MORLOC_TUPLE:
//       {
//         result = PROTECT(allocVector(VECSXP, data->size));
//         for (size_t i = 0; i < data->size; i++) {
//           SET_VECTOR_ELT(result, i, anything_to_r(data->data.obj_arr[i], schema->parameters[i]));
//         }
//         UNPROTECT(1);
//       }
//       break;
//     case MORLOC_MAP:
//       {
//         result = PROTECT(allocVector(VECSXP, data->size));
//         SEXP names = PROTECT(allocVector(STRSXP, data->size));
//         for (size_t i = 0; i < data->size; i++) {
//           SET_VECTOR_ELT(result, i, anything_to_r(data->data.obj_arr[i], schema->parameters[i]));
//           SET_STRING_ELT(names, i, mkChar(data->data.obj_arr[i]->key));
//         }
//         setAttrib(result, R_NamesSymbol, names);
//         UNPROTECT(2);
//       }
//       break;
//     case MORLOC_BOOL_ARRAY:
//       {
//         result = PROTECT(allocVector(LGLSXP, data->size));
//         for (size_t i = 0; i < data->size; i++) {
//           LOGICAL(result)[i] = data->data.bool_arr[i];
//         }
//         UNPROTECT(1);
//       }
//       break;
//     case MORLOC_INT_ARRAY:
//       {
//         result = PROTECT(allocVector(INTSXP, data->size));
//         memcpy(INTEGER(result), data->data.int_arr, data->size * sizeof(int));
//         UNPROTECT(1);
//       }
//       break;
//     case MORLOC_FLOAT_ARRAY:
//       {
//         result = PROTECT(allocVector(REALSXP, data->size));
//         memcpy(REAL(result), data->data.float_arr, data->size * sizeof(double));
//         UNPROTECT(1);
//       }
//       break;
//     case MORLOC_EXT:
//       // Assuming EXT is handled as raw bytes in R
//       {
//         result = PROTECT(allocVector(RAWSXP, data->size));
//         memcpy(RAW(result), data->data.char_arr, data->size);
//         UNPROTECT(1);
//       }
//       break;
//     default:
//       return R_NilValue;
//       error("Unsupported Anything type");
//   }
//   return result;
// }


SEXP from_voidstar(const void* data, const Schema* schema) {
    SEXP obj = R_NilValue;
    switch (schema->type) {
        case MORLOC_NIL:
            return R_NilValue;
        case MORLOC_BOOL:
            obj = ScalarLogical(*(bool*)data);
            break;
        case MORLOC_SINT8:
            obj = ScalarInteger((int)(*(int8_t*)data));
            break;
        case MORLOC_SINT16:
            obj = ScalarInteger((int)(*(int16_t*)data));
            break;
        case MORLOC_SINT32:
            obj = ScalarInteger(*(int32_t*)data);
            break;
        case MORLOC_SINT64:
            obj = ScalarReal((double)(*(int64_t*)data));
            break;
        case MORLOC_UINT8:
            obj = ScalarInteger((int)(*(uint8_t*)data));
            break;
        case MORLOC_UINT16:
            obj = ScalarInteger((int)(*(uint16_t*)data));
            break;
        case MORLOC_UINT32:
            obj = ScalarReal((double)(*(uint32_t*)data));
            break;
        case MORLOC_UINT64:
            obj = ScalarReal((double)(*(uint64_t*)data));
            break;
        case MORLOC_FLOAT32:
            obj = ScalarReal((double)(*(float*)data));
            break;
        case MORLOC_FLOAT64:
            obj = ScalarReal(*(double*)data);
            break;
        case MORLOC_STRING: {
            Array* str_array = (Array*)data;
            SEXP chr = PROTECT(mkCharLen(str_array->data, str_array->size));
            obj = PROTECT(ScalarString(chr));
            UNPROTECT(2);
            break;
        }
        case MORLOC_ARRAY:
            {
                Array* array = (Array*)data;
                Schema* element_schema = schema->parameters[0];
                
                switch(element_schema->type){
                    case MORLOC_BOOL:
                        obj = PROTECT(allocVector(LGLSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            LOGICAL(obj)[i] = *(bool*)((char*)array->data + i * sizeof(bool)) ? TRUE : FALSE;
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_SINT8:
                        obj = PROTECT(allocVector(INTSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            INTEGER(obj)[i] = (int)(*(int8_t*)((char*)array->data + i * sizeof(int8_t)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_SINT16:
                        obj = PROTECT(allocVector(INTSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            INTEGER(obj)[i] = (int)(*(int16_t*)((char*)array->data + i * sizeof(int16_t)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_SINT32:
                        obj = PROTECT(allocVector(INTSXP, array->size));
                        memcpy(INTEGER(obj), array->data, array->size * sizeof(int32_t));
                        UNPROTECT(1);
                        break;
                    case MORLOC_SINT64:
                        obj = PROTECT(allocVector(REALSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            REAL(obj)[i] = (double)(*(int64_t*)((char*)array->data + i * sizeof(int64_t)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_UINT8:
                        obj = PROTECT(allocVector(INTSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            INTEGER(obj)[i] = (int)(*(uint8_t*)((char*)array->data + i * sizeof(uint8_t)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_UINT16:
                        obj = PROTECT(allocVector(INTSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            INTEGER(obj)[i] = (int)(*(uint16_t*)((char*)array->data + i * sizeof(uint16_t)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_UINT32:
                        obj = PROTECT(allocVector(REALSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            REAL(obj)[i] = (double)(*(uint32_t*)((char*)array->data + i * sizeof(uint32_t)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_UINT64:
                        obj = PROTECT(allocVector(REALSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            REAL(obj)[i] = (double)(*(uint64_t*)((char*)array->data + i * sizeof(uint64_t)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_FLOAT32:
                        obj = PROTECT(allocVector(REALSXP, array->size));
                        for (size_t i = 0; i < array->size; i++) {
                            REAL(obj)[i] = (double)(*(float*)((char*)array->data + i * sizeof(float)));
                        }
                        UNPROTECT(1);
                        break;
                    case MORLOC_FLOAT64:
                        obj = PROTECT(allocVector(REALSXP, array->size));
                        memcpy(REAL(obj), array->data, array->size * sizeof(double));
                        UNPROTECT(1);
                        break;
                    case MORLOC_STRING:
                        {
                            obj = PROTECT(allocVector(STRSXP, array->size));
                            char* start = (char*)array->data;
                            size_t width = schema->width;
                            for (size_t i = 0; i < array->size; i++) {
                                Array* str_array = (Array*)(start + i * width);
                                SEXP item = PROTECT(mkCharLen(str_array->data, str_array->size));
                                UNPROTECT(1);
                                SET_STRING_ELT(obj, i, item);
                            }
                            UNPROTECT(1);
                        }
                        break;
                    default:
                        {
                            obj = allocVector(VECSXP, array->size);
                            char* start = (char*)array->data;
                            size_t width = element_schema->width;
                            for (size_t i = 0; i < array->size; i++) {
                                SEXP item = from_voidstar(start + width * i, element_schema);
                                if (item == R_NilValue) {
                                    obj = R_NilValue;
                                    goto error;
                                }
                                SET_VECTOR_ELT(obj, i, item);
                            }
                        }
                        break;
                }
            }
            break;
        /* case MORLOC_TUPLE: {                                                      */
        /*     obj = allocVector(VECSXP, schema->size);                              */
        /*     for (size_t i = 0; i < schema->size; i++) {                           */
        /*         void* item_ptr = (char*)data + schema->offsets[i];                */
        /*         SEXP item = anything_to_r(schema->parameters[i], item_ptr);       */
        /*         if (item == R_NilValue) {                                         */
        /*             obj = R_NilValue;                                             */
        /*             goto error;                                                   */
        /*         }                                                                 */
        /*         SET_VECTOR_ELT(obj, i, item);                                     */
        /*     }                                                                     */
        /*     break;                                                                */
        /* }                                                                         */
        /* case MORLOC_MAP: {                                                        */
        /*     obj = allocVector(VECSXP, schema->size);                              */
        /*     SEXP names = allocVector(STRSXP, schema->size);                       */
        /*     for (size_t i = 0; i < schema->size; i++) {                           */
        /*         void* item_ptr = (char*)data + schema->offsets[i];                */
        /*         SEXP value = anything_to_r(schema->parameters[i], item_ptr);      */
        /*         if (value == R_NilValue) {                                        */
        /*             obj = R_NilValue;                                             */
        /*             goto error;                                                   */
        /*         }                                                                 */
        /*         SET_VECTOR_ELT(obj, i, value);                                    */
        /*         SET_STRING_ELT(names, i, mkChar(schema->keys[i]));                */
        /*     }                                                                     */
        /*     setAttrib(obj, R_NamesSymbol, names);                                 */
        /*     break;                                                                */
        /* }                                                                         */
        default:
            error("Unsupported schema type");
            goto error;
    }

    return obj;

error:
    return R_NilValue;
}








SEXP to_mesgpack(SEXP r_obj, SEXP r_schema_str) {
    PROTECT(r_obj);
    PROTECT(r_schema_str);

    const char* schema_str = CHAR(STRING_ELT(r_schema_str, 0));
    Schema* schema = parse_schema(&schema_str);

    if (!schema) {
        UNPROTECT(2);
        error("Failed to parse schema");
    }

    void* data = to_voidstar(NULL, r_obj, schema);

    if (!data) {
        free_schema(schema);
        UNPROTECT(2);
        error("Failed to convert R object to Anything");
    }

    char* packed_data = NULL;
    size_t packed_size = 0;
    int result = pack_with_schema(data, schema, &packed_data, &packed_size);

    if (result != 0 || !packed_data) {
        free_schema(schema);
        UNPROTECT(2);
        error("Packing failed");
    }

    SEXP r_packed = PROTECT(allocVector(RAWSXP, packed_size));
    memcpy(RAW(r_packed), packed_data, packed_size);

    // Clean up
    free(packed_data);
    free_schema(schema);

    UNPROTECT(3);
    return r_packed;
}





// R-callable function to unpack to R object
SEXP from_mesgpack(SEXP r_packed, SEXP r_schema_str) {
    PROTECT(r_packed);
    PROTECT(r_schema_str);
    
    const char* schema_str = CHAR(STRING_ELT(r_schema_str, 0));
    Schema* schema = parse_schema(&schema_str);
    if (!schema) {
        UNPROTECT(2);
        error("Failed to parse schema");
    }

    const char* packed_data = (const char*)RAW(r_packed);
    size_t packed_size = LENGTH(r_packed);

    void* unpacked_data = NULL;
    int result = unpack_with_schema(packed_data, packed_size, schema, &unpacked_data);

    if (result != 0 || !unpacked_data) {
        free_schema(schema);
        UNPROTECT(2);
        error("Unpacking failed");
    }

    SEXP r_unpacked = PROTECT(from_voidstar(unpacked_data, schema));
    
    // Assuming unpack_with_schema allocates memory for unpacked_data
    free(unpacked_data);
    free_schema(schema);

    UNPROTECT(3);
    return r_unpacked;
}


void R_init_mlcmpack(DllInfo *info) {
    R_CallMethodDef callMethods[] = {
        {"to_voidstar", (DL_FUNC) &to_voidstar, 2},
        {"from_voidstar", (DL_FUNC) &from_voidstar, 2},
        {"to_mesgpack", (DL_FUNC) &to_mesgpack, 2},
        {"from_mesgpack", (DL_FUNC) &from_mesgpack, 2},
        {NULL, NULL, 0}
    };

    R_registerRoutines(info, NULL, callMethods, NULL, NULL);
    R_useDynamicSymbols(info, FALSE);
}
