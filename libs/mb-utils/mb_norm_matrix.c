#include "mb_utils.h"

void mb_norm_matrix(size_t rows, size_t cols, float matrix[rows][cols]) {
    float temp_matrix[rows][cols], temp_vector[cols];

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            temp_matrix[j][k] = matrix[j][k];
        }
    }

    mb_element_square_matrix(rows, cols, temp_matrix);
    mb_column_sum(rows, cols, temp_matrix, temp_vector);
    mb_element_sqrt_vector(cols, temp_vector);

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] = matrix[j][k] / temp_vector[k];
        }
    }
}
