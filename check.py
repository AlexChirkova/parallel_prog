def compare_files(file1_path, file2_path):
    try:
        with open(file1_path, 'rb') as f1, open(file2_path, 'rb') as f2:
            chunk_size = 4096
            while True:
                chunk1 = f1.read(chunk_size)
                chunk2 = f2.read(chunk_size)
                
                if chunk1 != chunk2:
                    return False
                
                if not chunk1:
                    return True
                    
    except FileNotFoundError as e:
        print(f"Error: File not found - {e}")
        return False
    except IOError as e:
        print(f"Input/output error: {e}")
        return False
    

if __name__ == "__main__":
    print(compare_files("sourse/res_py.txt", "sourse/res_cpp.txt"))