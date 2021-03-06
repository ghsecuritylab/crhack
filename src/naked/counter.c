/*****************************************************************************/
/*                                                  ###                      */
/*       #####          ###    ###                  ###  CREATE: 2017-03-04  */
/*     #######          ###    ###      [CORE]      ###  ~~~~~~~~~~~~~~~~~~  */
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
/*  >>>>>>>>>>>>>>>>>>>> CrHack 定时器函数库 for Naked <<<<<<<<<<<<<<<<<<<<  */
/*  =======================================================================  */
/*****************************************************************************/

#include "applib.h"
#include "memlib.h"

/* 需要在1毫秒计数器里计时 */
extern int32u   g_tick_ms;

#if !defined(_CR_NAKED_NO_MALLOC_)
/*
=======================================
    创建定时器
=======================================
*/
CR_API xtime_t
timer_new (void_t)
{
    int32u* timer;

    timer = struct_new(int32u);
    if (timer == NULL)
        return (NULL);
    *timer = 0;
    return ((xtime_t)timer);
}

/*
=======================================
    释放定时器
=======================================
*/
CR_API void_t
timer_del (
  __CR_IN__ xtime_t timer
    )
{
    mem_free(timer);
}

#endif  /* !_CR_NAKED_NO_MALLOC_ */

/*
=======================================
    获取系统计时 (低精度)
=======================================
*/
CR_API int32u
timer_get32 (void_t)
{
    return (g_tick_ms);
}

/*
=======================================
    获取系统计时 (高精度)
=======================================
*/
CR_API int64u
timer_get64 (void_t)
{
    return (g_tick_ms);
}

#if !defined(_CR_NAKED_NO_MALLOC_)
/*
=======================================
    设置定时器基数
=======================================
*/
CR_API void_t
timer_set_base (
  __CR_IN__ xtime_t timer
    )
{
    *(int32u*)timer = timer_get32();
}

/*
=======================================
    获取定时器时差 (ms)
=======================================
*/
CR_API fp32_t
timer_get_delta (
  __CR_IN__ xtime_t timer
    )
{
    int32u  base = *(int32u*)timer;
    int32u  now = timer_get32();

    if (rarely(now < base))
        return ((fp32_t)(0xFFFFFFFFUL - base + now + 1));
    return ((fp32_t)(now - base));
}

#endif  /* !_CR_NAKED_NO_MALLOC_ */

/*****************************************************************************/
/* _________________________________________________________________________ */
/* uBMAzRBoAKAHaACQD6FoAIAPqbgA/7rIA+5CM9uKw8D4Au7u7mSIJ0t18mYz0mYz9rAQZCgHc */
/* wRIZIgHZovGBXUAZg+v0GbB+gRmgcJ7BAAAisIlAwB1Av7LSHUC/s9IdQL+w0h1Av7HZkZmgf */
/* 4JLgIAdb262gPsqAh0+zP/uQB9ZYsFZYktq+L3sMi/AAK7gAKJAUtLdfq5IANXvT8BiQzfBIv */
/* FLaAAweAEmff53wb+Adjx3kQE2xwy5Io8ithkigcFgACJBN8E3sneNvwB2xyLHDkdfA2JHSyA */
/* adtAAQPdZYgHR0dNdbZfSYP5UHWr/kQEtAHNFg+Eef/DWAKgDw== |~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>> END OF FILE <<< */
/*****************************************************************************/
