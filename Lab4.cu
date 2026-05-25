#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <cuda_runtime.h>
#include <cstdlib>

using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::cout;
using std::cerr;
using std::endl;

__global__
void matrixMulKernel(
    const int* A,
    const int* B,
    int* C,
    int rowsA,
    int colsA,
    int colsB
) {

    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < rowsA && col < colsB) {

        int sum = 0;

        for (int k = 0; k < colsA; ++k) {
            sum += A[row * colsA + k] * B[k * colsB + col];
        }

        C[row * colsB + col] = sum;
    }
}

vector<vector<int>> readMatrix(const string& filename) {

    vector<vector<int>> matrix;

    string line;

    ifstream in(filename);

    if (!in.is_open()) {
        throw std::invalid_argument(
            "Something goes wrong with file: " + filename
        );
    }

    while (getline(in, line)) {

        if (line.empty())
            continue;

        vector<int> row;

        stringstream ss(line);

        for (int value; ss >> value;) {
            row.push_back(value);
        }

        if (!row.empty()) {

            if (!matrix.empty() &&
                row.size() != matrix[0].size()) {

                throw std::runtime_error(
                    "Matrix row sizes are inconsistent!"
                );
            }

            matrix.push_back(row);
        }
    }

    in.close();

    if (matrix.empty()) {
        throw std::runtime_error(
            "The file is empty or contains invalid data."
        );
    }

    return matrix;
}

void writeMatrix(
    const vector<vector<int>>& matrix,
    const string& filename
) {

    ofstream file(filename);

    if (!file.is_open()) {
        throw std::invalid_argument(
            "Something goes wrong with file: " + filename
        );
    }

    for (const auto& row : matrix) {

        for (int i = 0; i < row.size(); ++i) {

            file << row[i];

            if (i < row.size() - 1) {
                file << '\t';
            }
        }

        file << '\n';
    }

    file.close();

    cout << "The matrix was successfully written to file: "
         << filename << endl;
}

vector<int> flatten(
    const vector<vector<int>>& matrix
) {

    int rows = matrix.size();
    int cols = matrix[0].size();

    vector<int> flat(rows * cols);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            flat[i * cols + j] = matrix[i][j];
        }
    }

    return flat;
}

vector<vector<int>> unflatten(
    const vector<int>& flat,
    int rows,
    int cols
) {

    vector<vector<int>> matrix(
        rows,
        vector<int>(cols)
    );

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = flat[i * cols + j];
        }
    }

    return matrix;
}

int main(int argc, char* argv[]) {

    try {

        if (argc != 3) {

            cerr << "Usage: program block_x block_y"
                 << endl;

            return 1;
        }

        int blockX = atoi(argv[1]);
        int blockY = atoi(argv[2]);

        auto matrix1 = readMatrix("Source/matrix1.txt");
        auto matrix2 = readMatrix("Source/matrix2.txt");

        int rowsA = matrix1.size();
        int colsA = matrix1[0].size();

        int rowsB = matrix2.size();
        int colsB = matrix2[0].size();

        if (colsA != rowsB) {
            throw std::invalid_argument(
                "Matrix sizes must match!"
            );
        }

        vector<int> flatA = flatten(matrix1);
        vector<int> flatB = flatten(matrix2);

        vector<int> flatC(rowsA * colsB, 0);

        size_t sizeA = rowsA * colsA * sizeof(int);
        size_t sizeB = rowsB * colsB * sizeof(int);
        size_t sizeC = rowsA * colsB * sizeof(int);

        int* d_A = nullptr;
        int* d_B = nullptr;
        int* d_C = nullptr;

        cudaMalloc(&d_A, sizeA);
        cudaMalloc(&d_B, sizeB);
        cudaMalloc(&d_C, sizeC);

        cudaMemcpy(
            d_A,
            flatA.data(),
            sizeA,
            cudaMemcpyHostToDevice
        );

        cudaMemcpy(
            d_B,
            flatB.data(),
            sizeB,
            cudaMemcpyHostToDevice
        );

        dim3 block(blockX, blockY);

        dim3 grid(
            (colsB + block.x - 1) / block.x,
            (rowsA + block.y - 1) / block.y
        );

        cudaEvent_t start, stop;

        cudaEventCreate(&start);
        cudaEventCreate(&stop);

        cudaEventRecord(start);

        matrixMulKernel<<<grid, block>>>(
            d_A,
            d_B,
            d_C,
            rowsA,
            colsA,
            colsB
        );

        cudaEventRecord(stop);

        cudaEventSynchronize(stop);

        float milliseconds = 0.0f;

        cudaEventElapsedTime(
            &milliseconds,
            start,
            stop
        );

        cudaMemcpy(
            flatC.data(),
            d_C,
            sizeC,
            cudaMemcpyDeviceToHost
        );

        auto result = unflatten(
            flatC,
            rowsA,
            colsB
        );

        cout << "\nMultiply matrix time: "
             << milliseconds / 1000.0
             << " s."
             << endl;

        cout << "Block size: "
             << blockX
             << "x"
             << blockY
             << endl;

        cout << "Grid size: "
             << grid.x
             << "x"
             << grid.y
             << endl;

        writeMatrix(
            result,
            "Source/res_cpp.txt"
        );

        cudaFree(d_A);
        cudaFree(d_B);
        cudaFree(d_C);

        cudaEventDestroy(start);
        cudaEventDestroy(stop);
    }

    catch (const std::exception& e) {

        cerr << "Error: "
             << e.what()
             << endl;

        return 1;
    }

    return 0;
}