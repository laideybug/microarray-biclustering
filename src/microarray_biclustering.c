#include <complex.h>
#include <e-hal.h>
#include <e-loader.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IN_ROWS 56
#define IN_COLS 12625
#define N 3
#define PI 3.141592654

float gaussRand();
void scalarMultiply(size_t rows, size_t cols, float matrix[rows][cols], float num);
void fill(size_t rows, size_t cols, float matrix[rows][cols], float num);
void fillRandom(size_t rows, size_t cols, float matrix[rows][cols]);
void mean(size_t rows, size_t cols, float matrix[rows][cols], float mean_vector[cols]);
void sum(size_t rows, size_t cols, float matrix[rows][cols], float sum_vector[cols]);
void square(size_t rows, size_t cols, float matrix[rows][cols]);
void squareRootComplex(size_t rows, size_t cols, complex float _Complex matrix[rows][cols]);
void removeDC(size_t rows, size_t cols, float matrix[rows][cols]);
void initDictionaries(size_t rows, size_t cols, float _Complex update_dictionary[rows][cols], float _Complex dictionary[row][cols]);
void getColumn(size_t rows, size_t cols, int column_index, float matrix[rows][cols], float column[rows]);
void getColumnComplex(size_t rows, size_t cols, int column_index, float _Complex matrix[rows][cols], float _Complex column[rows]);

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
        printf("Failed to open input file.\n");
        return EXIT_FAILURE;
    }

    // Remove DC component from input data (i.e. centering)
    removeDC(IN_ROWS, IN_COLS, input_data);

    // Initialise the dictionaries
    float _Complex update_w[IN_ROWS][N];
    float _Complex dictionary_w[IN_ROWS][N];
    initDictionaries(IN_ROWS, N, update_w, dictionary_w);

    // Epiphany setup
    e_platform_t platform;
	e_epiphany_t dev;

    e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);
    unsigned clr;
    clr = (unsigned)0x00000000;

	e_set_loader_verbosity(L_D0);
	e_set_host_verbosity(H_D0);

    // Open a 1 x N workgroup
	e_open(&dev, 0, 0, 1, N);
    e_reset_group(&dev);

    // xt
    float xt[IN_ROWS];
    getColumn(IN_ROWS, IN_COLS, 0, input_data, xt);

    // Load the first input sample, and respective dictionary atoms into each core
    for (int i = 0; i < N; ++i) {
        float _Complex dictionary_wk[IN_ROWS];
        getColumnComplex(IN_ROWS, N, i, dictionary_w, dictionary_wk);
        
        float _Complex update_wk[IN_ROWS];
        getColumnComplex(IN_ROWS, N, i, update_w, update_wk);

        e_write(&dev, 0, i, 0x2000, &xt, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, 0x4000, &dictionary_wk, IN_ROWS*sizeof(float _Complex));
        e_write(&dev, 0, i, 0x5000, &update_wk, IN_ROWS*sizeof(float _Complex));
        e_write(&dev, 0, i, 0x7000, &clr, sizeof(clr));
    }

    // Load program to the workgroup and run
    if (e_load_group("./bin/Debug/e_microarray_biclustering.srec", &dev, 0, 0, 1, N, E_TRUE) != E_OK) {
        printf("Failed to load e_microarray_biclustering.srec\n");
        return EXIT_FAILURE;
    }

    // Start all of the cores
    e_start_group(&dev);

    e_close(&dev);
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
* Function: fill
* --------------
* Fills a matrix with a given number
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the number to fill the matrix with
*
*/

void fill(size_t rows, size_t cols, float matrix[rows][cols], float num) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            matrix[j][k] = num;
        }
    }
}

/*
* Function: fillRandom
* --------------------
* Fills a matrix with normally distributed
* random numbers
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void fillRandom(size_t rows, size_t cols, float matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float rand_num = gaussRand();
            matrix[j][k] = rand_num;
        }
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
* Function: squareRootComplex
* ---------------------------
* Finds the square root of each element of
* the input matrix
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* sqrt_matrix: the output matrix
*
*/

void squareRootComplex(size_t rows, size_t cols, float matrix[rows][cols], float _Complex sqrt_matrix[rows][cols]) {
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            float _Complex element = (float _Complex)matrix[j][k];
            sqrt_matrix[j][k] = csqrtf(element);
        }
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
    float ones_vector[rows][1];     // 56 x 1
    fill(rows, 1, ones_vector, 1.0f);

    float mean_vector[cols];        // 1 x 12625
    mean(rows, cols, matrix, mean_vector);

    float mult_matrix[rows][cols];      // 56 x 12625

    // Multiplication
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            mult_matrix[j][k] = ones_vector[j][0] * mean_vector[k];
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

void initDictionaries(size_t rows, size_t cols, float _Complex update_dictionary[rows][cols], float _Complex dictionary[row][cols]) {
    float temp_update_dictionary[rows][cols];
    fill(rows, cols, temp_update_dictionary, 0.0f);

    // Convert to float _Complex
    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            update_dictionary[j][k] = (float _Complex)temp_update_dictionary[j][k];
        }
    }

    float temp_dictionary[rows][cols];
    fillRandom(rows, cols, temp_dictionary);
    scalarMultiply(rows, cols, temp_dictionary, 10.0f);
    square(rows, cols, temp_dictionary);

    float sum_vector[1][cols];  // 1 x 3
    sum(rows, cols, temp_dictionary, sum_vector);

    float _Complex sqrt_vector[1][cols];
    squareRootComplex(1, cols, sum_vector, sqrt_vector); 

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            dictionary[j][k] = temp_dictionary[j][k] / sqrt_vector[0][k];
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

/*
* Function: getColumnComplex
* --------------------------
* Retrieves a specified column from matrix
* and assigns it to a column vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* column_index: the index of the column to be retrieved
* matrix: the input matrix
* column: the vector to store the retrieved column in
*
*/

void getColumnComplex(size_t rows, size_t cols, int column_index, float _Complex matrix[rows][cols], float _Complex column[rows]) {
    for (int i = 0; i < rows; ++i) {
        column[i] = matrix[i][column_index];
    }
}
