
#include <assert.h>
#include <strings.h>
#include <R.h>
#include <R_ext/Lapack.h>
#include "bcv-svd.h"
#include "bcv-types.h"
#include "bcv-matrix-private.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define BLOCKSIZE 64


struct _bcv_svd
{
    int M_max, N_max;
    bcv_matrix_t *x11; bcv_matrix_t *x12;
    bcv_matrix_t *x21; bcv_matrix_t *x22;
    double *d;
};


static bcv_error_t
bcv_svd_decompose (bcv_svd_t *bcv);


bcv_svd_t *
bcv_svd_alloc (int M_max, int N_max)
{
    bcv_svd_t *bcv;
    
    assert( M_max >= 1 );
    assert( N_max >= 1 );
    
    if (   (bcv            = malloc (sizeof (bcv_svd_t)))
        && (bcv->x11       = malloc (sizeof (bcv_matrix_t)))
        && (bcv->x21       = malloc (sizeof (bcv_matrix_t)))
        && (bcv->x12       = malloc (sizeof (bcv_matrix_t)))
        && (bcv->x22       = malloc (sizeof (bcv_matrix_t)))
        && (bcv->x11->data = malloc (M_max * N_max * sizeof (double)))
        && (bcv->d         = malloc (MIN (M_max, N_max) *  sizeof (double)))
       )
    {
        bcv->M_max = M_max;
        bcv->N_max = N_max;
        
        return bcv;
    } 
        
    error ("Could not allocate bcv_svd_t for size (%d,%d)", M_max, N_max);
    return NULL;
}


bcv_error_t
bcv_svd_init (bcv_svd_t *bcv, int M, int N, int m, int n, double *x, int ldx)
{
    return bcv_svd_initp (bcv, M, N, m, n, x, ldx, NULL, NULL);
}

bcv_error_t
bcv_svd_initp (bcv_svd_t *bcv, int M, int N, int m, int n, double *x, int ldx, 
               int *p, int *q)
{
    bcv_error_t result = 0;
    
    assert (bcv);
    assert (x);
    assert (0 < M && M <= bcv->M_max);
    assert (0 < N && N <= bcv->N_max);
    assert (0 < m && m < M);
    assert (0 < n && n < N);
    assert (ldx >= M);
    
    bcv->x11->m   = m;
    bcv->x11->n   = n;
    bcv->x11->lda = M;

    bcv->x21->m   = M - m;
    bcv->x21->n   = n;
    bcv->x21->data = bcv->x11->data + m;
    bcv->x21->lda = M;
    
    bcv->x12->m    = m;
    bcv->x12->n    = N - n;
    bcv->x12->data = bcv->x11->data + n * M;
    bcv->x12->lda  = M;

    bcv->x22->m    = M - m;
    bcv->x22->n    = N - n;
    bcv->x22->data = bcv->x11->data + m + n * M;
    bcv->x22->lda  = M;

    bcv_matrix_t dst = { M, N, bcv->x11->data, M   };
    bcv_matrix_t src = { M, N, x,              ldx };
    _bcv_matrix_permute_copy (&dst, &src, p, q);
    
    result = bcv_svd_decompose (bcv);
    
    return result;
}

void 
bcv_svd_free (bcv_svd_t *bcv)
{
    if (bcv)
    {
        free (bcv->d);
        free (bcv->x11->data);
        free (bcv->x11);
        free (bcv->x12);
        free (bcv->x21);
        free (bcv->x22);
        free (bcv);
    }
}


void 
bcv_svd_get_resid (const bcv_svd_t *bcv, int *m2, int *n2, double **resid, 
                   int *ldr)
{
    assert (bcv);
    assert (m2);
    assert (n2);
    assert (resid);
    assert (ldr);
    
    *m2    = bcv->x22->m;
    *n2    = bcv->x22->n;
    *resid = bcv->x22->data;
    *ldr   = bcv->x22->lda;
}


int 
bcv_svd_get_max_rank (bcv_svd_t *bcv)
{
    assert (bcv);
    return MIN (bcv->x11->m, bcv->x11->n);
}


double 
bcv_svd_get_resid_mse (const bcv_svd_t *bcv)
{
    int m2 = bcv->x22->m;
    int n2 = bcv->x22->n;
    double frob;
    double mse;
    
    frob = _bcv_matrix_norm_frob (bcv->x22);
    mse  = (frob * frob) / (m2 * n2);
    
    return mse;
}


/*
 * Decompose x11 = Q Q1 D P1^T P^T
 * Set       x11  := P1^T
 *           x12  := Q1^T Q^T x12
 *           x21  := x21 P
 *           work := D
 */
static bcv_error_t
bcv_svd_decompose (bcv_svd_t *bcv)
{
    bcv_error_t result = 0;
    bcv_index_t m, n, mn;
    bcv_matrix_uplo_t uplo;
    
    assert (bcv);
    assert (bcv->d);
    _bcv_assert_valid_matrix (bcv->x11);
    _bcv_assert_valid_matrix (bcv->x12);
    _bcv_assert_valid_matrix (bcv->x21);
    
    m  = bcv->x11->m;
    n  = bcv->x11->n;
    mn = MIN (m, n);

    /* decompose x11 := Q B P^T */ 
    bcv_index_t lwork = (m + n) * BLOCKSIZE;
    double e[mn];
    double tauq[mn];
    double taup[mn];
    double work[lwork];
    _bcv_lapack_dgebrd (bcv->x11, bcv->d, e, tauq, taup, work, lwork);

    /* set x21 := x21 P */
    _bcv_lapack_dormbr (BCV_MATRIX_VECT_P, BCV_MATRIX_RIGHT, 
                        BCV_MATRIX_NOTRANS, bcv->x11, taup, bcv->x21, 
                        work, lwork);

    /* set x12 := Q^T x12 */
    _bcv_lapack_dormbr (BCV_MATRIX_VECT_Q, BCV_MATRIX_LEFT, 
                        BCV_MATRIX_TRANS, bcv->x11, tauq, bcv->x12, 
                        work, lwork);

    /* we can now drop the extra rows and columns; they never enter into
     * the svd.
     */
    uplo   = (m >= n) ? BCV_MATRIX_UPPER : BCV_MATRIX_LOWER;
    bcv->x11->m = mn;
    bcv->x11->n = mn;
    bcv->x12->m = mn;
    bcv->x21->n = mn;
    
    /* decompose B = Q1 S P1^T
     *    set x11 := P1^T 
     *        x12 := Q1^T x12
     */
    _bcv_matrix_set_identity (bcv->x11);
    result = _bcv_lapack_dbdsqr (uplo, mn, bcv->d, e, 
                                 bcv->x11, NULL, bcv->x12, work);
    
    return result;
}


/*
 * This function updates x22 as:
 *         x22 := x22 + (scale/d[i]) * u[i] * v[i]^T
 */
void
bcv_svd_update_resid (bcv_svd_t *bcv, double scale, int i)
{
    assert (bcv);
    assert (0 <= i && i < bcv_svd_get_max_rank (bcv));

    double alpha = scale / bcv->d[i];
    
    /* x11 stores P1^T
     * Set p1 := ei^T P1^T = P1 ei 
     */
    bcv_vector_t p1 = { bcv->x11->n, bcv->x11->data + i, bcv->x11->lda };
    
    /* x21 stores x21 P
     * Set u := x21 P u = x21 P P1 ei
     *
     * In bcv_svd_size, we have been careful to make sure that the extra work
     * space has room for at least M doubles.
     */
    bcv_index_t m2 = bcv->x21->m;
    double work[m2];
    bcv_vector_t u = { m2, work, 1 };
    _bcv_blas_dgemv (BCV_MATRIX_NOTRANS, 1.0, bcv->x21, &p1, 0.0, &u);

    /* x12 stores Q1^T Q^T x12
     * Set v := ei^T Q1^T Q^T x12
     */
    bcv_vector_t v = { bcv->x12->n, bcv->x12->data + i, bcv->x12->lda };
    
    /* Update x22 := x22 + alpha * u v^T */
    _bcv_blas_dger (alpha, &u, &v, bcv->x22);
}
