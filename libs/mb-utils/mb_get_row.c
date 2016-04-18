#include "mb_utils.h"

void mb_get_row(size_t rows, size_t cols, int row_index, float matrix[rows][cols], float row[rows]) {
    for (int i = 0; i < cols; ++i) {
        row[i] = matrix[row_index][i];
    }
}
