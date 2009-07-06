
#ifndef _BCV_SVD_GABRIEL_H
#define _BCV_SVD_GABRIEL_H

#include "bcv-partition.h"
#include "bcv-types.h"

/** 
 * bcv_svd_gabriel_t:
 *  
 * A #bcv_svd_gabriel_t is a workspace for a perfoming a Gabrial-style SVD
 * cross-validation.
 *
 * After being initiliazed with bcv_svd_grep_init(), the errors from the
 * various holdouts can be gotten from bcv_svd_gabriel_get_rss().
 */
typedef struct _bcv_svd_gabriel bcv_svd_gabriel_t;

/**
 * bcv_gabriel_holdin_t:
 * @m: the number of rows in the held-in set
 * @n: the number of columns on the held-in set
 *
 * A #bcv_gabriel_holdin_t specifies the dimensions of the held-in matrix.
 */
typedef struct _bcv_gabriel_holdin {
    bcv_index_t m;
    bcv_index_t n;
} bcv_gabriel_holdin_t;

#define _bcv_assert_valid_gabriel_holdin(x,M,N) \
    assert (x); \
    assert (0 <= (x)->m && (x)->m <= (M)); \
    assert (0 <= (x)->n && (x)->n <= (N));

/**
 * bcv_svd_gabriel_alloc:
 * @max_holdin: the maximum holdin dimensions.
 * @M: the number of rows in the matrix being cross-validated
 * @N: the number of columns in the matrix being cross-validated
 *
 * Allocate enough space to hold a #bcv_svd_gabriel_t workspace for 
 * performing a Gabriel-style cross-validation of a matrix with the
 * given dimensions and the given maximum holdin sizes.
 */
bcv_svd_gabriel_t *
bcv_svd_gabriel_alloc (bcv_gabriel_holdin_t max_holdin, bcv_index_t M, 
                       bcv_index_t N);

/**
 * bcv_svd_gabriel_init:
 * @bcv: uninitialized memory for a #bcv_svd_gabriel_t
 * @x: a matrix to cross-validate
 * @rows: a partition of the rows of @x
 * @cols: a partition of the columns of @x
 *
 * Initialize a #bcv_svd_gabriel_t workspace to cross-validate the matrix
 * @x with the hold-outs specified in the @rows and @cols partitions.
 *
 * This function does not allocate any memory.
 */
void
bcv_svd_gabriel_init (bcv_svd_gabriel_t *bcv, const bcv_matrix_t *x, 
                      const bcv_partition_t *rows, 
                      const bcv_partition_t *cols);

/**
 * bcv_svd_gabriel_free:
 * @bcv: a BCV workspace
 *
 * Free a workspace allocated by bcv_svd_gabriel_alloc().
 */
void
bcv_svd_gabriel_free (bcv_svd_gabriel_t *bcv);

/**
 * bcv_svd_gabriel_get_rss:
 * @bcv: an initialized BCV workspace
 * @i: the index of the hold-out row set
 * @j: the index of the hold-out column set
 * @rss: an array to store the RSS from predicting the given hold-out set
 * @max_rank: the maximum rank from which to get the RSS
 *
 * Get the residual sum of squares (RSS) from predicting the (@i,@j) hold-out
 * block with ranks 0, 1, ..., @max_rank SVD terms from the held-in set
 * and store the results in @rss[0], @rss[1], @rss[@max_rank].  Return
 * zero on success and a positive number on failing to compute the SVD of
 * the held-in set.
 *
 * The @max_rank parameter must be less than or equal to the minimum dimension
 * of the held-in set.  See also bcv_svd_gabriel_get_max_rank().
 */
bcv_error_t
bcv_svd_gabriel_get_rss (const bcv_svd_gabriel_t *bcv, bcv_index_t i,
                         bcv_index_t j, double *rss, bcv_index_t max_rank);

/**
 * bcv_svd_gabriel_get_max_rank:
 * @bcv: an initialized BCV workspace
 * @i: the index of the hold-out row set
 * @j: the index of the hold-out column set
 *
 * The the maximum rank possible for predicting the (@i,@j) hold-out set.
 * This is equal to minimum dimension of the corresponding hold-in set.
 */
bcv_index_t
bcv_svd_gabriel_get_max_rank (const bcv_svd_gabriel_t *bcv, bcv_index_t i,
                              bcv_index_t j);

#endif /* _BCV_SVD_GABRIEL_H */
