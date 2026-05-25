import numpy as np


def create_matrix(n):
    try:
        matrix1 = np.random.randint(-100, 100, size=(n, n))
        matrix2 = np.random.randint(-100, 100, size=(n, n))
        
        np.savetxt("source/matrix1.txt", matrix1, delimiter=" ", fmt="%.0f")
        np.savetxt("source/matrix2.txt", matrix2, delimiter=" ", fmt="%.0f")
      
    except Exception as e:
        print(f"Error: {e}")

