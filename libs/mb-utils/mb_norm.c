#include <math.h>
#include "mb_utils.h"

float mb_norm(size_t length, float vect[length]) {
    float v_sum = 0.0f;

    for (int i = 0; i < length; ++i) {
        v_sum += pow(vect[i], 2);
    }

    return sqrtf(v_sum);
}
