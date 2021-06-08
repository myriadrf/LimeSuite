/* --------------------------------------------------------------------------------------------
FILE:		qadpd.h
DESCRIPTION  Implementation of Quadrature Adaptive Digital Predistorter module, using memory polinomials
CONTENT:
AUTHOR:		Lime Microsystems LTD
DATE:		Jan 01, 2016
-------------------------------------------------------------------------------------------- */
//using namespace std;
#include <wx/string.h>

class qadpd { 
public:   
	double xip, xqp;	// Quadrature inputs
    double xi, xq;	// Quadrature feedback signal
    double ypi, ypq;	// Quadrature outputs, //19.11.2015

	double err, aerr, perr;	// Errors
    // Constructors
    qadpd(int, int, int);
    qadpd(int, int, int, double, double, double, int);
    ~qadpd();
    // Parameters
public:	// These can not be changed after contructor is called
        // to prevent memory leacking
    int n;			// Memory order - number of taps in the filter
    int m;			// Nonlinearity order - polynomial order
    int nd;			// Delay the input before error is calculated
public:
	//double range;
    double g;		// The gain of the predistorter
    double lambda;  // RLS/SGRAD weighting coefficient
    double alpha;   // SGRAD adaptation step size
    int training;	// Training algorithm flag
    bool sEnv;		// Use squared envelope if true
    double am;		// Amplitude of IO signals.
                    // Used to normalise  error signal and envelopes
    int skip;		// Start adaptation process after skip clock cycles
	int skiping;
	int updating;

	wxString fname;	// Log file name
	wxString fname2;	// Log file name

    enum {LU, GS, GRAD};	// Available training algorithms
                // LU = LU factorisation
                // GS = Gauss-Seidel
                // GRAD = Gradient descent
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
	double uI, uQ, yI, yQ; //19.11.2015
	double **a, **b;		// The cefficients
	double **a_, **b_;		// The novel cefficients
private:    
		// Output evaluation
    void train();		// RLS or gradient descent adaptation

    // Internal variables
    // Delayed yp and postdistorter output.
    // Have to be visable to oeval(), rls() and sgrad()
    //double uI, uQ, yI,yQ; //19.11.2015
    
	FILE *fp;			// Log file
	FILE *fp2;			// Log file

    double *dI_reg, *dQ_reg; // *u_reg, izbaceno 19.11.2015
    // Delay registers
    double *uI_reg,*uQ_reg; // novo 19.11.2015
    // Predistorter parameters
   
	

    double **xIe, **xIep;	// Delay register matrices
    double **xQe, **xQep;
    // RLS variables used in constructing and solving the equations
    double **A, *B;
    int *index;
    double **Ap, *Bp;
	 // , update;
};