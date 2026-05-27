#include <mpi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

// Чтение матрицы из файла в одномерный вектор. Возвращает размер n.
vector<int> readMatrixFlat(const string& filename, int& n) {
    vector<int> data;
    ifstream in(filename);
    if (!in.is_open()) throw runtime_error("Cannot open file: " + filename);

    string line;
    int cols = 0;
    bool first_row = true;

    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        int val;
        int current_row_cols = 0;
        while (ss >> val) {
            data.push_back(val);
            current_row_cols++;
        }
        if (first_row) {
            cols = current_row_cols;
            first_row = false;
        }
        else if (current_row_cols != cols) {
            throw runtime_error("Inconsistent row sizes in file.");
        }
    }
    if (data.empty()) throw runtime_error("Empty matrix file.");
    n = cols;
    if (data.size() != static_cast<size_t>(n * n))
        throw runtime_error("Matrix is not square.");
    return data;
}

// Запись одномерной матрицы в файл
void writeMatrixFlat(const vector<int>& data, int n, const string& filename) {
    ofstream out(filename);
    if (!out.is_open()) throw runtime_error("Cannot open file: " + filename);
    for (int i = 0; i < n * n; ++i) {
        out << data[i];
        if ((i + 1) % n == 0) out << "\n";
        else out << "\t";
    }
    out.close();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 0;
    vector<int> A_flat; // Будет заполнен только на rank 0

    // 1. Чтение данных только нулевым процессом
    if (rank == 0) {
        try {
            A_flat = readMatrixFlat("Source/matrix1.txt", n);
            // Читаем вторую матрицу во временный буфер, чтобы потом разослать
        }
        catch (const exception& e) {
            cerr << "Rank 0 error: " << e.what() << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (n % size != 0) {
            cerr << "Error: Matrix size " << n << " is not divisible by processes " << size << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // 2. Рассылаем размер матрицы всем
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_n = n / size;
    int local_size = local_n * n;

    // Буферы для локальных вычислений
    vector<int> local_A(local_size);
    vector<int> local_C(local_size);
    vector<int> B_flat(n * n); // Полная матрица B нужна всем процессам

    // Читаем B только на rank 0 и сразу рассылаем
    if (rank == 0) {
        try {
            int n_dummy;
            B_flat = readMatrixFlat("Source/matrix2.txt", n_dummy);
        }
        catch (const exception& e) {
            cerr << "Rank 0 error reading B: " << e.what() << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    MPI_Bcast(B_flat.data(), n * n, MPI_INT, 0, MPI_COMM_WORLD);

    // Рассылаем строки матрицы A между процессами
    MPI_Scatter(A_flat.data(), local_size, MPI_INT,
        local_A.data(), local_size, MPI_INT,
        0, MPI_COMM_WORLD);

    // Синхронизация перед замером чистого времени вычислений
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // 3. Локальное перемножение
    for (int i = 0; i < local_n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            int row_offset = i * n;
            for (int k = 0; k < n; ++k) {
                sum += local_A[row_offset + k] * B_flat[k * n + j];
            }
            local_C[i * n + j] = sum;
        }
    }

    double end_time = MPI_Wtime();
    double elapsed = end_time - start_time;

    // 4. Сбор результата на rank 0
    vector<int> C_flat;
    if (rank == 0) C_flat.resize(n * n);

    MPI_Gather(local_C.data(), local_size, MPI_INT,
        C_flat.data(), local_size, MPI_INT,
        0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "Multiply matrix time: " << elapsed << " s." << endl;
        try {
            writeMatrixFlat(C_flat, n, "Source/res_cpp.txt");
        }
        catch (const exception& e) {
            cerr << e.what() << endl;
        }
    }

    MPI_Finalize();
    return 0;
}