#include "mb_utils.h"

void mb_fill_vector(size_t length, float vect[length], float num) {
    for (int i = 0; i < length; ++i) {
        vect[i] = num;
    }
}
