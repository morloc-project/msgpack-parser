#!/usr/bin/env python

import ctypes
import sys
from enum import IntEnum
from typing import Union, List, Dict, Tuple

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

class ParsedData(ctypes.Structure):
    pass

ParsedDataPtr = ctypes.POINTER(ParsedData)

class _Data(ctypes.Union):
    _fields_ = [
        ("nil_val", ctypes.c_char),
        ("bool_val", ctypes.c_bool),
        ("int_val", ctypes.c_int),
        ("double_val", ctypes.c_double),
        ("char_arr", ctypes.POINTER(ctypes.c_char)),
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


def parse_schema_size(schema: str, index: int) -> tuple[int, int]:
    c = schema[index]
    if '0' <= c <= '9':
        size = ord(c) - ord('0')
    elif 'a' <= c <= 'z':
        size = ord(c) - ord('a') + 10
    elif 'A' <= c <= 'Z':
        size = ord(c) - ord('A') + 36
    elif c == '+':
        size = 62
    elif c == '/':
        size = 63
    else:
        raise ValueError(f"Invalid character for size: {c}")
    return size, index + 1

def parse_schema_key(schema: str, index: int) -> tuple[str, int]:
    key_size, index = parse_schema_size(schema, index)
    key = schema[index:index + key_size]
    return key, index + key_size

def parse_schema_r(schema: str, index: int = 0) -> tuple[list, int]:
    if index >= len(schema):
        return [], index

    c = schema[index]
    index += 1

    if c == 'a':  # SCHEMA_ARRAY
        sub_schema, index = parse_schema_r(schema, index)
        return ['a', sub_schema], index
    elif c == 't':  # SCHEMA_TUPLE
        size, index = parse_schema_size(schema, index)
        tuple_schema = ['t', []]
        for _ in range(size):
            sub_schema, index = parse_schema_r(schema, index)
            tuple_schema[1].append(sub_schema)
        return tuple_schema, index
    elif c == 'm':  # SCHEMA_MAP
        size, index = parse_schema_size(schema, index)
        map_schema = ['m', []]
        for _ in range(size):
            key, index = parse_schema_key(schema, index)
            value_schema, index = parse_schema_r(schema, index)
            map_schema[1].append([key, value_schema])
        return map_schema, index
    elif c == 'z':  # SCHEMA_NIL
        return ([c, []], index)
    elif c == 'b':  # SCHEMA_BOOL
        return [c, []], index
    elif c in ['i', 'u']:  # SCHEMA_SINT or SCHEMA_UINT
        _, index = parse_schema_size(schema, index)
        return [c, []], index
    elif c == 'f':  # SCHEMA_FLOAT
        _, index = parse_schema_size(schema, index)
        return ['f', []], index
    elif c == 's':  # SCHEMA_STRING
        return ['s', []], index
    elif c == 'r':  # SCHEMA_BINARY
        return ['r', []], index
    else:
        raise ValueError(f"Unrecognized schema type '{c}'")

def parse_schema(schema: str) -> list:
    result, _ = parse_schema_r(schema)
    return result

def python_to_parsed_data_r(data, schema, key = None) -> ParsedData:
    pd = ParsedData()

    if isinstance(key, str):
        pd.key = key.encode('utf-8')

    if schema[0] == "z":
        pd.size = 0
        pd.data.nil_val = b'\x00'
    elif schema[0] == "b":
        pd.size = 0
        pd.data.bool_val = bool(data)
    elif schema[0] == "i":
        pd.size = 0
        pd.data.int_val = int(data)
    elif schema[0] == "f":
        pd.size = 0
        pd.data.double_val = float(data)
    elif schema[0] == "s":
        encoded = data.encode('utf-8')
        pd.size = len(encoded)
        buffer = ctypes.create_string_buffer(encoded)
        pd.data.char_arr = ctypes.cast(buffer, ctypes.POINTER(ctypes.c_char))
    elif schema[0] == "r":
        pd.size = len(data)
        buffer = ctypes.create_string_buffer(data)
        pd.data.char_arr = ctypes.cast(buffer, ctypes.POINTER(ctypes.c_char))
    elif schema[0] == "a":
        array_schema = schema[1][0]
        if array_schema[0] == "b":
             pd.size = len(data)
             arr = (ctypes.c_bool * pd.size)(*data)
             pd.data.bool_arr = arr
        elif array_schema[0] == "i":
             pd.size = len(data)
             arr = (ctypes.c_int * pd.size)(*data)
             pd.data.int_arr = arr
        elif array_schema[0] == "f":
             pd.size = len(data)
             arr = (ctypes.c_double * pd.size)(*data)
             pd.data.float_arr = arr
        else:
             pd.size = len(data)
             arr = (ParsedDataPtr * pd.size)()
             for i, item in enumerate(data):
                 arr[i] = ctypes.pointer(python_to_parsed_data_r(item, array_schema))
             pd.data.obj_arr = arr
    elif schema[0] == "t":
        tuple_schemata = schema[1]
        pd.size = len(tuple_schemata)
        arr = (ParsedDataPtr * pd.size)()
        for i, (item, element_schema) in enumerate(zip(data, tuple_schemata)):
            arr[i] = ctypes.pointer(python_to_parsed_data_r(item, element_schema))
        pd.data.obj_arr = arr
    elif schema[0] == "m":
        kwargs_schemata = schema[1]
        pd.size = len(kwargs_schemata)
        arr = (ParsedDataPtr * pd.size)()
        for (i, (k, s)) in enumerate(kwargs_schemata):
            arr[i] = ctypes.pointer(python_to_parsed_data_r(data[k], s, key=k))
        pd.data.obj_arr = arr
    else:
        print(f"Bad schema: {str(schema)}", file=sys.stderr)
    return pd

def python_to_parsed_data(data, schema: str) -> ParsedData:
    return python_to_parsed_data_r(data, parse_schema(schema), key = None)

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
        return pd.data.char_arr[:pd.size].decode('utf-8')
    elif pd.type == MorlocSerialType.MORLOC_BINARY:
        return pd.data.char_arr[:pd.size]
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

# Update the function signatures
lib.pack.argtypes = [ctypes.POINTER(ParsedData), ctypes.c_char_p, ctypes.POINTER(ctypes.POINTER(ctypes.c_char)), ctypes.POINTER(ctypes.c_size_t)]
lib.pack.restype = ctypes.c_int

lib.unpack.argtypes = [ctypes.POINTER(ctypes.c_char), ctypes.c_size_t, ctypes.c_char_p, ctypes.POINTER(ctypes.POINTER(ParsedData))]
lib.unpack.restype = ctypes.c_int

def pack_data(data: ParsedData, schema: str) -> bytes:
    out_data = ctypes.POINTER(ctypes.c_char)()
    out_size = ctypes.c_size_t()
    
    # Convert Python string to bytes, then to c_char_p
    schema_bytes = schema.encode('utf-8')
    schema_c_str = ctypes.c_char_p(schema_bytes)
    
    result = lib.pack(ctypes.byref(data), schema_c_str, ctypes.byref(out_data), ctypes.byref(out_size))
    if result != 0:
        raise RuntimeError("Packing failed")

    packed_data = ctypes.string_at(out_data, out_size.value)

    return packed_data


def unpack_data(packed_data: bytes, schema: str) -> ParsedData:
    data_ptr = ctypes.cast(packed_data, ctypes.POINTER(ctypes.c_char))
    out_data = ctypes.POINTER(ParsedData)()

    # Convert Python string to bytes, then to c_char_p
    schema_bytes = schema.encode('utf-8')
    schema_c_str = ctypes.c_char_p(schema_bytes)

    result = lib.unpack(data_ptr, len(packed_data), schema_c_str, ctypes.byref(out_data))
    if result != 0:
        raise RuntimeError("Unpacking failed")

    # Create a deep copy of the ParsedData
    parsed_data = copy_parsed_data(out_data.contents)

    return parsed_data


def copy_parsed_data(data: ParsedData) -> ParsedData:
    new_data = ParsedData()
    new_data.type = data.type
    new_data.size = data.size
    new_data.key = ctypes.c_char_p(data.key) if data.key else None

    if data.type == MorlocSerialType.MORLOC_NIL:
        new_data.data.nil_val = data.data.nil_val
    elif data.type == MorlocSerialType.MORLOC_BOOL:
        new_data.data.bool_val = data.data.bool_val
    elif data.type == MorlocSerialType.MORLOC_INT:
        new_data.data.int_val = data.data.int_val
    elif data.type == MorlocSerialType.MORLOC_FLOAT:
        new_data.data.double_val = data.data.double_val
    elif data.type == MorlocSerialType.MORLOC_STRING:
        new_data.size = data.size  # Ensure the size is copied
        new_buffer = (ctypes.c_char * data.size)()
        ctypes.memmove(new_buffer, data.data.char_arr, data.size)
        new_data.data.char_arr = ctypes.cast(new_buffer, ctypes.POINTER(ctypes.c_char))
    elif data.type == MorlocSerialType.MORLOC_BINARY:
        new_data.size = data.size  # Ensure the size is copied
        new_buffer = (ctypes.c_char * data.size)()
        ctypes.memmove(new_buffer, data.data.char_arr, data.size)
        new_data.data.char_arr = ctypes.cast(new_buffer, ctypes.POINTER(ctypes.c_char))
    elif data.type in [MorlocSerialType.MORLOC_ARRAY, MorlocSerialType.MORLOC_MAP, MorlocSerialType.MORLOC_TUPLE]:
        new_arr = (ParsedDataPtr * data.size)()
        for i in range(data.size):
            new_arr[i] = ctypes.pointer(copy_parsed_data(data.data.obj_arr[i].contents))
        new_data.data.obj_arr = new_arr
    elif data.type == MorlocSerialType.MORLOC_BOOL_ARRAY:
        new_arr = (ctypes.c_bool * data.size)()
        ctypes.memmove(new_arr, data.data.bool_arr, data.size * ctypes.sizeof(ctypes.c_bool))
        new_data.data.bool_arr = new_arr
    elif data.type == MorlocSerialType.MORLOC_INT_ARRAY:
        new_arr = (ctypes.c_int * data.size)()
        ctypes.memmove(new_arr, data.data.int_arr, data.size * ctypes.sizeof(ctypes.c_int))
        new_data.data.int_arr = new_arr
    elif data.type == MorlocSerialType.MORLOC_FLOAT_ARRAY:
        new_arr = (ctypes.c_double * data.size)()
        ctypes.memmove(new_arr, data.data.float_arr, data.size * ctypes.sizeof(ctypes.c_double))
        new_data.data.float_arr = new_arr
    elif data.type == MorlocSerialType.MORLOC_EXT:
        new_data.data.char_arr = ctypes.c_char_p(ctypes.cast(data.data.char_arr, ctypes.c_char_p).value)
    else:
        raise ValueError(f"Unknown ParsedData type: {data.type}")

    return new_data



def pack(py_data, schema: str) -> bytes:
    # Convert Python data to ParsedData
    parsed_data = python_to_parsed_data(py_data, schema)

    # Translate to MessagePack
    return pack_data(parsed_data, schema)



def unpack(msgpack_data, schema: str):
    # Unpack the data
    unpacked_data = unpack_data(msgpack_data, schema)

    # Convert ParsedData back to Python
    return parsed_data_to_python(unpacked_data)



if __name__ == "__main__":
    import time
    from colorama import Fore, Style, init

    # Initialize colorama
    init(autoreset=True)

    def generate_test_integers():
        # Maximum and minimum 32-bit signed integers
        max_int = 2**31 - 1
        min_int = -2**31

        # Generate 1024 evenly spaced values
        step = (max_int - min_int) // 1023
        main_range = list(range(max_int, min_int, -step))

        # Generate powers of 2, their +/- 1 values, and sums of powers of 2
        powers_of_2 = []
        sums_of_powers = []
        current_sum = 0
        for i in range(31):  # 31 because 2^31 is already the max signed 32-bit int
            base = 2**i
            current_sum += base
        
            # Individual powers of 2 and their +/- 1 values
            powers_of_2.extend([base - 1, base, base + 1, -(base - 1), -base, -(base + 1)])
        
            # Sums of powers of 2 and their +/- 1 values
            sums_of_powers.extend([current_sum - 1, current_sum, current_sum + 1,
                                   -(current_sum - 1), -current_sum, -(current_sum + 1)])
        
            # Add intermediate sums (e.g., 2^3 + 2^2)
            if i > 0:
                intermediate_sum = base + 2**(i-1)
                sums_of_powers.extend([intermediate_sum - 1, intermediate_sum, intermediate_sum + 1,
                                       -(intermediate_sum - 1), -intermediate_sum, -(intermediate_sum + 1)])

        # Combine all values, remove duplicates, and sort
        all_values = sorted(set(main_range + powers_of_2 + sums_of_powers), reverse=True)

        # lazy solution to the edge cases generated above that overflow at the 32bits
        return [x for x in all_values if x > min_int and x < max_int]

    def schema_size(number: int) -> str:
        """
        Convert a number between 0 and 63 to its corresponding character.
        
        :param number: An integer between 0 and 63
        :return: The corresponding character
        """
        if not (0 <= number <= 63):
            raise ValueError("Number must be between 0 and 63")

        if 0 <= number <= 9:
            return chr(ord('0') + number)
        elif 10 <= number <= 35:
            return chr(ord('a') + (number - 10))
        elif 36 <= number <= 61:
            return chr(ord('A') + (number - 36))
        elif number == 62:
            return '+'
        elif number == 63:
            return '/'
        else:
            raise ValueError("Invalid number")  # This should never happen due to the initial check


    big_schema_dict = {
        "null_value": "z",
        "boolean": "b",
        "integer": "i4",
        "float": "f8",
        "binary": "r",
        "string": "s",
        "array": "ai4",
        "nested_map": "m21ai41bi4",
        "tuple": "t3i4bi4"
    }

    big_schema = "m" + schema_size(len(big_schema_dict)) + "".join([schema_size(len(k)) + k + v for (k, v) in big_schema_dict.items()])

    big_data = {
        "null_value": None,
        "boolean": True,
        "integer": 42,
        "float": 3.14,
        "binary": b'\x10\x01\x02\x03',
        "string": "Hello, World!",
        "array": list(range(5)),
        "nested_map": {"a": 1, "b": 2},
        "tuple": (4, True, 6),
    }
    test_cases = [
        ("Empty string", "s", ""),
        ("Single character string", "s", "x"),
        ("Large string", "s", "x" * 1000000),
        ("Boolean true", "b", True),
        ("Boolean false", "b", False),
        ("Negative integer", "i4", -1000000),
        ("Positive integer", "i4", 1000000),
        ("Positive float", "f8", 6.9420),
        ("Positive float from int", "f8", 6),
        ("Negative float", "f8", -6.9420),
        ("Empty binary", "r", b''),
        ("Binary with prefix", "r", b'\x00susan'),
        ("Large binary", "r", b'x' * 1000000),
        #
        ("Empty boolean array", "ab", []),
        ("Single boolean array", "ab", [True]),
        ("Large boolean array", "ab", [True, False] * 10000),
        #
        ("Empty integer array", "ai4", []),
        ("Single integer array", "ai4", list(range(1))),
        ("Special ints", "ai4", [-129, -32769]), # yes, these specific integers failed
        ("Test integers", "ai4", generate_test_integers()),
        ("Small integer array with negatives", "ai4", list(range(10)) + [-1 * i for i in range(10)]),
        ("Large integer array with negatives", "ai4", list(range(500000)) + [-1 * i for i in range(500000)] + []),
        ("Large integer array of positives", "ai4", list(range(1000000))),
        #
        ("Empty float array", "af8", []),
        ("Single float array", "af8", [float(1)]),
        ("Single float array", "af8", [-3-2,-1,0,1,2,3]),
        ("Large float array with negatives", "af8", [float(x) for x in list(range(10000)) + [-1 * i for i in range(10000)]]),
        ("Very large float array", "af8", [float(x) for x in range(1000000)]),
        #
        ("String array with increasing size strings", "as", ["x" * i for i in range(5000)]),
        ("String array with repeated elements (small)", "as", ["as44" for _ in range(10000)]),
        ("String array with repeated elements (large)", "as", ["as44" for _ in range(100000)]),
        #
        ("Tuple with bool and int", "t2bi4", (False, 42069)),
        ("Map with bool and int keys", "m21ab1bi4", {"a": True, "b": 42}),
        #
        ("Complex nested structure", big_schema, big_data)
    ]

    max_width = max(len(desc) for (desc, _, _) in test_cases) + 2

    for description, schema, data in test_cases:
        start_time = time.time()

        try:
            msgpack_data = pack(data, schema)

            result = unpack(msgpack_data, schema)

            end_time = time.time()

            elapsed_time = end_time - start_time

            if result == data:
                print(f"{description:<{max_width}} {Fore.GREEN}pass{Style.RESET_ALL} ({elapsed_time:.4f}s)")
            else:
                print(f"{description:<{max_width}} {Fore.RED}fail{Style.RESET_ALL}")
                print(f"Error: Result does not match expected data")

        except Exception as e:
            print(f"{description:<{max_width}} {Fore.RED}fail{Style.RESET_ALL}")
            print(f"Error: {e}")
