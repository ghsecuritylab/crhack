/*****************************************************************************/
/*                                                  ###                      */
/*       #####          ###    ###                  ###  CREATE: 2012-01-13  */
/*     #######          ###    ###      [MATH]      ###  ~~~~~~~~~~~~~~~~~~  */
/*    ########          ###    ###                  ###  MODIFY: XXXX-XX-XX  */
/*    ####  ##          ###    ###                  ###  ~~~~~~~~~~~~~~~~~~  */
/*   ###       ### ###  ###    ###    ####    ####  ###   ##  +-----------+  */
/*  ####       ######## ##########  #######  ###### ###  ###  |  A NEW C  |  */
/*  ###        ######## ########## ########  ###### ### ###   | FRAMEWORK |  */
/*  ###     ## #### ### ########## ###  ### ###     ######    |  FOR ALL  |  */
/*  ####   ### ###  ### ###    ### ###  ### ###     ######    | PLATFORMS |  */
/*  ########## ###      ###    ### ######## ####### #######   |  AND ALL  |  */
/*   #######   ###      ###    ### ########  ###### ###  ###  | COMPILERS |  */
/*    #####    ###      ###    ###  #### ##   ####  ###   ##  +-----------+  */
/*  =======================================================================  */
/*  >>>>>>>>>>>>>>>>>>>>> CrHack IAPWS-IF97 一区函数库 <<<<<<<<<<<<<<<<<<<<  */
/*  =======================================================================  */
/*****************************************************************************/

#include "phylib.h"

#ifndef _CR_NO_STDC_
    #include <math.h>
#endif

/* IAPWS-IF97 一区（液态区）计算常数表 */
static const sint_t _rom_ s_iif97_ii[34] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    3, 3, 3, 4, 4, 4, 5, 8, 8, 21, 23, 29, 30, 31, 32,
};

static const sint_t _rom_ s_iif97_ji[34] =
{
    -2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1, 3, -3, 0, 1, 3, 17,
    -4, 0, 6, -5, -2, 10, -8, -11, -6, -29, -31, -38, -39, -40, -41,
};

static const double _rom_ s_iif97_ni[34] =
{
     0.14632971213167E+00, -0.84548187169114E+00, -0.37563603672040E+01,
     0.33855169168385E+01, -0.95791963387872E+00,  0.15772038513228E+00,
    -0.16616417199501E-01,  0.81214629983568E-03,  0.28319080123804E-03,
    -0.60706301565874E-03, -0.18990068218419E-01, -0.32529748770505E-01,
    -0.21841717175414E-01, -0.52838357969930E-04, -0.47184321073267E-03,
    -0.30001780793026E-03,  0.47661393906987E-04, -0.44141845330846E-05,
    -0.72694996297594E-15, -0.31679644845054E-04, -0.28270797985312E-05,
    -0.85205128120103E-09, -0.22425281908000E-05, -0.65171222895601E-06,
    -0.14341729937924E-12, -0.40516996860117E-06, -0.12734301741641E-08,
    -0.17424871230634E-09, -0.68762131295531E-18,  0.14478307828521E-19,
     0.26335781662795E-22, -0.11947622640071E-22,  0.18228094581404E-23,
    -0.93537087292458E-25,
};

/*
=======================================
    IAPWS-IF97 一区密度计算
=======================================
*/
CR_API double
iif97_1_ws_d (
  __CR_IN__ double  t,
  __CR_IN__ double  mpa
    )
{
    double  xx;
    double  vvv;
    ufast_t idx;

    xx = 0.0;
    t += 273.15;
    mpa = 7.1 - mpa / 16.53;
    for (vvv = 1386.0 / t - 1.222, idx = 0; idx < 34; idx++) {
        xx -= s_iif97_ni[idx] * s_iif97_ii[idx] *
                  DPOW(mpa, s_iif97_ii[idx] - 1) *
                  DPOW(vvv, s_iif97_ji[idx]);
    }
    return (16530.0 / (0.461526 * t * xx));
}

/*
=======================================
    IAPWS-IF97 一区热焓计算
=======================================
*/
CR_API double
iif97_1_ws_e (
  __CR_IN__ double  t,
  __CR_IN__ double  mpa
    )
{
    double  xx;
    double  vvv;
    ufast_t idx;

    xx = 0.0;
    t += 273.15;
    mpa = 7.1 - mpa / 16.53;
    for (vvv = 1386.0 / t - 1.222, idx = 0; idx < 34; idx++) {
        xx += s_iif97_ni[idx] * s_iif97_ji[idx] *
                  DPOW(mpa, s_iif97_ii[idx]) *
                  DPOW(vvv, s_iif97_ji[idx] - 1);
    }
    return (1386.0 * 0.461526 / 4.187 * xx);
}

/*****************************************************************************/
/* _________________________________________________________________________ */
/* uBMAzRBoAKAHaACQD6FoAIAPqbgA/7rIA+5CM9uKw8D4Au7u7mSIJ0t18mYz0mYz9rAQZCgHc */
/* wRIZIgHZovGBXUAZg+v0GbB+gRmgcJ7BAAAisIlAwB1Av7LSHUC/s9IdQL+w0h1Av7HZkZmgf */
/* 4JLgIAdb262gPsqAh0+zP/uQB9ZYsFZYktq+L3sMi/AAK7gAKJAUtLdfq5IANXvT8BiQzfBIv */
/* FLaAAweAEmff53wb+Adjx3kQE2xwy5Io8ithkigcFgACJBN8E3sneNvwB2xyLHDkdfA2JHSyA */
/* adtAAQPdZYgHR0dNdbZfSYP5UHWr/kQEtAHNFg+Eef/DWAKgDw== |~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>> END OF FILE <<< */
/*****************************************************************************/
