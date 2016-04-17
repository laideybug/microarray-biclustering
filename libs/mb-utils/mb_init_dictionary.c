#include "mb_utils.h"

void mb_init_dictionary(size_t rows, size_t cols, float dictionary[rows][cols]) {
    float temp_dictionary[rows][cols];
    mb_fill_matrix_random(rows, cols, temp_dictionary);
    mb_scalar_multiply(rows, cols, temp_dictionary, 10.0f);
    mb_square(rows, cols, temp_dictionary);

    float sum_vector[cols];  // 1 x 3
    mb_sum(rows, cols, temp_dictionary, sum_vector);

    float sqrt_vector[cols];
    mb_square_root_vector(cols, sum_vector, sqrt_vector);

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            dictionary[j][k] = temp_dictionary[j][k] / sqrt_vector[k];
        }
    }
}
