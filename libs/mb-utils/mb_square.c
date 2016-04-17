#include <math.h>
#include "mb_utils.h"

void mb_square(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float element = matrix[j][k];
            matrix[j][k] = pow(element, 2);
        }
    }
}
