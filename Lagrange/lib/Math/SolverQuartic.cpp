#include "SolverQuartic.hpp"
#include <stdexcept>
#include <cmath>

using namespace std;

SolverQuartic::SolverQuartic()
{
}

SolverQuartic::~SolverQuartic()
{
}

SolverQuartic::Roots SolverQuartic::Run(double a, double b, double c, double d, double e)
{

	if (a == 0)
	{
		throw runtime_error("The coefficient of the power four of x is 0. Please use the utility for a third degree quadratic.");
	}
	if (e == 0)
	{
		throw runtime_error("One root is 0. Now divide through by x and use the utility for a third degree quadratic to solve the resulting equation for the other three roots.");
	}
	if (a != 1)
	{
		b /= a;
		c /= a;
		d /= a;
		e /= a;
	}

// Coefficients for cubic solver
	double cb, cc, cd;
	double discrim, q, r, RRe, RIm, DRe, DIm, dum1, ERe, EIm, s, t, term1, r13, sqR, y1, z1Re, z1Im, z2Re;
	cb = -c;
	cc = -4.0*e + d*b;
	cd = -(b*b*e + d*d) + 4.0*c*e;
	if (cd == 0)
	{
		//alert("cd = 0.");
	}
	q = (3.0*cc - (cb*cb))/9.0;
	r = -(27.0*cd) + cb*(9.0*cc - 2.0*(cb*cb));
	r /= 54.0;
	discrim = q*q*q + r*r;
	term1 = (cb/3.0);
	if (discrim > 0)
	{
// 1 root real, 2 are complex
		s = r + sqrt(discrim);
		s = ((s < 0) ? -pow(-s, (1.0/3.0)) : pow(s, (1.0/3.0)));
		t = r - sqrt(discrim);
		t = ((t < 0) ? -pow(-t, (1.0/3.0)) : pow(t, (1.0/3.0)));
		y1 = -term1 + s + t;
		}
		else
		{
			if (discrim == 0)
			{
				r13 = ((r < 0) ? -pow(-r,(1.0/3.0)) : pow(r,(1.0/3.0)));
				y1 = -term1 + 2.0*r13;
			}
			else
			{
				q = -q;
				dum1 = q*q*q;
				dum1 = acos(r/sqrt(dum1));
				r13 = 2.0*sqrt(q);
				y1 = -term1 + r13*cos(dum1/3.0);
			}
		}
// Determined y1, a real root of the resolvent cubic.
		term1 = b/4.0;
		sqR = -c + term1*b + y1;
		RRe = RIm = DRe = DIm = ERe = EIm = z1Re = z1Im = z2Re = 0;
		if (sqR >= 0)
		{
			if (sqR == 0)
			{
				dum1 = -(4.0*e) + y1*y1;
				if (dum1 < 0) //D and E will be complex
	   			z1Im = 2.0*sqrt(-dum1);
				else
				{                      //else (dum1 >= 0)
	  				z1Re = 2.0*sqrt(dum1);
					z2Re = -z1Re;
				}
			}
			else
			{
			RRe = sqrt(sqR);
			z1Re = -(8.0*d + b*b*b)/4.0 + b*c;
			z1Re /= RRe;
			z2Re = -z1Re;
		 }
	}
	else
	{
		RIm = sqrt(-sqR);
		z1Im = -(8.0*d + b*b*b)/4.0 + b*c;
		z1Im /= RIm;
		z1Im = -z1Im;
	}
	z1Re += -(2.0*c + sqR) + 3.0*b*term1;
	z2Re += -(2.0*c + sqR) + 3.0*b*term1;

//At this point, z1 and z2 should be the terms under the square root for D and E
	if (z1Im == 0)
	{               // Both z1 and z2 real
		if (z1Re >= 0)
		{
			DRe = sqrt(z1Re);
		}
		else
		{
			DIm = sqrt(-z1Re);
		}
		if (z2Re >= 0)
		{
			ERe = sqrt(z2Re);
		}
		else
		{
			EIm = sqrt(-z2Re);
		}
	}
	else
	{
		r = sqrt(z1Re*z1Re + z1Im*z1Im);
		r = sqrt(r);
		dum1 = atan2(z1Im, z1Re);
		dum1 /= 2; //Divide this angle by 2
		ERe = DRe = r*cos(dum1);
		DIm = r*sin(dum1);
		EIm = -DIm;
	}

	std::array<std::complex<double>, 4> ret;
    int i = 0;
	ret.at(i++) = std::complex<double>(-term1 + (RRe + DRe)/2,   (RIm + DIm)/2);
	ret.at(i++) = std::complex<double>(-(term1 + DRe/2) + RRe/2, (-DIm + RIm)/2);
	ret.at(i++) = std::complex<double>(-(term1 + RRe/2) + ERe/2, (-RIm + EIm)/2);
	ret.at(i++) = std::complex<double>(-(term1 + (RRe + ERe)/2), -(RIm + EIm)/2);
	return ret;
}
