/* --------------------------------------------------------------------------------------------
FILE:		qadpd.h
DESCRIPTION  Implementation of Quadrature Adaptive Digital Predistorter module, using memory polinomials
CONTENT:
AUTHOR:		Lime Microsystems LTD
DATE:		Jan 01, 2018
-------------------------------------------------------------------------------------------- */
using namespace std;
#include <string>

class qadpd
{
public:
    double xip, xqp;
    double xi, xq;
    double ypi, ypq;
    double err, aerr, perr;
    qadpd(int, int, int);
    qadpd(int, int, int, double, double, double, int);
    ~qadpd();

public:
    int n;
    int m;
    int nd;

public:
    double g;
    double lambda;
    double alpha;
    int training;
    bool sEnv;
    double am;
    int skip;
    int skiping;
    int updating;
    std::string fname;
    std::string fname2;
    bool enlog;
    enum
    {
        LU,
        GS,
        GRAD
    };
    void write_coeff();
    void reset_coeff();
    void write_error();
    void prepare();
    void reset_matrix();
    void init(int N, int M, int Nd, double G, double Lambda, double Am, int Skip);
    void release_memory();
    void always(double XIp, double XQp, double XI, double XQ, double YIp, double YQp, bool Yp_FPGA);
    void oeval(double XIp, double XQp, double XI, double XQ, double YIp, double YQp, bool Yp_FPGA);
    void start();
    void finish();
    int update_coeff(double range);
    double uI, uQ, yI, yQ;
    double uenv, ueps;
    double **a, **b;
    double **a_, **b_;
private:
    void train();
    FILE *fp;
    FILE *fp2;
    double *dI_reg, *dQ_reg;
    double *uI_reg, *uQ_reg;
    double **xIe, **xIep;
    double **xQe, **xQep;
    double **A, *B;
    int *index;
    double **Ap, *Bp;
};
