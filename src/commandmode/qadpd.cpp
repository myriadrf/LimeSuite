/* --------------------------------------------------------------------------------------------
FILE:		qadpd.cpp
DESCRIPTION  Implementation of Quadrature Adaptive Digital Predistorter module, using memory polinomials                
CONTENT:
AUTHOR:		Lime Microsystems LTD
DATE:		Jan 01, 2018
-------------------------------------------------------------------------------------------- */
#include "nrc.h"
#include "qadpd.h"

// Constructors
// --------------------------------------------------------------------------------------------

#ifndef __unix__
#define M_PI 3.141592654
#endif

#include <iostream>

qadpd::qadpd(int N, int M, int Nd)
{
	n = N;
	m = M;
	nd = Nd;
	g = 1.0;
	lambda = 0.999;
	alpha = 0.1;
	training = LU;
	sEnv = true;
	am = (double)(1 << (14 - 1));
	skip = 0;
	fname = ("qadpd_error.log");
	fname2 = ("qadpd_coeff.log");
	uI_reg = 0;
	uQ_reg = 0;
	dI_reg = 0;
	dQ_reg = 0;
	a = 0;
	b = 0;
	a_ = 0;
	b_ = 0;
	xIe = 0;
	xIep = 0;
	xQe = 0;
	xQep = 0;
	A = 0;
	B = 0;
	index = 0;
	Ap = 0;
	Bp = 0;
	fp = 0;
	fp2 = 0;
	err = 0.0;
	aerr = 0.0;
	perr = 0.0;
	skiping = -1;
	updating = -1;
	enlog = false;
	ueps = 0.01;
}

qadpd::qadpd(int N, int M, int Nd, double G, double Lambda, double Am, int Skip)
{

	n = N;
	m = M;
	nd = Nd;
	g = G;
	lambda = Lambda;
	alpha = 0.1;
	training = LU;
	sEnv = true;
	am = Am;
	skip = Skip;
	fname = ("qadpd_error.log");
	fname2 = ("qadpd_coeff.log");
	uI_reg = 0;
	uQ_reg = 0;
	dI_reg = 0;
	dQ_reg = 0;
	a = 0;
	b = 0;
	a_ = 0;
	b_ = 0;
	xIe = 0;
	xIep = 0;
	xQe = 0;
	xQep = 0;
	A = 0;
	B = 0;
	index = 0;
	Ap = 0;
	Bp = 0;
	fp = 0;
	fp2 = 0;
	err = 0.0;
	aerr = 0.0;
	perr = 0.0;
	skiping = -1;
	updating = -1;
	enlog = false;
	ueps = 0.01;
}

void qadpd::release_memory()
{

	if (nd != 0)
	{
		if (uI_reg)
			delete[] uI_reg;
		if (uQ_reg)
			delete[] uQ_reg;
		if (dI_reg)
			delete[] dI_reg;
		if (dQ_reg)
			delete[] dQ_reg;
		uI_reg = 0;
		uQ_reg = 0;
		dI_reg = 0;
		dQ_reg = 0;
	}

	if (a)
		nrc::free_matrix(a, 0, n, 0, m);
	if (b)
		nrc::free_matrix(b, 0, n, 0, m);

	if (a_)
		nrc::free_matrix(a_, 0, n, 0, m);
	if (b_)
		nrc::free_matrix(b_, 0, n, 0, m);

	if (xIe)
		nrc::free_matrix(xIe, 0, n, 0, m);
	if (xIep)
		nrc::free_matrix(xIep, 0, n, 0, m);
	if (xQe)
		nrc::free_matrix(xQe, 0, n, 0, m);
	if (xQep)
		nrc::free_matrix(xQep, 0, n, 0, m);

	if (A)
		nrc::free_matrix(A, 1, 2 * (n + 1) * (m + 1), 1, 2 * (n + 1) * (m + 1));
	if (Ap)
		nrc::free_matrix(Ap, 1, 2 * (n + 1) * (m + 1), 1, 2 * (n + 1) * (m + 1));
	if (B)
		nrc::free_vector(B, 1, 2 * (n + 1) * (m + 1));
	if (Bp)
		nrc::free_vector(Bp, 1, 2 * (n + 1) * (m + 1));
	if (index)
		nrc::free_ivector(index, 1, 2 * (n + 1) * (m + 1));

	//if (fp)
	//	fclose(fp);
	//if (fp2)
	//	fclose(fp2);

	a = 0;
	b = 0;
	a_ = 0;
	b_ = 0;
	xIe = 0;
	xIep = 0;
	xQe = 0;
	xQep = 0;
	A = 0;
	B = 0;
	index = 0;
	Ap = 0;
	Bp = 0;
	fp = 0;
	fp2 = 0;
	skiping = -1;
	updating = -1;
}

qadpd::~qadpd()
{

	if (nd != 0)
	{
		if (uI_reg)
			delete[] uI_reg;
		if (uQ_reg)
			delete[] uQ_reg;
		if (dI_reg)
			delete[] dI_reg;
		if (dQ_reg)
			delete[] dQ_reg;
		uI_reg = 0;
		uQ_reg = 0;
		dI_reg = 0;
		dQ_reg = 0;
	}

	if (a)
		nrc::free_matrix(a, 0, n, 0, m);
	if (b)
		nrc::free_matrix(b, 0, n, 0, m);
	if (a_)
		nrc::free_matrix(a_, 0, n, 0, m);
	if (b_)
		nrc::free_matrix(b_, 0, n, 0, m);
	if (xIe)
		nrc::free_matrix(xIe, 0, n, 0, m);
	if (xIep)
		nrc::free_matrix(xIep, 0, n, 0, m);
	if (xQe)
		nrc::free_matrix(xQe, 0, n, 0, m);
	if (xQep)
		nrc::free_matrix(xQep, 0, n, 0, m);

	if (A)
		nrc::free_matrix(A, 1, 2 * (n + 1) * (m + 1), 1, 2 * (n + 1) * (m + 1));
	if (Ap)
		nrc::free_matrix(Ap, 1, 2 * (n + 1) * (m + 1), 1, 2 * (n + 1) * (m + 1));
	if (B)
		nrc::free_vector(B, 1, 2 * (n + 1) * (m + 1));
	if (Bp)
		nrc::free_vector(Bp, 1, 2 * (n + 1) * (m + 1));
	if (index)
		nrc::free_ivector(index, 1, 2 * (n + 1) * (m + 1));

	if (fp)
		fclose(fp);
	if (fp2)
		fclose(fp2);
	a = 0;
	b = 0;
	a_ = 0;
	b_ = 0;
	xIe = 0;
	xIep = 0;
	xQe = 0;
	xQep = 0;
	A = 0;
	B = 0;
	index = 0;
	Ap = 0;
	Bp = 0;
	fp = 0;
	fp2 = 0;
	skiping = -1;
	updating = -1;
}

void qadpd::reset_matrix()
{

	for (int i = 1; i <= 2 * (n + 1) * (m + 1); i++)
	{
		B[i] = Bp[i] = 0.0;
		index[i] = 0;
		for (int j = 1; j <= 2 * (n + 1) * (m + 1); j++)
			A[i][j] = Ap[i][j] = 0.0;
	}
}

void qadpd::prepare()
{
	for (int i = 0; i <= n; i++)
	{
		for (int j = 0; j <= m; j++)
		{
			xIe[i][j] = xIep[i][j] = 0.0;
			xQe[i][j] = xQep[i][j] = 0.0;
		}
	}
	if (nd != 0)
	{
		for (int k = 0; k < nd; k++)
		{
			dI_reg[k] = dQ_reg[k] = 0.0;
			uI_reg[k] = uQ_reg[k] = 0.0;
		}
	}
}

void qadpd::init(int N, int M, int Nd, double G, double Lambda, double Am, int Skip)
{

	n = N;
	m = M;
	nd = Nd;
	g = G;
	lambda = Lambda;
	alpha = 0.1;
	training = LU;
	sEnv = true;
	am = Am;
	skip = Skip;
	fname = ("qadpd_error.log");
	fname2 = ("qadpd_coeff.log");

	if (nd != 0)
	{
		dI_reg = new double[nd];
		dQ_reg = new double[nd];
		uI_reg = new double[nd];
		uQ_reg = new double[nd];

		for (int k = 0; k < nd; k++)
		{
			dI_reg[k] = dQ_reg[k] = 0.0;
			uI_reg[k] = uQ_reg[k] = 0.0;
		}
	}

	if (a == 0)
		a = nrc::matrix(0, n, 0, m);
	if (b == 0)
		b = nrc::matrix(0, n, 0, m);
	if (a_ == 0)
		a_ = nrc::matrix(0, n, 0, m);
	if (b_ == 0)
		b_ = nrc::matrix(0, n, 0, m);
	xIe = nrc::matrix(0, n, 0, m);
	xIep = nrc::matrix(0, n, 0, m);
	xQe = nrc::matrix(0, n, 0, m);
	xQep = nrc::matrix(0, n, 0, m);

	for (int i = 0; i <= n; i++)
	{
		for (int j = 0; j <= m; j++)
		{
			a[i][j] = b[i][j] = 0.0;
			a_[i][j] = b_[i][j] = 0.0;
			xIe[i][j] = xIep[i][j] = 0.0;
			xQe[i][j] = xQep[i][j] = 0.0;
		}
	}

	a[0][0] = 1.0;
	a_[0][0] = 1.0;
	A = nrc::matrix(1, 2 * (n + 1) * (m + 1), 1, 2 * (n + 1) * (m + 1));
	Ap = nrc::matrix(1, 2 * (n + 1) * (m + 1), 1, 2 * (n + 1) * (m + 1));
	B = nrc::vector(1, 2 * (n + 1) * (m + 1));
	Bp = nrc::vector(1, 2 * (n + 1) * (m + 1));
	index = nrc::ivector(1, 2 * (n + 1) * (m + 1));

	for (int i = 1; i <= 2 * (n + 1) * (m + 1); i++)
	{
		B[i] = Bp[i] = 0.0;
		index[i] = 0;
		for (int j = 1; j <= 2 * (n + 1) * (m + 1); j++)
			A[i][j] = Ap[i][j] = 0.0;
	}

	skiping = -1;
	updating = -1;

	if (enlog)
	{
		if (fname.length() > 0)
		{
			fp = fopen(fname.c_str(), "w");
		}
		if (fp)
		{
			fprintf(fp, "# 'error' 'amplitude error' 'phase error'\n");
			fprintf(fp, "# ----------------------------------------------------\n");
		}

		if (fp)
			fclose(fp);

		if (fname2.length() > 0)
		{
			fp2 = fopen(fname2.c_str(), "w");
		}
		if (fp2)
		{
			fprintf(fp2, "\n# QADPD coefficients:\n");
			fprintf(fp2, "# ----------------------------------------------------\n");
		}

		if (fp2)
			fclose(fp2);

		write_coeff();
	}
}

void qadpd::always(double XIp, double XQp, double XI, double XQ, double YIp, double YQp, bool Yp_FPGA)
{

	oeval(XIp, XQp, XI, XQ, YIp, YQp, Yp_FPGA);
	if (skiping == 0)
		train();
	if (skiping > 0)
	{
		skiping--;
	}
	else if (skiping == 0)
	{
		skiping = 0;
		if (updating > 0)
		{
			updating--;
		}
		else if (updating == 0)
		{
			write_coeff();
			write_error();
			updating = -1;
			skiping = -1;
		}
	}
}

void qadpd::write_coeff()
{
	if (enlog)
	{
		fname2 = ("qadpd_coeff.log");
		if (fname2.length() > 0)
		{
			fp2 = fopen(fname2.c_str(), "a");
		}
		if (fp2 == NULL)
			return;

		for (int i = 0; i <= n; i++)
		{
			fprintf(fp2, "# i = %d, a[%d][] = ", i, i);
			for (int j = 0; j <= m; j++)
			{
				fprintf(fp2, "%lg, ", a_[i][j]);
			}
			fprintf(fp2, "\n");
		}
		for (int i = 0; i <= n; i++)
		{
			fprintf(fp2, "# i = %d, b[%d][] = ", i, i);
			for (int j = 0; j <= m; j++)
			{
				fprintf(fp2, "%lg, ", b_[i][j]);
			}
			fprintf(fp2, "\n");
		}
		fprintf(fp2, "\n");
		if (fp2)
			fclose(fp2);
		fp2 = NULL;
	}
}

void qadpd::write_error()
{
	if (enlog)
	{
		fname = ("T800_error.log");
		if (fname.length() > 0)
		{
			fp = fopen(fname.c_str(), "a");
		}
		if (fp == NULL)
			return;
		fprintf(fp, "%lg %lg %lg\n", err, aerr, perr);

		if (fp)
			fclose(fp);
		fp = NULL;
	}
}

void qadpd::start()
{
	if (enlog)
	{
	}
}

void qadpd::finish()
{
	if (enlog)
	{
	}
}

void qadpd::oeval(double XIp, double XQp, double XI, double XQ, double YIp, double YQp, bool Yp_FPGA)
{
	double YpI, YpQ;
	double e, ep;
	double dI, dQ;

	e = XI * XI + XQ * XQ;
	e /= am * am;
	if (sEnv == false)
		e = sqrt(e);
	ep = XIp * XIp + XQp * XQp;
	ep /= am * am;
	if (sEnv == false)
		ep = sqrt(ep);

	for (int i = n; i > 0; i--)
	{
		for (int j = 0; j <= m; j++)
		{
			xIe[i][j] = xIe[i - 1][j];
			xIep[i][j] = xIep[i - 1][j];
			xQe[i][j] = xQe[i - 1][j];
			xQep[i][j] = xQep[i - 1][j];
		}
	}
	xIe[0][0] = XI;
	xIep[0][0] = XIp;
	xQe[0][0] = XQ;
	xQep[0][0] = XQp;

	for (int j = 1; j <= m; j++)
	{
		xIe[0][j] = xIe[0][j - 1] * e;
		xIep[0][j] = xIep[0][j - 1] * ep;
		xQe[0][j] = xQe[0][j - 1] * e;
		xQep[0][j] = xQep[0][j - 1] * ep;
	}

	YpI = YpQ = yI = yQ = 0;
	for (int i = 0; i <= n; i++)
	{
		for (int j = 0; j <= m; j++)
		{
			yI += a[i][j] * xIe[i][j] - b[i][j] * xQe[i][j];
			yQ += a[i][j] * xQe[i][j] + b[i][j] * xIe[i][j];
			YpI += a[i][j] * xIep[i][j] - b[i][j] * xQep[i][j];
			YpQ += a[i][j] * xQep[i][j] + b[i][j] * xIep[i][j];
		}
	}

	if (yI > (am - 1))
		yI = (am - 1);
	if (yI < (0 - am))
		yI = (0 - am);

	if (yQ > (am - 1))
		yQ = (am - 1);
	if (yQ < (0 - am))
		yQ = (0 - am);

	ypi = 0;
	ypq = 0;

	double YpI_ = 0;
	double YpQ_ = 0;

	if (Yp_FPGA == true)
	{
		ypi = (g * YIp);
		ypq = (g * YQp);
		YpI_ = (YIp);
		YpQ_ = (YQp);
	}
	else
	{
		ypi = (g * YpI);
		ypq = (g * YpQ);
		YpI_ = YpI;
		YpQ_ = YpQ;
	}

	if (nd == 0)
	{
		dI = XIp;
		dQ = XQp;
		uI = YpI;
		uQ = YpQ;
	}
	else
	{
		dI = dI_reg[nd - 1];
		for (int k = nd - 1; k >= 1; k--)
			dI_reg[k] = dI_reg[k - 1];
		dI_reg[0] = XIp;
		dQ = dQ_reg[nd - 1];
		for (int k = nd - 1; k >= 1; k--)
			dQ_reg[k] = dQ_reg[k - 1];
		dQ_reg[0] = XQp;
		uI = uI_reg[nd - 1];
		for (int k = nd - 1; k >= 1; k--)
			uI_reg[k] = uI_reg[k - 1];
		uI_reg[0] = YpI_;
		uQ = uQ_reg[nd - 1];
		for (int k = nd - 1; k >= 1; k--)
			uQ_reg[k] = uQ_reg[k - 1];
		uQ_reg[0] = YpQ_;
	}

	err = fabs(sqrt((yI - uI) * (yI - uI) + (yQ - uQ) * (yQ - uQ))) / am;
	aerr = fabs(sqrt(XI * XI + XQ * XQ) - sqrt(dI * dI + dQ * dQ)) / am;
	perr = 180.0 * atan2(-XI * dQ + XQ * dI, XI * dI + XQ * dQ) / M_PI;

	uenv = fabs(sqrt(uI * uI + uQ * uQ)) / am;
}

int qadpd::update_coeff(double range)
{

	double a1 = 0.0;
	double b1 = 0.0;
	double eps = 0.002;
	int temp = 0;
	double K = 0.0; //

	for (int i = 0; i <= n; i++)
	{
		for (int j = 0; j <= m; j++)
		{

			a1 = a_[i][j];
			b1 = b_[i][j];

			K = (double)pow(2, j);

			if (a1 > range * K)
			{
				a1 = (range * K - eps);
				temp = -1;
			}
			if (a1 < (-range * K))
			{
				a1 = (-range * K + eps);
				temp = -1;
			}
			if (b1 > range * K)
			{
				b1 = range * K - eps;
				temp = -1;
			}
			if (b1 < (-range * K))
			{
				b1 = (-range * K + eps);
				temp = -1;
			}

			a[i][j] = a1;
			b[i][j] = b1;
		}
	}
	return temp;
}

void qadpd::reset_coeff()
{
	for (int i = 0; i <= n; i++)
	{
		for (int j = 0; j <= m; j++)
		{
			a[i][j] = b[i][j] = 0.0;
			a_[i][j] = b_[i][j] = 0.0;
			xIe[i][j] = xIep[i][j] = 0.0;
			xQe[i][j] = xQep[i][j] = 0.0;
		}
	}
	a[0][0] = 1.0;
	a_[0][0] = 1.0;
}

void qadpd::train()
{

	int ij, kl;
	double dd = 0.0;

	if (uenv < ueps)
		return;

	for (int k = 0; k <= n; k++)
	{
		for (int l = 0; l <= m; l++)
		{
			kl = k + (n + 1) * l;

			Bp[1 + kl] = lambda * Bp[1 + kl] + uI * xIe[k][l] / am / am + uQ * xQe[k][l] / am / am;

			B[1 + kl] = Bp[1 + kl];
			Bp[1 + kl + (n + 1) * (m + 1)] = lambda * Bp[1 + kl + (n + 1) * (m + 1)] - uI * xQe[k][l] / am / am + uQ * xIe[k][l] / am / am;
			B[1 + kl + (n + 1) * (m + 1)] = Bp[1 + kl + (n + 1) * (m + 1)];
			for (int i = 0; i <= n; i++)
			{
				for (int j = 0; j <= m; j++)
				{
					ij = i + (n + 1) * j;
					Ap[1 + kl][1 + ij] = lambda * Ap[1 + kl][1 + ij] +
										 xIe[i][j] * xIe[k][l] / am / am +
										 xQe[i][j] * xQe[k][l] / am / am;
					A[1 + kl][1 + ij] = Ap[1 + kl][1 + ij];

					Ap[1 + kl][1 + ij + (n + 1) * (m + 1)] =
						lambda * Ap[1 + kl][1 + ij + (n + 1) * (m + 1)] -
						xQe[i][j] * xIe[k][l] / am / am +
						xIe[i][j] * xQe[k][l] / am / am;
					A[1 + kl][1 + ij + (n + 1) * (m + 1)] = Ap[1 + kl][1 + ij + (n + 1) * (m + 1)];

					Ap[1 + kl + (n + 1) * (m + 1)][1 + ij] =
						lambda * Ap[1 + kl + (n + 1) * (m + 1)][1 + ij] -
						xIe[i][j] * xQe[k][l] / am / am +
						xQe[i][j] * xIe[k][l] / am / am;
					A[1 + kl + (n + 1) * (m + 1)][1 + ij] = Ap[1 + kl + (n + 1) * (m + 1)][1 + ij];

					Ap[1 + kl + (n + 1) * (m + 1)][1 + ij + (n + 1) * (m + 1)] =
						lambda * Ap[1 + kl + (n + 1) * (m + 1)][1 + ij + (n + 1) * (m + 1)] +
						xQe[i][j] * xQe[k][l] / am / am +
						xIe[i][j] * xIe[k][l] / am / am;
					A[1 + kl + (n + 1) * (m + 1)][1 + ij + (n + 1) * (m + 1)] =
						Ap[1 + kl + (n + 1) * (m + 1)][1 + ij + (n + 1) * (m + 1)];
				}
			}
		}
	}

	if (updating == 0)
	{
		if (training == LU)
		{
			nrc::ludcmp(A, 2 * (n + 1) * (m + 1), index, &dd);
			nrc::lubksb(A, 2 * (n + 1) * (m + 1), index, B);
		}
		else
		{
			for (int i = 0; i <= n; i++)
			{
				for (int j = 0; j <= m; j++)
				{
					B[1 + i + (n + 1) * j] = a[i][j];
					B[1 + i + (n + 1) * j + (n + 1) * (m + 1)] = b[i][j];
				}
			}
			if (training == GRAD)
				nrc::lgrad(A, Bp, B, 2 * (n + 1) * (m + 1), alpha);
			else if (training == GS)
				nrc::gauss_seidel(A, Bp, B, 2 * (n + 1) * (m + 1));
		}

		for (int i = 0; i <= n; i++)
		{
			for (int j = 0; j <= m; j++)
			{
				a_[i][j] = B[1 + i + (n + 1) * j];
				b_[i][j] = B[1 + i + (n + 1) * j + (n + 1) * (m + 1)];
			}
		}
	}
}
