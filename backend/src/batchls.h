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




#ifndef BATCHLS_H
#define BATCHLS_H

#ifdef __cplusplus
extern "C" {
#endif

#define BATCHLS_MAXPARAM 1000
#define BATCHLS_OBS_PHASE 1
#define BATCHLS_OBS_CODE  0

typedef struct {
    double *A;      //design matrix [n_obs x n_param], row-major
    double *l;      // OMC residuals [n_obs]
    double *W;      //diagonal weights [n_obs] = 1/sigma^2
    int *type;      // obs type [n_obs]: BATCHLS_OBS_PHASE or _CODE
    int n_obs;
    int n_param;
    int n_alloc;
} BatchLS_t;

typedef struct {
    double Cxx[BATCHLS_MAXPARAM * BATCHLS_MAXPARAM];
    double C_fixed[9];
    double x[BATCHLS_MAXPARAM];
    double sigma0_sq;
    double sigma0;
    int dof;
    int n_obs;
    int n_phase;
    int n_param;
    int status;
} BatchLS_Result_t;

typedef struct {
    BatchLS_t  acc;
    BatchLS_Result_t result;
    int ready;
} BatchLS_Session_t;

BatchLS_Session_t *batchls_session_create(int n_param, int n_alloc_rows);
void batchls_session_reset (BatchLS_Session_t *s);
void batchls_session_free (BatchLS_Session_t *s);

int  batchls_append (BatchLS_Session_t *s,const double *H, const double *v,  const double *R_diag, const int *vflg,   int nv, int nx);
int  batchls_solve (BatchLS_Session_t *s);
int  batchls_fix_cov(BatchLS_Session_t *s);

void rtkpos_set_session(BatchLS_Session_t *s);
BatchLS_Session_t *rtkpos_get_session(void);
void batchls_print_result(BatchLS_Session_t *s);

#ifdef __cplusplus
}
#endif
#endif
