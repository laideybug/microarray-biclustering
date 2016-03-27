#include "microarray_biclustering_utils.h"

/*
* Function: gaussRand
* -------------------
* Returns a normally distributed
* floating-point random number
*
* Reference: Abramowitz and Stegun
*            Handbook of Mathematical Functions
*
*            S. Summit, C Programming FAQs: Frequently Asked Questions
*            Addison-Wesley, 1995, ISBN 0-201-84519-9
*
*            c-faq.com/lib/gaussian.html
*
*/

float gaussRand() {
    static float U, V;
    static int phase = 0;
    float Z;

    if (phase == 0) {
        U = (rand() + 1.0f) / (RAND_MAX + 2.0f);
        V = rand() / (RAND_MAX + 1.0f);
        Z = sqrt(-2 * log(U)) * sin(2 * PI * V);
    } else {
        Z = sqrt(-2 * log(U)) * cos(2 * PI * V);
    }

    phase = 1 - phase;

    return Z;
}

/*
* Function: scalarMultiply
* ------------------------
* Multiplies all elements of a matrix
* by a given scalar value
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the scalar multiplier
*
*/

void scalarMultiply(size_t rows, size_t cols, float matrix[rows][cols], float num) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] *= num;
        }
    }
}

/*
* Function: fillMatrix
* --------------------
* Fills a matrix with a given number
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the number to fill the matrix with
*
*/

void fillMatrix(size_t rows, size_t cols, float matrix[rows][cols], float num) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] = num;
        }
    }
}

/*
* Function: fillMatrixRandom
* --------------------------
* Fills a matrix with normally distributed
* random numbers
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void fillMatrixRandom(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float rand_num = gaussRand();
            matrix[j][k] = rand_num;
        }
    }
}

/*
* Function: fillVector
* --------------------
* Fills a vector with a given number
*
* length: the number of elements in vector
* vector: the input vector
* num: the number to fill the vector with
*
*/

void fillVector(size_t length, float vector[length], float num) {
    for (int i = 0; i < length; ++i) {
        vector[i] = num;
    }
}

/*
* Function: mean
* --------------
* Averages each column of a matrix and stores the result
* in a given row vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* mean_vector: the row vector to store the mean values
*
*/

void mean(size_t rows, size_t cols, float matrix[rows][cols], float mean_vector[cols]) {
    float column_total;

    for (int j = 0; j < cols; ++j) {
        column_total = 0.0f;

        for (int k = 0; k < rows; ++k) {
            column_total += matrix[k][j];
        }

        float mean = column_total / rows;
        mean_vector[j] = mean;
    }
}

/*
* Function: sum
* -------------
* Sums each column of a matrix and stores the result
* in a given row vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* sum_vector: the row vector to store the summed values
*
*/

void sum(size_t rows, size_t cols, float matrix[rows][cols], float sum_vector[cols]) {
    float column_total;

    for (int j = 0; j < cols; ++j) {
        column_total = 0.0f;

        for (int k = 0; k < rows; ++k) {
            column_total += matrix[k][j];
        }

        sum_vector[j] = column_total;
    }
}

/*
* Function: square
* ----------------
* Squares each element of the input matrix
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void square(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float element = matrix[j][k];
            matrix[j][k] = pow(element, 2);
        }
    }
}

/*
* Function: squareRootMatrix
* --------------------------
* Finds the square root of each element of
* the input matrix and places the results
* in an output matrix
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* sqrt_matrix: the output matrix
*
*/

void squareRootMatrix(size_t rows, size_t cols, float matrix[rows][cols], float sqrt_matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float element = matrix[j][k];
            sqrt_matrix[j][k] = sqrt(element);
        }
    }
}

/*
* Function: squareRootVector
* --------------------------
* Finds the square root of each element of
* the input vector and places the results
* in an output vector
*
* length: the number of elements in vector
* vector: the input vector
* sqrt_vector: the output vector
*
*/

void squareRootVector(size_t length, float vector[length], float sqrt_vector[length]) {
    for (int i = 0; i < length; ++i) {
        float element = vector[i];
        sqrt_vector[i] = sqrt(element);
    }
}

/*
* Function: removeDC
* ------------------
* Removes the DC offset from a matrix of float data
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void removeDC(size_t rows, size_t cols, float matrix[rows][cols]) {
    float ones_vector[rows];     // 56 x 1
    fillVector(rows, ones_vector, 1.0f);

    float mean_vector[cols];        // 1 x 12625
    mean(rows, cols, matrix, mean_vector);

    float mult_matrix[rows][cols];      // 56 x 12625

    // Multiplication
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            mult_matrix[j][k] = ones_vector[j] * mean_vector[k];
        }
    }

    // Subtraction
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] -= mult_matrix[j][k];
        }
    }
}

/*
* Function: initDictionaries
* --------------------------
* Initialises the full dictionary and full
* update dictionary
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* dictionary: the dictionary
*
*/

void initDictionary(size_t rows, size_t cols, float dictionary[rows][cols]) {
    float temp_dictionary[rows][cols];
    fillMatrixRandom(rows, cols, temp_dictionary);
    scalarMultiply(rows, cols, temp_dictionary, 10.0f);
    square(rows, cols, temp_dictionary);

    float sum_vector[cols];  // 1 x 3
    sum(rows, cols, temp_dictionary, sum_vector);

    float sqrt_vector[cols];
    squareRootVector(cols, sum_vector, sqrt_vector);

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            dictionary[j][k] = temp_dictionary[j][k] / sqrt_vector[k];
        }
    }
}

/*
* Function: getColumn
* -------------------
* Retrieves a specified column from a matrix
* and assigns it to a column vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* column_index: the index of the column to be retrieved
* matrix: the input matrix
* column: the vector to store the retrieved column in
*
*/

void getColumn(size_t rows, size_t cols, int column_index, float matrix[rows][cols], float column[rows]) {
    for (int i = 0; i < rows; ++i) {
        column[i] = matrix[i][column_index];
    }
}
