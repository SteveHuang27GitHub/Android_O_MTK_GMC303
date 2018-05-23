#ifndef __softiron_h
#define __softiron_h

//SM11
//#define SOFT_IRON_MATRIX {0.768564,  0.018403,  -0.038208,  0.026433,  1.210734,  0.004722,  -0.097869,  -0.000596,  1.081984}

/* [SMx1 O] S-BUG#2769 Grace_Chang [Sensor] E-compass softiron compensate */
// 20180110
#define SOFT_IRON_MATRIX_SM11 {0.735764,  0.001873,-0.061656, 0.02721, 1.233678, 0.005542,-0.111157,-0.023321,1.113230}
#define SOFT_IRON_MATRIX_SM21 {0.951419, -0.121969,-0.01202, -0.126487,0.940907,-0.049949, 0.034304,-0.020772,1.134147}
#define SOFT_IRON_MATRIX_SM11L {0.983148,-0.042868,-0.062865,-0.021148,0.897361,-0.027823, 0.028269, 0.002135,1.133766}
/* [SMx1 O] E-BUG#2769 Grace_Chang [Sensor] E-compass softiron compensate */

//Identity
#define SOFT_IRON_MATRIX {1,0,0,0,1,0,0,0,1}
#define SOFT_IRON_Version 1.0

#endif
