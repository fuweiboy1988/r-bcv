 
#ifndef _BCV_SVD_H
#define _BCV_SVD_H

#include <stdlib.h>
#include "bcv-types.h"

/** 
 * bcv_svd_t:
 *  
 * A #bcv_svd_t is a workspace for a perfoming a Gabrial-style SVD
 * cross-validation.
 *
 * After being initiliazed with either bcv_svd_init() or bcv_svd_initp(), 
 * this structure maintains the current residual estimate of the held-out 
 * block along with the estimated factors.  The residuals can we updated with
 * bcv_svd_update() and queried with bcv_svd_get_rss().
 */
typedef struct _bcv_svd bcv_svd_t;

/**
 * bcv_holdin_t:
 * @m: the number of rows in the held-in set.
 * @n: the number of columns on the held-in set.
 *
 * A #bcv_holdin_t specifies the dimensions of the held-in matrix.
 */
typedef struct _bcv_holdin {
    bcv_index_t m;
    bcv_index_t n;
} bcv_holdin_t;

#define _bcv_assert_valid_holdin(x,M,N) \
    assert (x); \
    assert (0 <= (x)->m && (x)->m <= (M)); \
    assert (0 <= (x)->n && (x)->n <= (N));

/** 
 * bcv_svd_alloc:
 * @holdin: the size of the held-in matrix.
 * @M: the number of rows in the matrix
 * @N: the number of columns in the matrix
 * 
 * Allocate memory for a BCV computation large enough to be used by
 * bcv_svd_init().  The workspace should be freed with bcv_svd_free().
 *
 * This function is monotonic in all of its arguments.
 */
void *
bcv_svd_alloc (bcv_holdin_t holdin, bcv_index_t M, bcv_index_t N);

/**
 * bcv_svd_size:
 * @holdin: the size of the held-in matrix.
 * @M: the number of rows in the matrix
 * @N: the number of columns in the matrix
 *
 * Returns the size (in bytes) of a BCV workspace necessary to cross-validate
 * a matrix with the given dimensions, or 0 if the workspace is larger
 * than %SIZE_MAX bytes.
 */
size_t
bcv_svd_size (bcv_holdin_t holdin, bcv_index_t M, bcv_index_t N);

/**
 * bcv_svd_free:
 * @bcv: the BCV workspace
 * 
 * Free the BCV workspace.
 */
void 
bcv_svd_free (bcv_svd_t *bcv);

/**
 * bcv_svd_init:
 * @mem: unitialized memory for a #bcv_svd_t
 * @holdin: the size of the held-in matrix.
 * @M: the number of rows in the matrix
 * @N: the number of columns in the matrix
 *
 * Cast @mem to point to a #bcv_svd_t and initialize the structure for
 * a BCV computation with the given matrix and holdout dimensions.  The
 * @mem array must have at least bcv_svd_size() bytes.  Return the
 * type-cast pointer.
 *
 * This function does not allocate any memory.
 */
bcv_svd_t *
bcv_svd_init (void *mem, bcv_holdin_t holdin, bcv_index_t M, bcv_index_t N);

/** 
 * bcv_svd_decompose:
 * @bcv: an initialized BCV workspace
 * @x: a matrix to cross-validate
 *
 * Divide @x into blocks, copy it into the BCV workspace, and take the SVD of
 * the held-in block.  Returns zero on success and a positive number on 
 * failure to compute the SVD of the held-in block.
 */
bcv_error_t
bcv_svd_decompose (bcv_svd_t *bcv, const bcv_matrix_t *x);

/**
 * bcv_svd_decompose_with_perm:
 * @bcv: an initialized BCV workspace
 * @x: a matrix to cross-validate
 * @p: a row permutation
 * @q: a column permutation
 *
 * Divide @x into blocks, copy it into the BCV workspace while
 * optionally permuting the rows or columns of the copy.  If either of
 * @p or @q is non-null, the matrix rows or columns get permuted.  
 * In this case that @p and @q are both non-null, @x[i,j] gets replaced by
 * @x[p[i], q[j]].  Finally, take the SVD of the held-in block.
 *
 * Returns zero on success and a positive number on failure to compute the
 * SVD of the held-in block.
 */
bcv_error_t
bcv_svd_decompose_with_perm (bcv_svd_t *bcv, const bcv_matrix_t *x,
                             bcv_index_t *p, bcv_index_t *q);

/**
 * bcv_svd_get_resid:
 * @bcv: the BCV workspace
 * @resid: the residual matrix
 *
 * Initialize @resid@ with the dimensions and data pointer of the residual
 * from the held-out matrix.
 */
void 
bcv_svd_get_resid (const bcv_svd_t *bcv, bcv_matrix_t *resid);

/**
 * bcv_svd_get_rss:
 * @bcv: the BCV workspace
 * 
 * Get the sum of squares of the elements in the residual matrix.
 */
double 
bcv_svd_get_resid_rss (const bcv_svd_t *bcv);

/**
 * bcv_svd_get_max_rank:
 * @bcv: the BCV workspace
 *
 * Get the maximum SVD rank.  This is equal to the smallest dimension
 * of the held-in set.
 */
bcv_index_t
bcv_svd_get_max_rank (bcv_svd_t *bcv);

/**
 * bcv_svd_update_resid
 * @bcv: the BCV workspace
 * @scale: a scalar to multiply the factor by before subtracting it
 *   from the residual matrix
 * @i: the factor index
 *
 * Replace the residual estimate x22 by x22 - scale d2[i] u2[i] v2[i]^T.
 * Normally, @scale will be 1.0.
 */
void 
bcv_svd_update_resid (bcv_svd_t *bcv, double scale, bcv_index_t k);

#endif /* _BCV_SVD_H */
