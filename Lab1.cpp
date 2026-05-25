#include <iostream>
#include <vector>
#include <fstream>
#include <string> 
#include <sstream>
#include <stdexcept>
#include <chrono>


using namespace std;

vector<vector<int>> mul(const vector<vector<int>>& m1, const vector<vector<int>>& m2) {
    if (m1.empty() || m2.empty() || m1[0].size() != m2.size()) {
        throw invalid_argument("Matrix sizes must match!");
    }
    int n = m1.size();
    vector<vector<int>> res(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                res[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    return res;
}

vector<vector<int>> readMatrix(const string& filename) {
    vector<vector<int>> matrix;

    string line;
    ifstream in(filename);
    if (!in.is_open()) {
        throw invalid_argument("Something goes wrong with file: " + filename);
    }

    while (getline(in, line))
    {
        if (line.empty()) continue;

        vector<int> row;
        stringstream ss(line);
        for (int value; ss >> value;) {
            row.push_back(value);
        }

        if (!row.empty()) {
            if (!matrix.empty() && row.size() != matrix[0].size()) {
                throw runtime_error("Matrix sizes must match!");
            }
            matrix.push_back(row);
        }
    }
    in.close();

    if (matrix.empty()) {
        throw runtime_error("The file is empty or contains invalid data.");
    }
    return matrix;
}

void writeMatrix(const vector<vector<int>>& matrix, const string& filename) {
    ofstream file(filename);

    if (!file.is_open()) {
        throw invalid_argument("Something goes wrong with file: " + filename);
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
    cout << "The matrix was successfully written to the file: " << filename << endl;
}


int main() {
    auto matrix1 = readMatrix("Source/matrix1.txt");
    auto matrix2 = readMatrix("Source/matrix2.txt");

    auto start = chrono::high_resolution_clock::now();

    auto mm = mul(matrix1, matrix2);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;

    cout << "\nMultiply matrix time: " << diff.count() << " s." << endl;

    writeMatrix(mm, "Source/res_cpp.txt");
}
   