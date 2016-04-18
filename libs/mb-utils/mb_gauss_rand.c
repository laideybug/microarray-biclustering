#include <math.h>
#include "mb_utils.h"

float mb_gauss_rand() {
    static float U, V;
    static int phase = 0;
    float Z;

    if (phase == 0) {
        U = (rand() + 1.0f) / (RAND_MAX + 2.0f);
        V = rand() / (RAND_MAX + 1.0f);
        Z = sqrtf(-2 * logf(U)) * sinf(2 * PI * V);
    } else {
        Z = sqrtf(-2 * logf(U)) * cosf(2 * PI * V);
    }

    phase = 1 - phase;

    return Z;
}
