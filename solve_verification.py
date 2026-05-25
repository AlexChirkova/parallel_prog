import os
import re
import subprocess

import numpy as np

from create_matrix import *

np.random.seed(666)
cuda_executable = os.environ.get("CPP_EXECUTABLE", "Lab4.exe")

block_configs = [
    (8, 8),
    (16, 16),
    (32, 32)
]


def read_file(file_name: str):
    try:
        matrix = np.loadtxt(file_name, dtype=int)
        return matrix
    except Exception as e:
        print(f"Error: {e}")
        return None


def main():
    for bx, by in block_configs:
        print(f"\nBlock configs: {bx}x{by}")
        for n in range(200, 2200, 200):
            total_time = 0
            for i in range(0, 10):
                create_matrix(n)
                cmd = [cuda_executable, str(bx), str(by)]
                try:
                    result = subprocess.run(cmd, check=True, capture_output=True, text=True).stdout
                    match = re.search(r'Multiply matrix time:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*s\.?', result)
                   
                    if match:
                        execution_time = float(match.group(1))
                        total_time += execution_time

                    else:
                        print("Time is not found in output")
                except subprocess.CalledProcessError as e:
                    print("Error in execute Cpp file", e)
                
            print(f"Time for matrix {n}*{n}: {total_time / 10} sec.")


if __name__ == "__main__":
    main()
