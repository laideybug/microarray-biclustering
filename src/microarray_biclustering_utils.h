#ifndef MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED
#define MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED

#include <math.h>
#include "common.h"

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

float gaussRand();

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

void scalarMultiply(size_t rows, size_t cols, float matrix[rows][cols], float num);

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

void fillMatrix(size_t rows, size_t cols, float matrix[rows][cols], float num);

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

void fillMatrixRandom(size_t rows, size_t cols, float matrix[rows][cols]);

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

void fillVector(size_t length, float vector[length], float num);

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

void mean(size_t rows, size_t cols, float matrix[rows][cols], float mean_vector[cols]);

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

void sum(size_t rows, size_t cols, float matrix[rows][cols], float sum_vector[cols]);

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

void square(size_t rows, size_t cols, float matrix[rows][cols]);

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

void squareRootMatrix(size_t rows, size_t cols, float matrix[rows][cols], float sqrt_matrix[rows][cols]);

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

void squareRootVector(size_t length, float vector[length], float sqrt_vector[length]);

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

void removeDC(size_t rows, size_t cols, float matrix[rows][cols]);

/*
* Function: initDictionaries
* --------------------------
* Initialises the full dictionary and full
* update dictionary
*
* rows: the number of rows in matrix
* cols: the number of columns in matrix
* dictionary: the dictionary
*
*/

void initDictionary(size_t rows, size_t cols, float dictionary[rows][cols]);

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

void getColumn(size_t rows, size_t cols, int column_index, float matrix[rows][cols], float column[rows]);

#endif // MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED
