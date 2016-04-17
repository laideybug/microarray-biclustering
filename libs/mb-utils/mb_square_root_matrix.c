#include "mb_utils.h"

void mb_square_root_matrix(size_t rows, size_t cols, float matrix[rows][cols], float sqrt_matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float element = matrix[j][k];
            sqrt_matrix[j][k] = sqrt(element);
        }
    }
}
