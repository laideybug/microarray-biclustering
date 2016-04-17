#include "mb_utils.h"

void mb_fill_matrix_random(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float rand_num = mb_gauss_rand();
            matrix[j][k] = rand_num;
        }
    }
}
