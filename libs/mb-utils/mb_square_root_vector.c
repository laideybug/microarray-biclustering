#include "mb_utils.h"

void mb_square_root_vector(size_t length, float vect[length], float sqrt_vector[length]) {
    for (int i = 0; i < length; ++i) {
        float element = vect[i];
        sqrt_vector[i] = sqrt(element);
    }
}
