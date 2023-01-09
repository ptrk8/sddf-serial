#!/usr/bin/python3

import fileinput
import os
from pprint import pprint


def main():
    # Get expected output from file.
    with open(os.environ.get('EXPECTED')) as file:
        lines_expected = file.readlines()

    # Get actual output from standard input.
    lines_actual = []
    is_parsing = False
    for line in fileinput.input():
        print(line, end='')
        if "=== START ===" in line:
            is_parsing = True
        if is_parsing:
            lines_actual.append(line)
        if "=== END ===" in line:
            is_parsing = False

    # Replace carriage return characters in `lines_actual`.
    lines_actual = list(map(lambda x: x.replace('\r', ''), lines_actual))

    if lines_actual == lines_expected:
        print("✔ Test for imx_uart PASSED.")
    else:
        print("✘ Test for imx_uart FAILED.")
        print("=== ACTUAL ===")
        pprint(lines_actual)
        print("=== EXPECTED ===")
        pprint(lines_expected)


if __name__ == '__main__':
    main()
