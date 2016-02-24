#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <e-hal.h>

#define IN_ROWS 56
#define IN_COLS 12625
#define N 3

void fill(size_t rows, size_t cols, float matrix[rows][cols], float num);
void mean(size_t rows, size_t cols, float matrix[rows][cols], float mean_vector[cols]);
void removeDC(size_t rows, size_t cols, float matrix[rows][cols]);

int main(int argc, char *argv[]) {
    float input_data[IN_ROWS][IN_COLS];

    FILE *file;
    file = fopen("data.txt", "r");

    if (file != NULL) {
        float data_point;
        int current_row = 0;
        int current_col = 0;

        while (fscanf(file, "%f", &data_point) != EOF) {
            input_data[current_row][current_col] = data_point;

            // data.txt is in 12625 x 56 format,
            // therefore we need to model it as
            // a 56 x 12625 matrix
            if (current_row < (IN_ROWS - 1)) {
                current_row++;
            } else {
                current_row = 0;
                current_col++;
            }
        }

        fclose(file);
    } else {
        printf("Error: unable to open input file.\n");
    }

    removeDC(IN_ROWS, IN_COLS, input_data);

	return 0;
}

/*
* Function: fill
* --------------
* Fills a matrix with a given number
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the number to fill the matrix with
*
*/

void fill(size_t rows, size_t cols, float matrix[rows][cols], float num) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] = num;
        }
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
    float ones_vector[rows][1];     // 56 x 1
    fill(rows, 1, ones_vector, 1.0f);

    float mean_vector[cols];        // 1 x 12625
    mean(rows, cols, matrix, mean_vector);

    float mult_matrix[rows][cols];      // 56 x 12625

    // Multiplication
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            mult_matrix[j][k] = ones_vector[j][0] * mean_vector[k];
        }
    }

    // Subtraction
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] -= mult_matrix[j][k];
        }
    }
}
