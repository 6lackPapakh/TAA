def obf_add_dummy(a, b, *unused_args):
    return a + b


def obf_add_packed(packed):
    a = (packed >> 16) & 0xFFFF
    b = packed & 0xFFFF
    return a + b


def part_add_one(value):
    return value + 1


def part_double(value):
    return value * 2


def run_scattered_flow(value):
    for func in (part_add_one, part_double):
        value = func(value)
    return value


if __name__ == "__main__":
    packed = (5 << 16) | 3
    print("Dummy parameters:", obf_add_dummy(5, 3, 999, 888, 777))
    print("Packed parameter:", obf_add_packed(packed))
    print("Scattered flow result:", run_scattered_flow(5))
