#include "mb_utils.h"

void mb_scalar_multiply(size_t rows, size_t cols, float matrix[rows][cols], float num) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] *= num;
        }
    }
}
