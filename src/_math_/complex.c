/*****************************************************************************/
/*                                                  ###                      */
/*       #####          ###    ###                  ###  CREATE: 2019-02-09  */
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
/*  >>>>>>>>>>>>>>>>>>>>>>>> CrHack 复数计算函数库 <<<<<<<<<<<<<<<<<<<<<<<<  */
/*  =======================================================================  */
/*****************************************************************************/

#include "phylib.h"

#ifndef _CR_NO_STDC_
    #include <math.h>
#endif

/*
=======================================
    复数的模
=======================================
*/
CR_API double
complex_abs (
  __CR_IN__ const sCOMPLEX* c
    )
{
    return (DSQRT(c->re * c->re + c->im * c->im));
}

/*
=======================================
    复数相位
=======================================
*/
CR_API double
complex_ang (
  __CR_IN__ const sCOMPLEX* c
    )
{
    return (DATAN2(c->im, c->re));
}

/*
=======================================
    复数指数
=======================================
*/
CR_API sCOMPLEX*
complex_exp (
  __CR_OT__ sCOMPLEX*       r,
  __CR_IN__ const sCOMPLEX* c
    )
{
    r->re = DEXP(c->re) * DCOS(c->im);
    r->im = DEXP(c->re) * DSIN(c->im);
    return (r);
}

/*
=======================================
    复数合成
=======================================
*/
CR_API sCOMPLEX*
complex_mak (
  __CR_OT__ sCOMPLEX*   r,
  __CR_IN__ double      length,
  __CR_IN__ double      theta
    )
{
    r->re = length * DCOS(theta);
    r->im = length * DSIN(theta);
    return (r);
}

/*
=======================================
    复数加法
=======================================
*/
CR_API sCOMPLEX*
complex_add (
  __CR_OT__ sCOMPLEX*       r,
  __CR_IN__ const sCOMPLEX* c1,
  __CR_IN__ const sCOMPLEX* c2
    )
{
    r->re = c1->re + c2->re;
    r->im = c1->im + c2->im;
    return (r);
}

/*
=======================================
    复数减法
=======================================
*/
CR_API sCOMPLEX*
complex_sub (
  __CR_OT__ sCOMPLEX*       r,
  __CR_IN__ const sCOMPLEX* c1,
  __CR_IN__ const sCOMPLEX* c2
    )
{
    r->re = c1->re - c2->re;
    r->im = c1->im - c2->im;
    return (r);
}

/*
=======================================
    复数乘法
=======================================
*/
CR_API sCOMPLEX*
complex_mul (
  __CR_OT__ sCOMPLEX*       r,
  __CR_IN__ const sCOMPLEX* c1,
  __CR_IN__ const sCOMPLEX* c2
    )
{
    double  aa, bb;

    aa = c1->re * c2->re - c1->im * c2->im;
    bb = c1->im * c2->re + c1->re * c2->im;
    r->re = aa;
    r->im = bb;
    return (r);
}

/*
=======================================
    复数除法
=======================================
*/
CR_API sCOMPLEX*
complex_div (
  __CR_OT__ sCOMPLEX*       r,
  __CR_IN__ const sCOMPLEX* c1,
  __CR_IN__ const sCOMPLEX* c2
    )
{
    double  aa, bb, cc;

    aa = c1->re * c2->re + c1->im * c2->im;
    bb = c1->im * c2->re - c1->re * c2->im;
    cc = c2->re * c2->re + c2->im * c2->im;
    r->re = aa / cc;
    r->im = bb / cc;
    return (r);
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
