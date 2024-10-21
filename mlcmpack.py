#!/usr/bin/env python

import ctypes
from enum import IntEnum
from typing import Union, List, Dict, Tuple
import json

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

# Define the Schema structure
class Schema(ctypes.Structure):
    pass

Schema._fields_ = [
    ("type", ctypes.c_int),  # Using c_int for the enum
    ("size", ctypes.c_size_t),
    ("parameters", ctypes.POINTER(ctypes.POINTER(Schema))),
    ("keys", ctypes.POINTER(ctypes.c_char_p))
]

class SchemaWrapper:
    def __init__(self, schema_type, size=0, parameters=None, keys=None):
        self.schema = Schema()
        self.schema.type = schema_type
        self.schema.size = size

        if parameters:
            self.schema.parameters = (ctypes.POINTER(Schema) * len(parameters))()
            for i, param in enumerate(parameters):
                self.schema.parameters[i] = ctypes.pointer(param.schema)
        else:
            self.schema.parameters = None

        if keys:
            self.schema.keys = (ctypes.c_char_p * len(keys))()
            for i, key in enumerate(keys):
                self.schema.keys[i] = key.encode('utf-8')
        else:
            self.schema.keys = None

    @classmethod
    def from_c_schema(cls, c_schema):
        schema_type = MorlocSerialType(c_schema.type)
        size = c_schema.size

        parameters = None
        if c_schema.parameters:
            parameters = [cls.from_c_schema(c_schema.parameters[i].contents) for i in range(size)]

        keys = None
        if c_schema.keys:
            keys = [c_schema.keys[i].decode('utf-8') for i in range(size)]

        return cls(schema_type, size, parameters, keys)

    def to_c_schema(self):
        return self.schema

    def __str__(self):
        return self._str_helper()

    def _str_helper(self, indent=0):
        result = "  " * indent + f"Type: {MorlocSerialType(self.schema.type).name}\n"
        if self.schema.size > 0:
            result += "  " * indent + f"Size: {self.schema.size}\n"
            if self.schema.parameters:
                result += "  " * indent + "Parameters:\n"
                for i in range(self.schema.size):
                    param = SchemaWrapper.from_c_schema(self.schema.parameters[i].contents)
                    result += param._str_helper(indent + 1)
            if self.schema.keys:
                result += "  " * indent + "Keys:\n"
                for i in range(self.schema.size):
                    key = self.schema.keys[i].decode('utf-8')
                    result += "  " * (indent + 1) + f"{key}\n"
        return result

def schema_nil() -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_NIL)

def schema_bool() -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_BOOL)

def schema_int() -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_INT)

def schema_float() -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_FLOAT)

def schema_string() -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_STRING)

def schema_binary() -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_BINARY)

def schema_array(element_schema: SchemaWrapper, size: int = 0) -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_ARRAY, size=size, parameters=[element_schema])

def schema_bool_array(size: int = 0) -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_BOOL_ARRAY, size=size)

def schema_int_array(size: int = 0) -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_INT_ARRAY, size=size)

def schema_float_array(size: int = 0) -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_FLOAT_ARRAY, size=size)

def schema_tuple(*element_schemas: SchemaWrapper) -> SchemaWrapper:
    return SchemaWrapper(MorlocSerialType.MORLOC_TUPLE, size=len(element_schemas), parameters=list(element_schemas))

def schema_map(key_value_schemas: Dict[str, SchemaWrapper]) -> SchemaWrapper:
    keys = list(key_value_schemas.keys())
    values = list(key_value_schemas.values())
    return SchemaWrapper(MorlocSerialType.MORLOC_MAP, size=len(keys), parameters=values, keys=keys)




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
    pd.key = key.encode('utf-8') if key else None

       # If schema is a SchemaWrapper, get the underlying Schema
    if isinstance(schema, SchemaWrapper):
        schema = schema.schema

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
        for i, (k, v) in enumerate(data.items()):
            v_schema = schema.parameters[k] if isinstance(schema.parameters, dict) else schema.parameters[1]
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

def create_read_only_parsed_data(data, schema):
    pd = python_to_parsed_data(data, schema, key = "")
    return ctypes.pointer(pd)

# Example usage
if __name__ == "__main__":

    schema = schema_array(schema_tuple(schema_bool(), schema_int_array()))

    # Create schema using helper functions
    schema = schema_map({
        "null_value": schema_nil(),
        "boolean": schema_bool(),
        "integer": schema_int(),
        "float": schema_float(),
        "string": schema_string(),
        "binary": schema_binary(),
        "binary_string": schema_string(),
        "array": schema_int_array(100),
        "nested_map": schema_map({"a": schema_int(), "b": schema_int()}),
        "tuple": schema_tuple(schema_int(), schema_int(), schema_int())
    })

    data = {
        "null_value": None,
        "boolean": True,
        "integer": 42,
        "float": 3.14,
        "string": "Hello, World!",
        "binary": b'\x00\x01\x02\x03',
        "binary": "1234",
        "array": list(range(100)),
        "nested_map": {"a": 1, "b": 2},
        "tuple": (4, 5, 6),
    }

    for j in range(50000):
        parsed = json.dumps(data)
        result = json.loads(parsed)
        #  parsed = create_read_only_parsed_data(data, schema)
        #  result = parsed_data_to_python(parsed.contents)

    print(f"Original: {data}")
    print(f"Roundtrip: {result}")

    # Attempt to modify ParsedData (this will raise an error)
    try:
        parsed.contents.data.int_val = 10
    except AttributeError as e:
        print(f"Modification attempt failed: {e}")
