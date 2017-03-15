/*****************************************************************************/
/*                                                  ###                      */
/*       #####          ###    ###                  ###  CREATE: 2017-03-13  */
/*     #######          ###    ###      [HARD]      ###  ~~~~~~~~~~~~~~~~~~  */
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
/*  >>>>>>>>>>>>>>>>>> RFGEO-SRV-2 采集器系统调用函数库 <<<<<<<<<<<<<<<<<<<  */
/*  =======================================================================  */
/*****************************************************************************/

#include "applib.h"
#include "device.h"
#include "devlib.h"
#include "stm32f10x_conf.h"

/*****************************************************************************/
/*                                   RTC                                     */
/*****************************************************************************/

/*
=======================================
    RTC 初始化
=======================================
*/
CR_API void_t
rtc_init (void_t)
{
    uint_t  idx;

    /* 重试几次 */
    for (idx = 0; idx < 5; idx++) {
        if (rx8025_init_time())
            break;
    }
}

/*
=======================================
    获取系统时间
=======================================
*/
CR_API bool_t
datetime_get (
  __CR_OT__ sDATETIME*  datetime
    )
{
    uint_t  idx;

    /* 重试几次 */
    for (idx = 0; idx < 5; idx++) {
        if (rx8025_get_time(datetime))
            return (TRUE);
    }
    return (FALSE);
}

/*
=======================================
    设置系统时间
=======================================
*/
CR_API bool_t
datetime_set (
  __CR_IN__ const sDATETIME*    datetime
    )
{
    uint_t  idx;

    /* 强制设置星期 */
    if (!datetime_chk(datetime))
        return (FALSE);
    date_set_week((sDATETIME*)datetime);

    /* 重试几次 */
    for (idx = 0; idx < 5; idx++) {
        if (rx8025_set_time(datetime))
            return (TRUE);
    }
    return (FALSE);
}

/*****************************************************************************/
/*                                   MISC                                    */
/*****************************************************************************/

/*
=======================================
    延时一段时间
=======================================
*/
CR_API void_t
delayms (
  __CR_IN__ byte_t  num
    )
{
    int32u  time = (int32u)num;
    int32u  base = timer_get32();

    while (timer_delta32(base) < time);
}

/*
=======================================
    延时一段时间
=======================================
*/
CR_API void_t
thread_sleep (
  __CR_IN__ uint_t  time_ms
    )
{
    byte_t  led_flag = FALSE;
    int32u  led_base = timer_get32();
    int32u  base = led_base;

    for (;;)
    {
        /* 等待时间到 */
        if (timer_delta32(base) >= time_ms)
            break;

        /* LED 与喂狗 */
        if (timer_delta32(led_base) >= 333) {
            led_base = timer_get32();
            if (led_flag)
                led_xon();
            else
                led_off();
            led_flag = !led_flag;
            WDT_FEED;
        }
    }
}

/*
=======================================
    加载串口库
=======================================
*/
CR_API void_t
sio_init (void_t)
{
}

/*
=======================================
    释放串口库
=======================================
*/
CR_API void_t
sio_free (void_t)
{
}

/*****************************************************************************/
/*                                   NET                                     */
/*****************************************************************************/

/* 是否通过桥接板 */
bool_t  g_is_bridge = FALSE;

/*
=======================================
    初始化网络库
=======================================
*/
CR_API bool_t
socket_init (void_t)
{
    /* 桥接板判断 */
    /**************/
    return (TRUE);
}

/*
=======================================
    释放网络库
=======================================
*/
CR_API void_t
socket_free (void_t)
{
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
