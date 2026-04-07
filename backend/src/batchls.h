#ifndef BATCHLS_H
#define BATCHLS_H

#ifdef __cplusplus
extern "C" {
#endif

#define BATCHLS_MAXPARAM 1000

/* Accumulator: stores only the normal equation (constant size) */
typedef struct {
    double *N;           /* normal matrix [np x np] row-major (fallback)   */
    double *c;           /* right-hand side [np] (fallback)                */
    double  vtRv_phase;  /* accumulated v'R^{-1}v for phase obs (fallback) */
    double  vtRv_code;   /* accumulated v'R^{-1}v for code obs  (fallback) */
    int     n_phase;     /* total phase DD observations (fallback)         */
    int     n_code;      /* total code DD observations  (fallback)         */
    int     n_param;     /* number of parameters (= rtk->nx)               */

    /* TBC-matching primary path: 3x3 baseline normal equations accumulated
     * from fixed-solution post-fit residuals only (ambiguities eliminated). */
    double  N_bb[9];        /* 3x3 baseline normal eq. N_bb, row-major     */
    double  vtRv_fixed;     /* phase v'Diag(R)^{-1}v from fixed residuals  */
    int     n_phase_fixed;  /* phase obs count from fixed epochs            */
} BatchLS_t;

typedef struct {
    double *Cxx;         /* full a posteriori cov [np x np] row-major      */
    double  C_fixed[9];  /* 3x3 baseline cov, column-major (RTKLIB conv.)  */
    double  x[BATCHLS_MAXPARAM];
    double  sigma0_sq;
    double  sigma0;
    int     dof;
    int     n_obs;
    int     n_phase;
    int     n_param;
    int     status;
} BatchLS_Result_t;

typedef struct {
    BatchLS_t         acc;
    BatchLS_Result_t  result;
    int               ready;
} BatchLS_Session_t;

BatchLS_Session_t *batchls_session_create(int n_param);
void  batchls_session_reset(BatchLS_Session_t *s);
void  batchls_session_free (BatchLS_Session_t *s);

/* Per-epoch accumulation with FULL R[nv x nv] col-major (legacy fallback) */
int   batchls_accumulate(BatchLS_Session_t *s,
                       const double *H,    /* [nv x nx] row-major */
                       const double *v,    /* [nv] */
                       const double *R,    /* [nv x nv] col-major */
                       const int    *vflg, /* [nv] */
                       int nv, int nx);

/* Fast per-epoch accumulation using diagonal R — phase observations only.
 * Call this with post-fit residuals from the FIXED solution (xa) to get
 * the TBC-matching a posteriori baseline covariance. O(nv) per epoch. */
int   batchls_accumulate_fixed(BatchLS_Session_t *s,
                       const double *H,    /* [nx x nv] col-major (RTKLIB) */
                       const double *v,    /* [nv]                         */
                       const double *R,    /* [nv x nv] col-major          */
                       const int    *vflg, /* [nv]                         */
                       int nv, int nx);

int   batchls_solve   (BatchLS_Session_t *s);
int   batchls_fix_cov (BatchLS_Session_t *s);

void  rtkpos_set_session(BatchLS_Session_t *s);
BatchLS_Session_t *rtkpos_get_session(void);

#ifdef __cplusplus
}
#endif
#endif














//---------------------------------was working
// #ifndef BATCHLS_H
// #define BATCHLS_H

// #ifdef __cplusplus
// extern "C" {
// #endif

// #define BATCHLS_MAXPARAM 1000
// #define BATCHLS_OBS_PHASE 1
// #define BATCHLS_OBS_CODE  0

// typedef struct {
//     double *A;      //design matrix [n_obs x n_param], row-major
//     double *l;      // OMC residuals [n_obs]
//     double *W;      //diagonal weights [n_obs] = 1/sigma^2
//     int *type;      // obs type [n_obs]: BATCHLS_OBS_PHASE or _CODE
//     int n_obs;
//     int n_param;
//     int n_alloc;
// } BatchLS_t;

// typedef struct {
//     double Cxx[BATCHLS_MAXPARAM * BATCHLS_MAXPARAM];
//     double C_fixed[9];
//     double x[BATCHLS_MAXPARAM];
//     double sigma0_sq;
//     double sigma0;
//     int dof;
//     int n_obs;
//     int n_phase;
//     int n_param;
//     int status;
// } BatchLS_Result_t;

// typedef struct {
//     BatchLS_t  acc;
//     BatchLS_Result_t result;
//     int ready;
// } BatchLS_Session_t;

// BatchLS_Session_t *batchls_session_create(int n_param, int n_alloc_rows);
// void batchls_session_reset (BatchLS_Session_t *s);
// void batchls_session_free (BatchLS_Session_t *s);

// int  batchls_append (BatchLS_Session_t *s,const double *H, const double *v,  const double *R_diag, const int *vflg,   int nv, int nx);
// int  batchls_solve (BatchLS_Session_t *s);
// int  batchls_fix_cov(BatchLS_Session_t *s);

// void rtkpos_set_session(BatchLS_Session_t *s);
// BatchLS_Session_t *rtkpos_get_session(void);
// void batchls_print_result(BatchLS_Session_t *s);

// #ifdef __cplusplus
// }
// #endif
// #endif
