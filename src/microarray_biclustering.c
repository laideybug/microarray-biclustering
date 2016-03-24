#include <common.h>
#include <e-hal.h>
#include <e-loader.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SHM_OFFSET 0x01000000

float gaussRand();
void scalarMultiply(size_t rows, size_t cols, float matrix[rows][cols], float num);
void fillMatrix(size_t rows, size_t cols, float matrix[rows][cols], float num);
void fillMatrixRandom(size_t rows, size_t cols, float matrix[rows][cols]);
void fillVector(size_t length, float vector[length], float num);
void mean(size_t rows, size_t cols, float matrix[rows][cols], float mean_vector[cols]);
void sum(size_t rows, size_t cols, float matrix[rows][cols], float sum_vector[cols]);
void square(size_t rows, size_t cols, float matrix[rows][cols]);
void squareRootMatrix(size_t rows, size_t cols, float matrix[rows][cols], float sqrt_matrix[rows][cols]);
void squareRootVector(size_t length, float vector[length], float sqrt_vector[length]);
void removeDC(size_t rows, size_t cols, float matrix[rows][cols]);
void initDictionaries(size_t rows, size_t cols, float update_dictionary[rows][cols], float dictionary[rows][cols]);
void getColumn(size_t rows, size_t cols, int column_index, float matrix[rows][cols], float column[rows]);

int main(int argc, char *argv[]) {
    float input_data[IN_ROWS][IN_COLS];
    srand(1);

    // Read input data
    FILE *file;
    file = fopen("data.txt", "r");

    if (file != NULL) {
        float data_point;
        int current_row = 0;
        int current_col = 0;

        while (fscanf(file, "%f", &data_point) != EOF) {
            input_data[current_row][current_col] = data_point;

            // data.txt is in 12625 x 56 format,
            // therefore we need to model it as
            // a 56 x 12625 matrix
            if (current_row < (IN_ROWS - 1)) {
                current_row++;
            } else {
                current_row = 0;
                current_col++;
            }
        }

        fclose(file);
    } else {
        printf("Error: Failed to open input file.\n");
        return EXIT_FAILURE;
    }

    // Remove DC component from input data (i.e. centering)
    removeDC(IN_ROWS, IN_COLS, input_data);

    // Initialise the dictionaries
    float update_w[IN_ROWS][N];
    float dictionary_w[IN_ROWS][N];
    initDictionaries(IN_ROWS, N, update_w, dictionary_w);

    // Epiphany setup
    e_platform_t platform;
	e_epiphany_t dev;
	e_mem_t mbuf;

    e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);
    //unsigned clr;
    //clr = (unsigned)CLR;

	e_set_loader_verbosity(L_D0);
	e_set_host_verbosity(H_D0);

    // Open a 1 x N workgroup
	e_open(&dev, 0, 0, 1, N);
    e_reset_group(&dev);

    // Load the dictionary atoms into each core
    for (int i = 0; i < N; ++i) {
        float dictionary_wk[IN_ROWS];
        getColumn(IN_ROWS, N, i, dictionary_w, dictionary_wk);

        float update_wk[IN_ROWS];
        getColumn(IN_ROWS, N, i, update_w, update_wk);

        float dual_var[IN_ROWS];
        fillVector(IN_ROWS, dual_var, 0.0f);

        e_write(&dev, 0, i, WK_MEM_ADDR, &dictionary_wk, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, UP_WK_MEM_ADDR, &update_wk, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_OPT_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_K0_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_K1_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_K2_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
    }

    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, N*sizeof(int)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Load program to the workgroup but do not run yet
    if (e_load_group("./bin/Debug/e_microarray_biclustering.srec", &dev, 0, 0, 1, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering.srec\n");
        return EXIT_FAILURE;
    }

    int done[N], all_done, clr = 0;
    float xt[IN_ROWS];  // xt (first input data sample)
    clock_t start = clock(), diff;

    for (int i = 0; i < IN_COLS; ++i) {
        getColumn(IN_ROWS, IN_COLS, i, input_data, xt);

        for (int j = 0; j < N; ++j) {
            e_write(&dev, 0, j, XT_MEM_ADDR, &xt, IN_ROWS*sizeof(float));   // "Stream" next data sample
            e_write(&mbuf, 0, 0, j*sizeof(int), &clr, sizeof(int));  // Clear done flag
        }

        // Start/wake workgroup
        e_start_group(&dev);
        printf("Processing input sample %i...\n\n", i);

        while (1) {
            all_done = 0;

            for (int j = 0; j < N; ++j) {
                e_read(&mbuf, 0, 0, j*sizeof(int), &done[j], sizeof(int));
                all_done += done[j];
            }

            if (all_done == N) {
                break;
            }
        }

        diff = clock() - start;

        float secs = diff / CLOCKS_PER_SEC;
        printf("Percent complete: %.3f%%, Average speed: %.2f seconds/sample\nTime elapsed: %.2f seconds Total time: %.2f seconds, Remaining time: %.2f seconds\n\n", ((i+1)/IN_COLS)*100.0f, secs/(i+1), secs, (secs/(i+1))*IN_COLS, (secs/(i+1))*IN_COLS - secs);
    }

    e_close(&dev);
    e_free(&mbuf);
    e_finalize();

	return EXIT_SUCCESS;
}

/*
* Function: gaussRand
* -------------------
* Returns a normally distributed
* floating-point random number
*
* Reference: Abramowitz and Stegun
*            Handbook of Mathematical Functions
*
*            S. Summit, C Programming FAQs: Frequently Asked Questions
*            Addison-Wesley, 1995, ISBN 0-201-84519-9
*
*            c-faq.com/lib/gaussian.html
*
*/

float gaussRand() {
    static float U, V;
    static int phase = 0;
    float Z;

    if (phase == 0) {
        U = (rand() + 1.0f) / (RAND_MAX + 2.0f);
        V = rand() / (RAND_MAX + 1.0f);
        Z = sqrt(-2 * log(U)) * sin(2 * PI * V);
    } else {
        Z = sqrt(-2 * log(U)) * cos(2 * PI * V);
    }

    phase = 1 - phase;

    return Z;
}

/*
* Function: scalarMultiply
* ------------------------
* Multiplies all elements of a matrix
* by a given scalar value
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the scalar multiplier
*
*/

void scalarMultiply(size_t rows, size_t cols, float matrix[rows][cols], float num) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] *= num;
        }
    }
}

/*
* Function: fillMatrix
* --------------------
* Fills a matrix with a given number
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the number to fill the matrix with
*
*/

void fillMatrix(size_t rows, size_t cols, float matrix[rows][cols], float num) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] = num;
        }
    }
}

/*
* Function: fillMatrixRandom
* --------------------------
* Fills a matrix with normally distributed
* random numbers
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void fillMatrixRandom(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float rand_num = gaussRand();
            matrix[j][k] = rand_num;
        }
    }
}

/*
* Function: fillVector
* --------------------
* Fills a vector with a given number
*
* length: the number of elements in vector
* vector: the input vector
* num: the number to fill the vector with
*
*/

void fillVector(size_t length, float vector[length], float num) {
    for (int i = 0; i < length; ++i) {
        vector[i] = num;
    }
}

/*
* Function: mean
* --------------
* Averages each column of a matrix and stores the result
* in a given row vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* mean_vector: the row vector to store the mean values
*
*/

void mean(size_t rows, size_t cols, float matrix[rows][cols], float mean_vector[cols]) {
    float column_total;

    for (int j = 0; j < cols; ++j) {
        column_total = 0.0f;

        for (int k = 0; k < rows; ++k) {
            column_total += matrix[k][j];
        }

        float mean = column_total / rows;
        mean_vector[j] = mean;
    }
}

/*
* Function: sum
* -------------
* Sums each column of a matrix and stores the result
* in a given row vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* sum_vector: the row vector to store the summed values
*
*/

void sum(size_t rows, size_t cols, float matrix[rows][cols], float sum_vector[cols]) {
    float column_total;

    for (int j = 0; j < cols; ++j) {
        column_total = 0.0f;

        for (int k = 0; k < rows; ++k) {
            column_total += matrix[k][j];
        }

        sum_vector[j] = column_total;
    }
}

/*
* Function: square
* ----------------
* Squares each element of the input matrix
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void square(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float element = matrix[j][k];
            matrix[j][k] = pow(element, 2);
        }
    }
}

/*
* Function: squareRootMatrix
* --------------------------
* Finds the square root of each element of
* the input matrix and places the results
* in an output matrix
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* sqrt_matrix: the output matrix
*
*/

void squareRootMatrix(size_t rows, size_t cols, float matrix[rows][cols], float sqrt_matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float element = matrix[j][k];
            sqrt_matrix[j][k] = sqrt(element);
        }
    }
}

/*
* Function: squareRootVector
* --------------------------
* Finds the square root of each element of
* the input vector and places the results
* in an output vector
*
* length: the number of elements in vector
* vector: the input vector
* sqrt_vector: the output vector
*
*/

void squareRootVector(size_t length, float vector[length], float sqrt_vector[length]) {
    for (int i = 0; i < length; ++i) {
        float element = vector[i];
        sqrt_vector[i] = sqrt(element);
    }
}

/*
* Function: removeDC
* ------------------
* Removes the DC offset from a matrix of float data
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void removeDC(size_t rows, size_t cols, float matrix[rows][cols]) {
    float ones_vector[rows];     // 56 x 1
    fillVector(rows, ones_vector, 1.0f);

    float mean_vector[cols];        // 1 x 12625
    mean(rows, cols, matrix, mean_vector);

    float mult_matrix[rows][cols];      // 56 x 12625

    // Multiplication
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            mult_matrix[j][k] = ones_vector[j] * mean_vector[k];
        }
    }

    // Subtraction
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] -= mult_matrix[j][k];
        }
    }
}

/*
* Function: initDictionaries
* --------------------------
* Initialises the full dictionary and full
* update dictionary
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* update_dictionary: the "update" dictionary
* dictionary: the dictionary
*
*/

void initDictionaries(size_t rows, size_t cols, float update_dictionary[rows][cols], float dictionary[rows][cols]) {
    fillMatrix(rows, cols, update_dictionary, 0.0f);

    float temp_dictionary[rows][cols];
    fillMatrixRandom(rows, cols, temp_dictionary);
    scalarMultiply(rows, cols, temp_dictionary, 10.0f);
    square(rows, cols, temp_dictionary);

    float sum_vector[cols];  // 1 x 3
    sum(rows, cols, temp_dictionary, sum_vector);

    float sqrt_vector[cols];
    squareRootVector(cols, sum_vector, sqrt_vector);

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            dictionary[j][k] = temp_dictionary[j][k] / sqrt_vector[k];
        }
    }
}

/*
* Function: getColumn
* -------------------
* Retrieves a specified column from a matrix
* and assigns it to a column vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* column_index: the index of the column to be retrieved
* matrix: the input matrix
* column: the vector to store the retrieved column in
*
*/

void getColumn(size_t rows, size_t cols, int column_index, float matrix[rows][cols], float column[rows]) {
    for (int i = 0; i < rows; ++i) {
        column[i] = matrix[i][column_index];
    }
}
