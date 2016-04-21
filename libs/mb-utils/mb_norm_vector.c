#include <math.h>
#include "mb_utils.h"

float mb_norm_vector(size_t length, float vect[length]) {
    float v_sum = 0.0f;

    for (int i = 0; i < length; ++i) {
        v_sum += vect[i] * vect[i];
    }

    return sqrtf(v_sum);
}
