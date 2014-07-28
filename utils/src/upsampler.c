/*******************************************************
 * Created: Jul 23, 2014
 * Brief:   Copid from Jong's ecei.c.
 *******************************************************/

#include "upsampler.h"

/*
 * Bicubic interpolation functions
 */
static double cubicInterpolate(double p[4], double x)
{
  return p[1] + 0.5 * x*(p[2] - p[0] + 
			 x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + 
			    x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

static double bicubicInterpolate (double p[4][4], double x, double y) 
{
  double arr[4];
  arr[0] = cubicInterpolate(p[0], y);
  arr[1] = cubicInterpolate(p[1], y);
  arr[2] = cubicInterpolate(p[2], y);
  arr[3] = cubicInterpolate(p[3], y);
  return cubicInterpolate(arr, x);
}


void upsampler_upsample(size_t dimy, size_t dimx, double X[][dimx], 
			size_t scale, size_t dimy2, size_t dimx2, double Y[][dimx2])
{
  assert(dimy2 == scale*dimy);
  assert(dimx2 == scale*dimx);

  double p[4][4];

  for (size_t j=0; j<dimy; j++) 
    for (size_t i=0; i<dimx; i++)
      for (size_t jk=0; jk<scale; jk++)
	for (size_t ik=0; ik<scale; ik++)
	  {
	    size_t jj = j*scale + jk;
	    size_t ii = i*scale + ik;

	    for (size_t t2=0; t2<4; t2++)
	      for (size_t t1=0; t1<4; t1++)
		{
		  int64_t o1 = i + t1 - 1;
		  int64_t o2 = j + t2 - 1;

		  if (o1 < 0) o1=0;
		  if (o2 < 0) o2=0;
		  if (o1 > dimx-1) o1 = dimx;
		  if (o2 > dimy-1) o2 = dimy;

		  p[t2][t1] = X[o2][o1];
		}
                    
	    //DUMP("%ld %ld", jj, ii);
	    Y[jj][ii] = bicubicInterpolate(p, (double)jk/scale, (double)ik/scale);
	  }
}
