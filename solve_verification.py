import os
import re
import subprocess

import numpy as np

from create_matrix import *

np.random.seed(666)
cpp_executable = os.environ.get("CPP_EXECUTABLE", "Lab3.exe")
mpiexec_path = "C:/Program Files/Microsoft MPI/Bin/mpiexec.exe"

cores_list = [1, 2, 4]


def read_file(file_name: str):
    try:
        matrix = np.loadtxt(file_name, dtype=int)
        return matrix
    except Exception as e:
        print(f"Error: {e}")
        return None


def main():
     for cores in cores_list:
        print(f"\nCores: {cores}")
        cmd = [mpiexec_path, "-n", str(cores), cpp_executable]
        for n in range(200, 2200, 200):
            total_time = 0
            for i in range(0, 10):
                create_matrix(n)
                try:
                    result = subprocess.run(cmd, check=True, capture_output=True, text=True).stdout
                    match = re.search(r'Multiply matrix time:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*s\.?', result)
                   
                    if match:
                        execution_time = float(match.group(1))
                        total_time += execution_time

                    else:
                        print("Time is not found in output")

                except subprocess.CalledProcessError as e:
                    print("Error in execute Cpp file:", e)
                
            print(f"Time for matrix {n}*{n}: {total_time / 10} sec.")


if __name__ == "__main__":
    main()
