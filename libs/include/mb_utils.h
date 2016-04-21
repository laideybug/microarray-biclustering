#ifndef MB_UTILS_H_INCLUDED
#define MB_UTILS_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#define PI 3.141592654

/*
* Function: mb_gauss_rand
* -----------------------
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

float mb_gauss_rand();

/*
* Function: mb_scalar_multiply
* ----------------------------
* Multiplies all elements of a matrix
* by a given scalar value
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the scalar multiplier
*
*/

void mb_scalar_multiply(size_t rows, size_t cols, float matrix[rows][cols], float num);

/*
* Function: mb_fill_matrix
* ------------------------
* Fills a matrix with a given number
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* num: the number to fill the matrix with
*
*/

void mb_fill_matrix(size_t rows, size_t cols, float matrix[rows][cols], float num);

/*
* Function: mb_fill_matrix_random
* -------------------------------
* Fills a matrix with normally distributed
* random numbers
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void mb_fill_matrix_random(size_t rows, size_t cols, float matrix[rows][cols]);

/*
* Function: mb_fill_vector
* ------------------------
* Fills a vector with a given number
*
* length: the number of elements in vector
* vector: the input vector
* num: the number to fill the vector with
*
*/

void mb_fill_vector(size_t length, float vect[length], float num);

/*
* Function: mb_column_mean
* ------------------------
* Averages each column of a matrix and stores the result
* in a given row vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* mean_vector: the row vector to store the mean values
*
*/

void mb_column_mean(size_t rows, size_t cols, float matrix[rows][cols], float mean_vector[cols]);

/*
* Function: mb_column_sum
* -----------------------
* Sums each column of a matrix and stores the result
* in a given row vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
* sum_vector: the row vector to store the summed values
*
*/

void mb_column_sum(size_t rows, size_t cols, float matrix[rows][cols], float sum_vector[cols]);

/*
* Function: mb_element_square_matrix
* ----------------------------------
* Squares each element of the input matrix
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void mb_element_square_matrix(size_t rows, size_t cols, float matrix[rows][cols]);

/*
* Function: mb_element_sqrt_matrix
* --------------------------------
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

void mb_element_sqrt_matrix(size_t rows, size_t cols, float matrix[rows][cols]);

/*
* Function: mb_element_sqrt_vector
* --------------------------------
* Squares each element of
* the input vector
*
* length: the number of elements in vector
* vector: the input vector
*
*/

void mb_element_sqrt_vector(size_t length, float vect[length]);

/*
* Function: mb_remove_dc
* ----------------------
* Removes the DC offset from a matrix of float data
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the input matrix
*
*/

void mb_remove_dc(size_t rows, size_t cols, float matrix[rows][cols]);

/*
* Function: mb_norm_vector
* ------------------------
* Returns the Euclidean norm of a given vector
*
* length: the length of the vector
* vect: the input vector to find the norm of
*
*/

float mb_norm_vector(size_t length, float vect[length]);

/*
* Function: mb_norm_matrix
* ------------------------
* Normalise the elements of the given matrix
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* matrix: the matrix
*
*/

void mb_norm_matrix(size_t rows, size_t cols, float matrix[rows][cols]);

/*
* Function: mb_get_matrix_column
* ------------------------------
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

void mb_get_matrix_column(size_t rows, size_t cols, int column_index, float matrix[rows][cols], float column[rows]);

/*
* Function: mb_get_matrix_row
* ---------------------------
* Retrieves a specified row from a matrix
* and assigns it to a row vector
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* row_index: the index of the row to be retrieved
* matrix: the input matrix
* row: the vector to store the retrieved row in
*
*/

void mb_get_matrix_row(size_t rows, size_t cols, int row_index, float matrix[rows][cols], float row[rows]);

#endif // MB_UTILS_H_INCLUDED
