import os
import stat
import sys


def main() -> int:
    if len(sys.argv) != 5:
        print(
            "Usage: python3 patch_string.py <input> <output> <old_text> <new_text>",
            file=sys.stderr,
        )
        return 1

    input_path, output_path, old_text, new_text = sys.argv[1:5]
    old_bytes = old_text.encode("utf-8")
    new_bytes = new_text.encode("utf-8")

    if len(old_bytes) != len(new_bytes):
        print("Replacement text must be the same length as the original text.", file=sys.stderr)
        return 1

    data = bytearray(open(input_path, "rb").read())
    offset = data.find(old_bytes)
    if offset == -1:
        print(f"Text '{old_text}' was not found in {input_path}.", file=sys.stderr)
        return 1

    data[offset : offset + len(old_bytes)] = new_bytes

    with open(output_path, "wb") as file:
        file.write(data)

    mode = stat.S_IMODE(os.stat(input_path).st_mode)
    os.chmod(output_path, mode)

    print(f"Patched '{old_text}' -> '{new_text}' at offset {offset}.")
    print(f"Created: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
