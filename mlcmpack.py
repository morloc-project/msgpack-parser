#!/usr/bin/env python

import ctypes
from enum import IntEnum
from typing import Union, List, Dict, Tuple
import json

# Define the Packet structure to match the C structure
class Packet(ctypes.Structure):
    _fields_ = [
        ("data", ctypes.c_char_p),
        ("size", ctypes.c_size_t)
    ]

class MorlocSerialType(IntEnum):
    MORLOC_NIL = 0
    MORLOC_BOOL = 1
    MORLOC_INT = 2
    MORLOC_FLOAT = 3
    MORLOC_STRING = 4
    MORLOC_BINARY = 5
    MORLOC_ARRAY = 6
    MORLOC_MAP = 7
    MORLOC_TUPLE = 8
    MORLOC_BOOL_ARRAY = 9
    MORLOC_INT_ARRAY = 10
    MORLOC_FLOAT_ARRAY = 11
    MORLOC_EXT = 12

class Schema(ctypes.Structure):
    pass

Schema._fields_ = [
    ("type", ctypes.c_int),
    ("size", ctypes.c_size_t),
    ("parameters", ctypes.POINTER(ctypes.POINTER(Schema))),
    ("keys", ctypes.POINTER(ctypes.c_char_p))
]

def python_schema_to_c(py_schema):
    c_schema = Schema()
    c_schema.type = py_schema[0]
    
    if len(py_schema) > 1:
        if isinstance(py_schema[1], list):
            c_schema.size = len(py_schema[1])
            c_schema.parameters = (ctypes.POINTER(Schema) * c_schema.size)()
            for i, param in enumerate(py_schema[1]):
                param_schema = python_schema_to_c(param)
                c_schema.parameters[i] = ctypes.pointer(param_schema)
        elif isinstance(py_schema[1], dict):
            c_schema.size = len(py_schema[1])
            c_schema.parameters = (ctypes.POINTER(Schema) * c_schema.size)()
            c_schema.keys = (ctypes.c_char_p * c_schema.size)()
            for i, (key, value) in enumerate(py_schema[1].items()):
                param_schema = python_schema_to_c(value)
                c_schema.parameters[i] = ctypes.pointer(param_schema)
                c_schema.keys[i] = key.encode('utf-8')
    else:
        c_schema.size = 0
        c_schema.parameters = None
        c_schema.keys = None
    
    return c_schema

# Wrapper functions for easily building schema objects
def schema_nil():
    return [MorlocSerialType.MORLOC_NIL]

def schema_bool():
    return [MorlocSerialType.MORLOC_BOOL]

def schema_int():
    return [MorlocSerialType.MORLOC_INT]

def schema_float():
    return [MorlocSerialType.MORLOC_FLOAT]

def schema_string():
    return [MorlocSerialType.MORLOC_STRING]

def schema_binary():
    return [MorlocSerialType.MORLOC_BINARY]

def schema_array(element_schema):
    return [MorlocSerialType.MORLOC_ARRAY, [element_schema]]

def schema_map(key_value_schemas: Dict[str, List]):
    return [MorlocSerialType.MORLOC_MAP, key_value_schemas]

def schema_tuple(*element_schemas):
    return [MorlocSerialType.MORLOC_TUPLE, list(element_schemas)]

def schema_bool_array():
    return [MorlocSerialType.MORLOC_BOOL_ARRAY]

def schema_int_array():
    return [MorlocSerialType.MORLOC_INT_ARRAY]

def schema_float_array():
    return [MorlocSerialType.MORLOC_FLOAT_ARRAY]



class ParsedData(ctypes.Structure):
    pass

ParsedDataPtr = ctypes.POINTER(ParsedData)

class _Data(ctypes.Union):
    _fields_ = [
        ("nil_val", ctypes.c_char),
        ("bool_val", ctypes.c_bool),
        ("int_val", ctypes.c_int),
        ("double_val", ctypes.c_double),
        ("char_arr", ctypes.c_char_p),
        ("bool_arr", ctypes.POINTER(ctypes.c_bool)),
        ("int_arr", ctypes.POINTER(ctypes.c_int)),
        ("float_arr", ctypes.POINTER(ctypes.c_double)),
        ("obj_arr", ctypes.POINTER(ParsedDataPtr))
    ]

ParsedData._fields_ = [
    ("type", ctypes.c_int),
    ("size", ctypes.c_size_t),
    ("key", ctypes.c_char_p),
    ("data", _Data)
]

def python_to_parsed_data(data, schema, key = None) -> ParsedData:
    pd = ParsedData()

    if isinstance(key, str):
        pd.key = key.encode('utf-8')

    # If schema is a pointer, dereference it
    if isinstance(schema, ctypes.POINTER(Schema)):
        schema = schema.contents

    pd.type = schema.type

    if schema.type == MorlocSerialType.MORLOC_NIL:
        pd.size = 0
        pd.data.nil_val = b'\x00'
    elif schema.type == MorlocSerialType.MORLOC_BOOL:
        pd.size = 0
        pd.data.bool_val = data
    elif schema.type == MorlocSerialType.MORLOC_INT:
        pd.size = 0
        pd.data.int_val = data
    elif schema.type == MorlocSerialType.MORLOC_FLOAT:
        pd.size = 0
        pd.data.double_val = data
    elif schema.type == MorlocSerialType.MORLOC_STRING:
        encoded = data.encode('utf-8')
        pd.size = len(encoded)
        pd.data.char_arr = ctypes.c_char_p(encoded)
    elif schema.type == MorlocSerialType.MORLOC_BINARY:
        pd.size = len(data)
        pd.data.char_arr = ctypes.c_char_p(data)
    elif schema.type == MorlocSerialType.MORLOC_ARRAY:
        pd.size = len(data)
        arr = (ParsedDataPtr * pd.size)()
        for i, item in enumerate(data):
            arr[i] = ctypes.pointer(python_to_parsed_data(item, schema.parameters[0]))
        pd.data.obj_arr = arr
    elif schema.type == MorlocSerialType.MORLOC_MAP:
        pd.size = len(data)
        arr = (ParsedDataPtr * pd.size)()
        for (i, (k, v)), v_schema in zip(enumerate(data.items()), schema.parameters):
            arr[i] = ctypes.pointer(python_to_parsed_data(v, v_schema, key=k))
        pd.data.obj_arr = arr
    elif schema.type == MorlocSerialType.MORLOC_TUPLE:
        pd.size = len(data)
        arr = (ParsedDataPtr * pd.size)()
        for i, (item, item_schema) in enumerate(zip(data, schema.parameters)):
            arr[i] = ctypes.pointer(python_to_parsed_data(item, item_schema))
        pd.data.obj_arr = arr
    elif schema.type == MorlocSerialType.MORLOC_BOOL_ARRAY:
        pd.size = len(data)
        arr = (ctypes.c_bool * pd.size)(*data)
        pd.data.bool_arr = arr
    elif schema.type == MorlocSerialType.MORLOC_INT_ARRAY:
        pd.size = len(data)
        arr = (ctypes.c_int * pd.size)(*data)
        pd.data.int_arr = arr
    elif schema.type == MorlocSerialType.MORLOC_FLOAT_ARRAY:
        pd.size = len(data)
        arr = (ctypes.c_double * pd.size)(*data)
        pd.data.float_arr = arr
    elif schema.type == MorlocSerialType.MORLOC_EXT:
        raise NotImplementedError("MORLOC_EXT type is not supported")
    else:
        raise ValueError(f"Unknown schema type: {schema.type}")

    return pd

def parsed_data_to_python(pd: ParsedData) -> Union[None, bool, int, float, str, bytes, List, Dict, Tuple]:
    if pd.type == MorlocSerialType.MORLOC_NIL:
        return None
    elif pd.type == MorlocSerialType.MORLOC_BOOL:
        return pd.data.bool_val
    elif pd.type == MorlocSerialType.MORLOC_INT:
        return pd.data.int_val
    elif pd.type == MorlocSerialType.MORLOC_FLOAT:
        return pd.data.double_val
    elif pd.type == MorlocSerialType.MORLOC_STRING:
        return pd.data.char_arr.decode('utf-8')
    elif pd.type == MorlocSerialType.MORLOC_BINARY:
        return bytes(pd.data.char_arr[:pd.size])
    elif pd.type == MorlocSerialType.MORLOC_ARRAY:
        return [parsed_data_to_python(pd.data.obj_arr[i].contents) for i in range(pd.size)]
    elif pd.type == MorlocSerialType.MORLOC_MAP:
        return {pd.data.obj_arr[i].contents.key.decode('utf-8') if pd.data.obj_arr[i].contents.key else None:
                parsed_data_to_python(pd.data.obj_arr[i].contents) for i in range(pd.size)}
    elif pd.type == MorlocSerialType.MORLOC_TUPLE:
        return tuple(parsed_data_to_python(pd.data.obj_arr[i].contents) for i in range(pd.size))
    elif pd.type == MorlocSerialType.MORLOC_BOOL_ARRAY:
        return [pd.data.bool_arr[i] for i in range(pd.size)]
    elif pd.type == MorlocSerialType.MORLOC_INT_ARRAY:
        return [pd.data.int_arr[i] for i in range(pd.size)]
    elif pd.type == MorlocSerialType.MORLOC_FLOAT_ARRAY:
        return [pd.data.float_arr[i] for i in range(pd.size)]
    elif pd.type == MorlocSerialType.MORLOC_EXT:
        return bytes(pd.data.char_arr[:pd.size])
    else:
        raise ValueError(f"Unknown ParsedData type: {pd.type}")


# Load the shared library
lib = ctypes.CDLL('./mlcmpack.so')

# Define the function signatures for the new wrappers
lib.pack.argtypes = [ctypes.POINTER(ParsedData), ctypes.POINTER(Schema)]
lib.pack.restype = Packet

lib.unpack.argtypes = [Packet, ctypes.POINTER(Schema)]
lib.unpack.restype = ctypes.POINTER(ParsedData)


def pack_data(data: ParsedData, schema: Schema) -> bytes:

    # data and schema are already the correct types (ParsedData and Schema),
    # so we just need to pass pointers to them
    packet = lib.pack(data, schema)
    
    # Copy the data from the Packet
    packed_data = ctypes.string_at(packet.data, packet.size)
    

    return packed_data


def unpack_data(packed_data: bytes, schema: Schema) -> ParsedData:
    packet = Packet(ctypes.c_char_p(packed_data), ctypes.c_size_t(len(packed_data)))
    result = lib.unpack(packet, ctypes.byref(schema))
    if not result:
        raise RuntimeError("Unpacking failed")
    return result.contents


if __name__ == "__main__":

    big_schema = schema_map({
        "null_value": schema_nil(),
        "boolean": schema_bool(),
        "integer": schema_int(),
        "float": schema_float(),
        "string": schema_string(),
        "binary": schema_binary(),
        "binary_string": schema_string(),
        "array": schema_int_array(),
        "nested_map": schema_map({"a": schema_int(), "b": schema_int()}),
        "tuple": schema_tuple(schema_int(), schema_int(), schema_int())
    })

    big_data = {
        "null_value": None,
        "boolean": True,
        "integer": 42,
        "float": 3.14,
        "string": "Hello, World!",
        "binary": b'\x00\x01\x02\x03',
        "binary_string": "a1234",
        "array": list(range(100)),
        "nested_map": {"a": 1, "b": 2},
        "tuple": (4, 5, 6),
    }


    pairs = [
        (schema_bool(), True),
        (schema_int(), 65536),
        (schema_float(), 6.9420),
        (schema_string(), "Marry Jane"),
        (schema_binary(), b"Marry Jane"),
        (schema_bool_array(), [True, False, False]),
        #  (schema_int_array(), [42, 69, 420, 65535, 65536, 65535]),
        (schema_float_array(), [42, 69, 420, 42069]),
        (schema_tuple(schema_bool(), schema_int()), (False, 42069)),
        (schema_tuple(schema_bool(), schema_int()), (False, 42069)),
        (schema_map({"a": schema_bool(), "b": schema_int()}), {"a": True, "b": 42}),
        (big_schema, big_data),
    ]

    for (py_schema, data) in pairs:

        schema = python_schema_to_c(py_schema)

        # Convert Python data to ParsedData
        parsed_data = python_to_parsed_data(data, schema)

        # Pack the data
        packed_data = pack_data(parsed_data, schema)

        # Unpack the data
        unpacked_data = unpack_data(packed_data, schema)

        # Convert ParsedData back to Python
        result = parsed_data_to_python(unpacked_data)

        print(f"Original:  {data}")
        print(f"Roundtrip: {result}")
        print()
