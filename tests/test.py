import pympack as mp
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
    "binary": "au1",
    "string": "s",
    "array": "ai4",
    "nested_map": "m21ai41bi4",
    "tuple": "t3i4bi4"
}

big_schema = "m" + schema_size(len(big_schema_dict)) + "".join([schema_size(len(k)) + k + v for (k, v) in big_schema_dict.items()])

map_data = (['a', 'b'], [(['c', 'd'], [(1, [1.0, 2.0]), (2, [3.0, 4.0])]), (['e'], [(45, [5.6, 6.7])])])
map_schema = "t2asat2asat2i4af8"

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

    ("u1", "u1", 10),
    ("u2", "u2", 10000),
    ("u4", "u4", 1000000),
    ("u8", "u8", 10000000),

    ("Negative i1", "i1", -10),
    ("Negative i2", "i2", -10000),
    ("Negative i4", "i4", -1000000),
    ("Negative i8", "i8", -10000000),

    ("Positive i1", "i1", 10),
    ("Positive i2", "i2", 10000),
    ("Positive i4", "i4", 1000000),
    ("Positive i8", "i8", 10000000),

    ("Positive f8", "f8", 6.9420),
    #  ("Positive f4", "f4", 6.9420),
    ("Positive f8 from int", "f8", 6),
    #  ("Positive f4 from int", "f4", 6),
    ("Negative f8", "f8", -6.9420),
    #  ("Negative f4", "f4", -6.9420),
    ("Empty binary", "au1", b''),
    ("Binary with prefix", "au1", b'\x00susan'),
    ("Large binary", "au1", b'x' * 1000000),

    ("Empty boolean array", "ab", []),
    ("Single boolean array", "ab", [True]),
    ("Small boolean array", "ab", [True, False] * 2),
    ("Large boolean array", "ab", [True, False] * 10000),

    ("Empty integer array", "ai4", []),
    ("Single integer array", "ai4", list(range(1))),
    ("Special ints", "ai4", [-129, -32769]), # yes, these specific integers failed
    ("Test integers", "ai4", generate_test_integers()),
    ("Small integer array with negatives", "ai4", list(range(10)) + [-1 * i for i in range(10)]),
    ("Large integer array with negatives", "ai4", list(range(500000)) + [-1 * i for i in range(500000)] + []),
    ("Large integer array of positives", "ai4", list(range(1000000))),

    ("Empty float array", "af8", []),
    ("Single float array", "af8", [float(1)]),
    ("Single float array", "af8", [-3-2,-1,0,1,2,3]),
    ("Large float array with negatives", "af8", [float(x) for x in list(range(10000)) + [-1 * i for i in range(10000)]]),
    ("Very large float array", "af8", [float(x) for x in range(1000000)]),

    ("String array with increasing size strings", "as", ["x" * i for i in range(5000)]),
    ("String array with repeated elements (small)", "as", ["as44" for _ in range(10000)]),
    ("String array with repeated elements (large)", "as", ["as44" for _ in range(100000)]),

    ("Tuple with bool and int", "t2bi4", (False, 42069)),
    ("Map with bool and int keys", "m21ab1bi4", {"a": True, "b": 42}),

    ("Complex nested structure", big_schema, big_data),
    ("map", map_schema, map_data),
    ("nested tuples", "at2si4", [("Alice",42), ("Bob",40)]),
]

max_width = max(len(desc) for (desc, _, _) in test_cases) + 2

for description, schema, data in test_cases:
    start_time = time.time()

    try:
        voidstar = mp.to_voidstar(data, schema)
    except Exception as e:
        print(f"{description:<{max_width}} {Fore.RED}fail to void{Style.RESET_ALL}")
        print(f"Error in pack: {e}")
        continue

    try:
        result = mp.from_voidstar(voidstar, schema)
    except Exception as e:
        print(f"{description:<{max_width}} {Fore.RED}fail from void{Style.RESET_ALL}")
        print(f"Error in pack: {e}")
        continue

    #  try:
    #      msgpack_data = mp.to_msgpack(voidstar, schema)
    #  except Exception as e:
    #      print(f"{description:<{max_width}} {Fore.RED}fail to msgpack {Style.RESET_ALL}")
    #      print(f"Error in pack: {e}")
    #      continue
    #
    #  try:
    #      result_data = mp.from_msgpack(msgpack_data, schema)
    #  except Exception as e:
    #      print(f"{description:<{max_width}} {Fore.RED}fail from msgpack{Style.RESET_ALL}")
    #      print(f"Error in unpack: {e}")
    #      continue
    #
    #  try:
    #      result = mp.from_voidstar(result_data, schema)
    #  except Exception as e:
    #      print(f"{description:<{max_width}} {Fore.RED}fail from void{Style.RESET_ALL}")
    #      print(f"Error in pack: {e}")
    #      continue

    end_time = time.time()

    elapsed_time = end_time - start_time

    if result == data:
        print(f"{description:<{max_width}} {Fore.GREEN}pass{Style.RESET_ALL} ({elapsed_time:.4f}s)")
    else:
        print(f"{description:<{max_width}} {Fore.RED}fail{Style.RESET_ALL}")
        print(f"Error: Result does not match expected data")
        print(f"Exected: {str(data)}")
        print(f"Founded: {str(result)}")
