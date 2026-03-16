// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <math.h>
// #include "rtklib.h"
// #include "batchls.h"

// static BatchLS_Session_t *g_session = NULL;
// void  rtkpos_set_session(BatchLS_Session_t *s) { g_session = s; }
// BatchLS_Session_t *rtkpos_get_session(void) { return g_session; }

// static double *rmat(int r, int c)
// {
//     double *m = (double *)calloc((size_t)r * c, sizeof(double));
//     if (!m) {
//         fprintf(stderr,"[batchls] malloc failed %dx%d\n",r,c); exit(1);
//     }
//     return m;
// }

// BatchLS_Session_t *batchls_session_create(int n_param, int n_alloc_rows)
// {
//     BatchLS_Session_t *s = (BatchLS_Session_t *)calloc(1, sizeof(*s));
//     if (!s) return NULL;
//     s->acc.n_param = n_param;
//     s->acc.n_alloc = n_alloc_rows;
//     s->acc.A  = (double *)calloc((size_t)n_alloc_rows * n_param, sizeof(double));
//     s->acc.l = (double *)calloc(n_alloc_rows, sizeof(double));
//     s->acc.W = (double *)calloc(n_alloc_rows, sizeof(double));
//     s->acc.type = (int    *)calloc(n_alloc_rows, sizeof(int));
//     if (!s->acc.A || !s->acc.l || !s->acc.W || !s->acc.type) {
//         batchls_session_free(s); return NULL;
//     }
//     return s;
// }

// void batchls_session_reset(BatchLS_Session_t *s)
// {
//     if (!s) return;
//     s->acc.n_obs = 0;
//     memset(s->acc.A, 0, (size_t)s->acc.n_alloc * s->acc.n_param * sizeof(double));
//     memset(s->acc.l, 0, s->acc.n_alloc * sizeof(double));
//     memset(s->acc.W,  0, s->acc.n_alloc * sizeof(double));
//     memset(s->acc.type, 0, s->acc.n_alloc * sizeof(int));
//     memset(&s->result, 0, sizeof(s->result));
//     s->ready = 0;
// }

// void batchls_session_free(BatchLS_Session_t *s)
// {
//     if (!s) return;
//     free(s->acc.A); free(s->acc.l);
//     free(s->acc.W); free(s->acc.type);
//     free(s);
// }

// int batchls_append(BatchLS_Session_t *s, const double *H, const double *v, const double *R_diag, const int *vflg, int nv, int nx)
// {
//     if (!s || !H || !v || !R_diag || nv <= 0) return -1;
//     BatchLS_t *b  = &s->acc;
//     int  np = b->n_param;
//     int i, j;

//     if (b->n_obs + nv > b->n_alloc) {
//         int na = (b->n_obs + nv) * 2;
//         double *nA = (double *)realloc(b->A,    (size_t)na * np * sizeof(double));
//         double *nl = (double *)realloc(b->l,    na * sizeof(double));
//         double *nW = (double *)realloc(b->W,    na * sizeof(double));
//         int    *nt = (int    *)realloc(b->type, na * sizeof(int));
//         if (!nA || !nl || !nW || !nt) {
//             fprintf(stderr,"[batchls] realloc failed\n"); return -2;
//         }
//         b->A = nA; b->l = nl; b->W = nW; b->type = nt; b->n_alloc = na;
//     }

//     int row0 = b->n_obs;
//     for (i = 0; i < nv; i++) {
//         double s2 = R_diag[i];
//         if (s2 <= 0.0) continue;

//         b->W[row0+i] = 1.0 / s2;
//         b->l[row0+i] = v[i];

//         if (vflg) {
//             b->type[row0+i] = (((vflg[i]>>4)&1) == 0) ? BATCHLS_OBS_PHASE : BATCHLS_OBS_CODE;
//         } else {
//             b->type[row0+i] = (s2 < 0.01) ? BATCHLS_OBS_PHASE : BATCHLS_OBS_CODE;
//         }
//         for (j = 0; j < np && j < nx; j++)
//             b->A[(row0+i)*np + j] = H[i*nx + j];
//     }
//     b->n_obs += nv;
//     return 0;
// }

// static int build_active_map(const double *A, int n, int np, int *col_map)
// {
//     int j, i, u = 0;
//     col_map[u++] = 0;
//     col_map[u++] = 1;
//     col_map[u++] = 2;
//     for (j = 3; j < np; j++) {
//         for (i = 0; i < n; i++) {
//             if (A[i*np+j] != 0.0) { col_map[u++] = j; break; }
//         }
//     }
//     return u;
// }

// static int rank_revealing_elimination(const double *N_in, int u, int *pivot, double tol)
// {
//     double *M = rmat(u, u);
//     int *perm = (int *)malloc(u * sizeof(int));
//     int i, j, k, rank = 0;

//     memcpy(M, N_in, (size_t)u * u * sizeof(double));
//     for (j = 0; j < u; j++) perm[j] = j;

//     for (k = 0; k < u; k++) {
//         double max_d = 0.0; int max_j = k;
//         for (j = k; j < u; j++)
//             if (M[j*u+j] > max_d) { max_d = M[j*u+j]; max_j = j; }
//         if (max_d < tol) break;

//         if (max_j != k) {
//             for (i=0;i<u;i++){double t=M[k*u+i];M[k*u+i]=M[max_j*u+i];M[max_j*u+i]=t;}
//             for (i=0;i<u;i++){double t=M[i*u+k];M[i*u+k]=M[i*u+max_j];M[i*u+max_j]=t;}
//             int t=perm[k]; perm[k]=perm[max_j]; perm[max_j]=t;
//         }
//         pivot[rank++] = perm[k];
//         double d = M[k*u+k];
//         for (i=k+1;i<u;i++) {
//             double f=M[i*u+k]/d;
//             for (j=k+1;j<u;j++) M[i*u+j]-=f*M[k*u+j];
//             M[i*u+k]=0.0;
//         }
//     }
//     free(M); free(perm);
//     return rank;
// }

// int batchls_solve(BatchLS_Session_t *s)
// {
//     if (!s) return -1;
//     BatchLS_t  *b = &s->acc;
//     BatchLS_Result_t *res = &s->result;
//     int n  = b->n_obs;
//     int np = b->n_param;
//     int i, j, k;

//     memset(res, 0, sizeof(*res));
//     res->n_obs = n; res->n_param = np;

//     if (n < 10) {
//         res->status = -2; return -2;
//     }

//     double *A = b->A, *l = b->l, *W = b->W;
//     int *type = b->type;
//     {
//         double n0=0,n1=0,n2=0;
//         for (i=0;i<n;i++){
//           n0+=A[i*np+0]*A[i*np+0];
//           n1+=A[i*np+1]*A[i*np+1];
//           n2+=A[i*np+2]*A[i*np+2];
//         }
//         if (n2 < 1e-10)
//             fprintf(stderr,"[batchls] WARNING: dZ still zero — check baseline geometry\n");
//     }
//     int *col_map = (int *)malloc(np * sizeof(int));
//     int  u  = build_active_map(A, n, np, col_map);

//     double *A_c = rmat(n, u);
//     for (i=0;i<n;i++)
//         for (j=0;j<u;j++)
//             A_c[i*u+j] = A[i*np + col_map[j]];

//     double *N_rm = rmat(u, u);
//     double *c_v  = rmat(u, 1);
//     for (i=0;i<n;i++) {
//         double wi = W[i];
//         if (wi<=0.0) continue;
//         const double *Ai = &A_c[i*u];
//         for (j=0;j<u;j++) {
//             c_v[j] += wi*Ai[j]*l[i];
//             for (k=j;k<u;k++) N_rm[j*u+k] += wi*Ai[j]*Ai[k];
//         }
//     }
//     for (j=0;j<u;j++) for (k=j+1;k<u;k++) N_rm[k*u+j]=N_rm[j*u+k];

//     double max_d=0.0;
//     for (j=0;j<u;j++) if (N_rm[j*u+j]>max_d) max_d=N_rm[j*u+j];
//     double tol = (max_d>0.0) ? max_d*1e-10 : 1e-12;

//     int *ind  = (int *)malloc(u * sizeof(int));
//     int  rank = rank_revealing_elimination(N_rm, u, ind, tol);

//     if (rank < 3) {
//         free(col_map);free(A_c);free(N_rm);free(c_v);free(ind);
//         res->status=-1; return -1;
//     }

//     double *A_r = rmat(n, rank);
//     double *N_r = rmat(rank, rank);
//     double *c_r = rmat(rank, 1);
//     for (i=0;i<n;i++)
//         for (j=0;j<rank;j++)
//             A_r[i*rank+j] = A_c[i*u + ind[j]];
//     for (i=0;i<n;i++) {
//         double wi=W[i]; if (wi<=0.0) continue;
//         const double *Ai=&A_r[i*rank];
//         for (j=0;j<rank;j++) {
//             c_r[j]+=wi*Ai[j]*l[i];
//             for (k=j;k<rank;k++) N_r[j*rank+k]+=wi*Ai[j]*Ai[k];
//         }
//     }
//     for (j=0;j<rank;j++) for (k=j+1;k<rank;k++) N_r[k*rank+j]=N_r[j*rank+k];

//     double *N_r_cm = mat(rank, rank);
//     for (j=0;j<rank;j++)
//         for (k=0;k<rank;k++)
//             N_r_cm[j+k*rank] = N_r[j*rank+k];
//     if (matinv(N_r_cm, rank)<0) {
//         free(col_map);free(A_c);free(N_rm);free(c_v);
//         free(ind);free(A_r);free(N_r);free(c_r);free(N_r_cm);
//         res->status=-1; return -1;
//     }
//     double *c_r_cm = mat(rank,1);
//     double *x_r_cm = mat(rank,1);
//     for (j=0;j<rank;j++) c_r_cm[j]=c_r[j];
//     matmul("NN", rank, 1, rank, N_r_cm, c_r_cm, x_r_cm);
//     for (j=0;j<rank;j++) res->x[col_map[ind[j]]] = x_r_cm[j];

//     double vtWv_phase=0.0, vtWv_code=0.0;
//     int    n_phase=0, n_code=0;
//     for (i=0;i<n;i++) {
//         double Ax=0.0;
//         for (j=0;j<rank;j++) Ax+=A_r[i*rank+j]*x_r_cm[j];
//         double vi=Ax-l[i], wi=W[i];
//         if (type[i]==BATCHLS_OBS_PHASE)
//           { vtWv_phase+=wi*vi*vi; n_phase++; }
//         else
//           { vtWv_code +=wi*vi*vi; n_code++;  }
//     }

//     int dof = n_phase - rank;
//     if (dof <= 0) {
//         dof = n_phase + n_code - rank;
//         vtWv_phase += vtWv_code;
//     }
//     res->dof       = dof;
//     res->n_phase   = n_phase;
//     res->sigma0_sq = vtWv_phase / (double)dof;
//     res->sigma0    = sqrt(res->sigma0_sq);

//     for (j=0;j<rank;j++) {
//         int rj=col_map[ind[j]];
//         for (k=0;k<rank;k++) {
//             int rk=col_map[ind[k]];
//             res->Cxx[rj*np+rk] = res->sigma0_sq * N_r_cm[j+k*rank];
//         }
//     }

//     res->status=0;
//     free(col_map);free(A_c);free(N_rm);free(c_v);
//     free(ind);free(A_r);free(N_r);free(c_r);
//     free(N_r_cm);free(c_r_cm);free(x_r_cm);
//     return 0;
// }

// int batchls_fix_cov(BatchLS_Session_t *s)
// {
//     if (!s || s->result.status!=0) return -1;
//     BatchLS_Result_t *res = &s->result;
//     int np=res->n_param, i, j;

//     int *amb=(int *)malloc(np*sizeof(int)); int na=0;
//     for (j=3;j<np;j++) if (res->Cxx[j*np+j]!=0.0) amb[na++]=j;

//     double *Cbb=mat(3,3);
//     for (i=0;i<3;i++) for (j=0;j<3;j++) Cbb[i+j*3]=res->Cxx[i*np+j];

//     if (na<=0) {
//         for (i=0;i<3;i++) for (j=0;j<3;j++) res->C_fixed[i*3+j]=Cbb[i+j*3];
//         free(Cbb);free(amb); s->ready=1; return 0;
//     }

//     double *Cba=mat(3,na);
//     double *Caa=mat(na,na);
//     for (i=0;i<3; i++) for (j=0;j<na;j++) Cba[i+j*3 ]=res->Cxx[i*np+amb[j]];
//     for (i=0;i<na;i++) for (j=0;j<na;j++) Caa[i+j*na]=res->Cxx[amb[i]*np+amb[j]];

//     if (matinv(Caa,na)<0) {
//         free(Cbb);free(Cba);free(Caa);free(amb); return -1;
//     }

//     double *Tmp=mat(3,na);
//     matmul ("NN", 3, na, na, Cba, Caa, Tmp);
//     matmulm("NT", 3,  3, na, Tmp, Cba, Cbb);

//     for (i=0;i<3;i++) for (j=0;j<3;j++) res->C_fixed[i*3+j]=Cbb[i+j*3];
//     free(Cbb);free(Cba);free(Caa);free(Tmp);free(amb);
//     s->ready=1; return 0;
// }







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rtklib.h"
#include "batchls.h"

static BatchLS_Session_t *g_session = NULL;
void  rtkpos_set_session(BatchLS_Session_t *s) { g_session = s; }
BatchLS_Session_t *rtkpos_get_session(void) { return g_session; }

static double *rmat(int r, int c)
{
    double *m = (double *)calloc((size_t)r * c, sizeof(double));
    if (!m) {
        fprintf(stderr,"[batchls] malloc failed %dx%d\n",r,c); exit(1);
    }
    return m;
}

BatchLS_Session_t *batchls_session_create(int n_param, int n_alloc_rows)
{
    BatchLS_Session_t *s = (BatchLS_Session_t *)calloc(1, sizeof(*s));
    if (!s) return NULL;
    s->acc.n_param = n_param;
    s->acc.n_alloc = n_alloc_rows;
    s->acc.A  = (double *)calloc((size_t)n_alloc_rows * n_param, sizeof(double));
    s->acc.l = (double *)calloc(n_alloc_rows, sizeof(double));
    s->acc.W = (double *)calloc(n_alloc_rows, sizeof(double));
    s->acc.type = (int    *)calloc(n_alloc_rows, sizeof(int));
    if (!s->acc.A || !s->acc.l || !s->acc.W || !s->acc.type) {
        batchls_session_free(s); return NULL;
    }
    return s;
}

void batchls_session_reset(BatchLS_Session_t *s)
{
    if (!s) return;
    s->acc.n_obs = 0;
    memset(s->acc.A, 0, (size_t)s->acc.n_alloc * s->acc.n_param * sizeof(double));
    memset(s->acc.l, 0, s->acc.n_alloc * sizeof(double));
    memset(s->acc.W,  0, s->acc.n_alloc * sizeof(double));
    memset(s->acc.type, 0, s->acc.n_alloc * sizeof(int));
    memset(&s->result, 0, sizeof(s->result));
    s->ready = 0;
}

void batchls_session_free(BatchLS_Session_t *s)
{
    if (!s) return;
    free(s->acc.A); free(s->acc.l);
    free(s->acc.W); free(s->acc.type);
    free(s);
}

int batchls_append(BatchLS_Session_t *s, const double *H, const double *v, const double *R_diag, const int *vflg, int nv, int nx)
{
    if (!s || !H || !v || !R_diag || nv <= 0) return -1;
    BatchLS_t *b  = &s->acc;
    int  np = b->n_param;
    int i, j;

    if (b->n_obs + nv > b->n_alloc) {
        int na = (b->n_obs + nv) * 2;
        double *nA = (double *)realloc(b->A,    (size_t)na * np * sizeof(double));
        double *nl = (double *)realloc(b->l,    na * sizeof(double));
        double *nW = (double *)realloc(b->W,    na * sizeof(double));
        int    *nt = (int    *)realloc(b->type, na * sizeof(int));
        if (!nA || !nl || !nW || !nt) {
            fprintf(stderr,"[batchls] realloc failed\n"); return -2;
        }
        b->A = nA; b->l = nl; b->W = nW; b->type = nt; b->n_alloc = na;
    }

    int row0 = b->n_obs;
    for (i = 0; i < nv; i++) {
        double s2 = R_diag[i];
        if (s2 <= 0.0) continue;

        b->W[row0+i] = 1.0 / s2;
        b->l[row0+i] = v[i];

        if (vflg) {
            b->type[row0+i] = (((vflg[i]>>4)&1) == 0) ? BATCHLS_OBS_PHASE : BATCHLS_OBS_CODE;
        } else {
            b->type[row0+i] = (s2 < 0.01) ? BATCHLS_OBS_PHASE : BATCHLS_OBS_CODE;
        }
        for (j = 0; j < np && j < nx; j++)
            b->A[(row0+i)*np + j] = H[i*nx + j];
    }
    b->n_obs += nv;
    return 0;
}

static int build_active_map(const double *A, int n, int np, int *col_map)
{
    int j, i, u = 0;
    col_map[u++] = 0;
    col_map[u++] = 1;
    col_map[u++] = 2;
    for (j = 3; j < np; j++) {
        for (i = 0; i < n; i++) {
            if (A[i*np+j] != 0.0) { col_map[u++] = j; break; }
        }
    }
    return u;
}

static int rank_revealing_elimination(const double *N_in, int u, int *pivot, double tol)
{
    double *M = rmat(u, u);
    int *perm = (int *)malloc(u * sizeof(int));
    int i, j, k, rank = 0;

    memcpy(M, N_in, (size_t)u * u * sizeof(double));
    for (j = 0; j < u; j++) perm[j] = j;

    for (k = 0; k < u; k++) {
        double max_d = 0.0; int max_j = k;
        for (j = k; j < u; j++)
            if (M[j*u+j] > max_d) { max_d = M[j*u+j]; max_j = j; }
        if (max_d < tol) break;

        if (max_j != k) {
            for (i=0;i<u;i++){double t=M[k*u+i];M[k*u+i]=M[max_j*u+i];M[max_j*u+i]=t;}
            for (i=0;i<u;i++){double t=M[i*u+k];M[i*u+k]=M[i*u+max_j];M[i*u+max_j]=t;}
            int t=perm[k]; perm[k]=perm[max_j]; perm[max_j]=t;
        }
        pivot[rank++] = perm[k];
        double d = M[k*u+k];
        for (i=k+1;i<u;i++) {
            double f=M[i*u+k]/d;
            for (j=k+1;j<u;j++) M[i*u+j]-=f*M[k*u+j];
            M[i*u+k]=0.0;
        }
    }
    free(M); free(perm);
    return rank;
}

int batchls_solve(BatchLS_Session_t *s)
{
    if (!s) return -1;
    BatchLS_t  *b = &s->acc;
    BatchLS_Result_t *res = &s->result;
    int n  = b->n_obs;
    int np = b->n_param;
    int i, j, k;

    memset(res, 0, sizeof(*res));
    res->n_obs = n; res->n_param = np;

    if (n < 10) {
        res->status = -2; return -2;
    }

    double *A = b->A, *l = b->l, *W = b->W;
    int *type = b->type;
    {
        double n0=0,n1=0,n2=0;
        for (i=0;i<n;i++){
          n0+=A[i*np+0]*A[i*np+0];
          n1+=A[i*np+1]*A[i*np+1];
          n2+=A[i*np+2]*A[i*np+2];
        }
        if (n2 < 1e-10)
            fprintf(stderr,"[batchls] WARNING: dZ still zero — check baseline geometry\n");
    }
    int *col_map = (int *)malloc(np * sizeof(int));
    int  u  = build_active_map(A, n, np, col_map);

    double *A_c = rmat(n, u);
    for (i=0;i<n;i++)
        for (j=0;j<u;j++)
            A_c[i*u+j] = A[i*np + col_map[j]];

    double *N_rm = rmat(u, u);
    double *c_v  = rmat(u, 1);
    for (i=0;i<n;i++) {
        double wi = W[i];
        if (wi<=0.0) continue;
        const double *Ai = &A_c[i*u];
        for (j=0;j<u;j++) {
            c_v[j] += wi*Ai[j]*l[i];
            for (k=j;k<u;k++) N_rm[j*u+k] += wi*Ai[j]*Ai[k];
        }
    }
    for (j=0;j<u;j++) for (k=j+1;k<u;k++) N_rm[k*u+j]=N_rm[j*u+k];

    double max_d=0.0;
    for (j=0;j<u;j++) if (N_rm[j*u+j]>max_d) max_d=N_rm[j*u+j];
    double tol = (max_d>0.0) ? max_d*1e-10 : 1e-12;

    int *ind  = (int *)malloc(u * sizeof(int));
    int  rank = rank_revealing_elimination(N_rm, u, ind, tol);

    if (rank < 3) {
        free(col_map);free(A_c);free(N_rm);free(c_v);free(ind);
        res->status=-1; return -1;
    }

    double *A_r = rmat(n, rank);
    double *N_r = rmat(rank, rank);
    double *c_r = rmat(rank, 1);
    for (i=0;i<n;i++)
        for (j=0;j<rank;j++)
            A_r[i*rank+j] = A_c[i*u + ind[j]];
    for (i=0;i<n;i++) {
        double wi=W[i]; if (wi<=0.0) continue;
        const double *Ai=&A_r[i*rank];
        for (j=0;j<rank;j++) {
            c_r[j]+=wi*Ai[j]*l[i];
            for (k=j;k<rank;k++) N_r[j*rank+k]+=wi*Ai[j]*Ai[k];
        }
    }
    for (j=0;j<rank;j++) for (k=j+1;k<rank;k++) N_r[k*rank+j]=N_r[j*rank+k];

    double *N_r_cm = mat(rank, rank);
    for (j=0;j<rank;j++)
        for (k=0;k<rank;k++)
            N_r_cm[j+k*rank] = N_r[j*rank+k];
    if (matinv(N_r_cm, rank)<0) {
        free(col_map);free(A_c);free(N_rm);free(c_v);
        free(ind);free(A_r);free(N_r);free(c_r);free(N_r_cm);
        res->status=-1; return -1;
    }
    double *c_r_cm = mat(rank,1);
    double *x_r_cm = mat(rank,1);
    for (j=0;j<rank;j++) c_r_cm[j]=c_r[j];
    matmul("NN", rank, 1, rank, N_r_cm, c_r_cm, x_r_cm);
    for (j=0;j<rank;j++) res->x[col_map[ind[j]]] = x_r_cm[j];

    // double vtWv_phase=0.0, vtWv_code=0.0;
    // int    n_phase=0, n_code=0;
    // for (i=0;i<n;i++) {
    //     double Ax=0.0;
    //     for (j=0;j<rank;j++) Ax+=A_r[i*rank+j]*x_r_cm[j];
    //     double vi=Ax-l[i], wi=W[i];
    //     if (type[i]==BATCHLS_OBS_PHASE)
    //       { vtWv_phase+=wi*vi*vi; n_phase++; }
    //     else
    //       { vtWv_code +=wi*vi*vi; n_code++;  }
    // }

    // int dof = n_phase - rank;
    // if (dof <= 0) {
    //     dof = n_phase + n_code - rank;
    //     vtWv_phase += vtWv_code;
    // }
    // res->dof       = dof;
    // res->n_phase   = n_phase;
    // res->sigma0_sq = vtWv_phase / (double)dof;
    // res->sigma0    = sqrt(res->sigma0_sq);

double vtWv = 0.0;
int    n_phase = 0, n_code = 0;
for (i = 0; i < n; i++) {
    double Ax = 0.0;
    for (j = 0; j < rank; j++) Ax += A_r[i*rank+j] * x_r_cm[j];
    double vi = Ax - l[i];
    vtWv += W[i] * vi * vi;               /* all obs contribute */
    if (type[i] == BATCHLS_OBS_PHASE) n_phase++;
    else                               n_code++;
}
int dof = (n_phase + n_code) - rank;      /* correct total DOF */
if (dof <= 0) dof = 1;
res->dof       = dof;
res->n_phase   = n_phase;
res->sigma0_sq = vtWv / (double)dof;
res->sigma0    = sqrt(res->sigma0_sq);


    // double vtWv = 0.0;
    // int n_phase = 0, n_code = 0;
    // for (i = 0; i < n; i++) {
    //     double Ax = 0.0;
    //     for (j = 0; j < rank; j++)
    //         Ax += A_r[i*rank + j] * x_r_cm[j];
    //     double vi = Ax - l[i];
    //     double wi = W[i];
    //     vtWv += wi * vi * vi;
    //     if (type[i] == BATCHLS_OBS_PHASE)
    //         n_phase++;
    //     else
    //         n_code++;
    // }

    // /* Proper least squares DOF */
    // int dof = n - rank;

    // if (dof <= 0) {
    //     fprintf(stderr,"[batchls] invalid DOF: n=%d rank=%d\n", n, rank);
    //     dof = 1;
    // }
    // res->dof       = dof;
    // res->n_phase   = n_phase;
    // res->sigma0_sq = vtWv / (double)dof;
    // res->sigma0    = sqrt(res->sigma0_sq);

    for (j=0;j<rank;j++) {
        int rj=col_map[ind[j]];
        for (k=0;k<rank;k++) {
            int rk=col_map[ind[k]];
            res->Cxx[rj*np+rk] = res->sigma0_sq * N_r_cm[j+k*rank];
        }
    }

    res->status=0;
    free(col_map);free(A_c);free(N_rm);free(c_v);
    free(ind);free(A_r);free(N_r);free(c_r);
    free(N_r_cm);free(c_r_cm);free(x_r_cm);
    return 0;
}

int batchls_fix_cov(BatchLS_Session_t *s)
{
    if (!s || s->result.status!=0) return -1;
    BatchLS_Result_t *res = &s->result;
    int np=res->n_param, i, j;

    int *amb=(int *)malloc(np*sizeof(int)); int na=0;
    for (j=3;j<np;j++) if (res->Cxx[j*np+j]!=0.0) amb[na++]=j;

    double *Cbb=mat(3,3);
    for (i=0;i<3;i++) for (j=0;j<3;j++) Cbb[i+j*3]=res->Cxx[i*np+j];

    if (na<=0) {
        for (i=0;i<3;i++) for (j=0;j<3;j++) res->C_fixed[i*3+j]=Cbb[i+j*3];
        free(Cbb);free(amb); s->ready=1; return 0;
    }

    double *Cba=mat(3,na);
    double *Caa=mat(na,na);
    for (i=0;i<3; i++) for (j=0;j<na;j++) Cba[i+j*3 ]=res->Cxx[i*np+amb[j]];
    for (i=0;i<na;i++) for (j=0;j<na;j++) Caa[i+j*na]=res->Cxx[amb[i]*np+amb[j]];

    if (matinv(Caa,na)<0) {
        free(Cbb);free(Cba);free(Caa);free(amb); return -1;
    }

    // double *Tmp=mat(3,na);
    // matmul ("NN", 3, na, na, Cba, Caa, Tmp);
    // matmulm("NT", 3,  3, na, Tmp, Cba, Cbb);

    double *Tmp    = mat(3, na);
    double *Schur  = mat(3, 3);
    /* Tmp   = Cba * Caa^{-1}          [3 x na], column-major */
    matmul("NN", 3, na, na, Cba, Caa, Tmp);
    /* Schur = Tmp * Cba^T             [3 x 3],  column-major */
    matmul("NT", 3, 3, na, Tmp, Cba, Schur);
    /* C_fixed = Cbb - Tmp * Cba^T    (Schur complement) */
    for (int _i = 0; _i < 9; _i++) Cbb[_i] -= Schur[_i];
    free(Schur);

    for (i=0;i<3;i++) for (j=0;j<3;j++) res->C_fixed[i*3+j]=Cbb[i+j*3];
    free(Cbb);free(Cba);free(Caa);free(Tmp);free(amb);
    s->ready=1; return 0;
}
