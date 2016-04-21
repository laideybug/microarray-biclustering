#include "mb_utils.h"

void mb_element_square_matrix(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] = matrix[j][k] * matrix[j][k];
        }
    }
}
