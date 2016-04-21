#include <math.h>
#include "mb_utils.h"

void mb_element_sqrt_vector(size_t length, float vect[length]) {
    for (int i = 0; i < length; ++i) {
        vect[i] = sqrtf(vect[i]);
    }
}
