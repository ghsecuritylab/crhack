/*****************************************************************************/
/*                                                  ###                      */
/*       #####          ###    ###                  ###  CREATE: 2017-03-16  */
/*     #######          ###    ###      [KRNL]      ###  ~~~~~~~~~~~~~~~~~~  */
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
/*  >>>>>>>>>>>>>>>>> CrHack SIMCOM 3G/4G 模块驱动函数库 <<<<<<<<<<<<<<<<<<  */
/*  =======================================================================  */
/*****************************************************************************/

#include "applib.h"
#include "devlib.h"
#include "memlib.h"
#include "strlib.h"

/* 默认定义 */
#ifndef SIMCOM_YIELD
    #define SIMCOM_YIELD
#endif
#ifndef SIMCOM_AT_TOUT
    #define SIMCOM_AT_TOUT  1000
#endif
#ifndef SIMCOM_DEF_TOUT
    #define SIMCOM_DEF_TOUT 5100
#endif
#ifndef SIMCOM_BUF_SIZE
    #define SIMCOM_BUF_SIZE CR_K2B(8)
#endif

/* 内部结构 */
typedef struct
{
        uint_t  linknum;        /* 连接编号 */
        int32u  recv_tout;      /* 接收超时 */
        int32u  send_tout;      /* 发送超时 */
        int16u  remote_port;    /* 远程端口 */
        ansi_t* remote_addr;    /* 远程地址 */
        uint_t  head, tail;     /* 队列头尾 */
        byte_t* fifo_buffer;    /* 队列数据 */

} sSIMCOM;

/* 最多10个连接 */
static sSIMCOM  s_simcom[10];

/* 一些有用的宏 */
#define SIMCOM_TIMEOUT(x)   ((x) +  100)
#define SIMCOM_TCP_PORT(x)  ((x) + 6500)
#define SIMCOM_UDP_PORT(x)  ((x) + 6600)

/*****************************************************************************/
/*                                 公用部分                                  */
/*****************************************************************************/

/*
=======================================
    初始化网络库
=======================================
*/
CR_API bool_t
simcom_socket_init (void_t)
{
    ufast_t idx;

    mem_zero(s_simcom, sizeof(s_simcom));
    for (idx = 0; idx < cntsof(s_simcom); idx++) {
        s_simcom[idx].linknum = idx;
        s_simcom[idx].recv_tout = 120000UL;
        s_simcom[idx].send_tout = 120000UL;
    }
    return (TRUE);
}

/*
=======================================
    释放网络库
=======================================
*/
CR_API void_t
simcom_socket_free (void_t)
{
    ufast_t idx;

    for (idx = 0; idx < cntsof(s_simcom); idx++)
        simcom_socket_close((socket_t)(&s_simcom[idx]));
    at_send("AT+NETCLOSE\r", 12);
}

/*
---------------------------------------
    查找空闲的接连
---------------------------------------
*/
static sint_t
simcom_socket_find (void_t)
{
    ufast_t idx;

    for (idx = 0; idx < cntsof(s_simcom); idx++) {
        if (s_simcom[idx].fifo_buffer == NULL)
            return (idx);
    }
    return (-1);
}

/*
---------------------------------------
    获取本地 IP 地址
---------------------------------------
*/
#if 0
static bool_t
simcom_socket_ip (
  __CR_OT__ ansi_t* ip,
  __CR_IN__ ansi_t* ret,
  __CR_IN__ uint_t  size
    )
{
    ansi_t* bak;
    ufast_t idx;

    /* 获取本地 IP 地址 */
    bak = at_iorw(ret, size, "AT+IPADDR\r", SIMCOM_AT_TOUT);
    if (bak == NULL)
        return (FALSE);
    bak = str_strA(bak, "+IPADDR: ");
    if (bak == NULL)
        return (FALSE);
    bak += 9;
    for (idx = 0; idx < 15; idx++) {
        if (bak[idx] == CR_AC('\r') ||
            bak[idx] == CR_AC('\n')) {
            ip[idx] = 0x00;
            break;
        }
        ip[idx] = bak[idx];
    }
    if (idx >= 15)
        ip[15] = 0x00;
    return (TRUE);
}
#endif

/*
---------------------------------------
    环形队列长度
---------------------------------------
*/
static uint_t
simcom_rx_size (
  __CR_IN__ const sSIMCOM*  netw
    )
{
    /* 保证永远不会满 */
    if (netw->tail >= netw->head)
        return (netw->tail - netw->head);
    return (SIMCOM_BUF_SIZE - netw->head + netw->tail);
}

/*
---------------------------------------
    判断连接是否断开
---------------------------------------
*/
static bool_t
simcom_is_close (
  __CR_IN__ const sSIMCOM*  netw
    )
{
    uint_t  idx;
    ansi_t* bak;
    ansi_t  ret[64];

    /* 发查询命令 */
    bak = at_iorw(ret, sizeof(ret), "AT+CIPCLOSE?\r", SIMCOM_AT_TOUT);
    if (bak == NULL)
        return (FALSE);
    bak = str_strA(bak, "+CIPCLOSE: ");
    if (bak == NULL)
        return (FALSE);
    bak += 11;
    if (str_lenA(bak) < 19)
        return (FALSE);
    idx = netw->linknum * 2;
    if (idx < 18 && bak[idx + 1] != CR_AC(','))
        return (FALSE);
    return ((bak[idx] == 0x30) ? TRUE : FALSE);
}

/*****************************************************************************/
/*                             SIM7100/SIM6320                               */
/*****************************************************************************/

/*
---------------------------------------
    创建客户端 TCP 套接字 (内部)
---------------------------------------
*/
static socket_t
simcom_client_tcp_open_int (
  __CR_IN__ const ansi_t*   addr,
  __CR_IN__ int16u          port,
  __CR_IN__ int32s          time,
  __CR_IN__ int16u          lport
    )
{
    sint_t  num;
    ansi_t* str;
    ansi_t* bak;
    byte_t* dat;
    ansi_t  ret[64];

    /* 查找是否有空闲连接 */
    num = simcom_socket_find();
    if (num < 0)
        return (NULL);

    /* 先关一遍 (这里的状态可能和模块不同步) */
    simcom_socket_close((socket_t)(&s_simcom[num]));

    /* 环形队列分配 */
    dat = (byte_t*)mem_malloc(SIMCOM_BUF_SIZE);
    if (dat == NULL)
        return (NULL);

    /* 打开连接 */
    if (lport == 0) {
        str = str_fmtA("AT+CIPOPEN=%u,\"TCP\",\"%s\",%u\r",
                        num, addr, port);
    }
    else {
        str = str_fmtA("AT+CIPOPEN=%u,\"TCP\",\"%s\",%u,%u\r",
                        num, addr, port, lport);
    }
    if (str == NULL)
        goto _failure;
    bak = at_iorw(ret, sizeof(ret), str, SIMCOM_AT_TOUT);
    mem_free(str);
    if (bak == NULL || str_strA(bak, "ERROR") != NULL)
        goto _failure;
    bak = str_strA(bak, "+CIPOPEN: ");
    if (bak == NULL)
    {
_retry: /* 等待返回 */
        bak = at_wait(ret, sizeof(ret), SIMCOM_TIMEOUT(time));
        if (bak == NULL || str_strA(bak, "ERROR") != NULL)
            goto _failure;
        bak = str_strA(bak, "+CIPOPEN: ");
        if (bak == NULL)
            goto _retry;
    }
    bak += 10;
    if (num != (uint_t)(*bak) - 0x30)
        goto _failure;
    if (bak[2] != CR_AC('0'))
        goto _failure;

    /* 连接成功 */
    s_simcom[num].head = 0;
    s_simcom[num].tail = 0;
    s_simcom[num].fifo_buffer = dat;
    s_simcom[num].recv_tout = 120000UL;
    s_simcom[num].send_tout = 120000UL;
    s_simcom[num].remote_port = port;
    s_simcom[num].remote_addr = str_dupA(addr);
    return ((socket_t)(&s_simcom[num]));

_failure:
    mem_free(dat);
    return (NULL);
}

/*
=======================================
    创建客户端 TCP 套接字
=======================================
*/
CR_API socket_t
simcom_client_tcp_open (
  __CR_IN__ const ansi_t*   addr,
  __CR_IN__ int16u          port,
  __CR_IN__ int32s          time
    )
{
    return (simcom_client_tcp_open_int(addr, port, time, 0));
}

/*
=======================================
    创建客户端 TCP 套接字 (绑本地地址)
=======================================
*/
CR_API socket_t
simcom_client_tcp_open2 (
  __CR_IN__ const ansi_t*   addr,
  __CR_IN__ int16u          port,
  __CR_IN__ int32s          time,
  __CR_IN__ const ansi_t*   laddr,
  __CR_IN__ int16u          lport
    )
{
    CR_NOUSE(laddr);
    return (simcom_client_tcp_open_int(addr, port, time, lport));
}

/*
---------------------------------------
    创建客户端 UDP 套接字 (内部)
---------------------------------------
*/
static socket_t
simcom_client_udp_open_int (
  __CR_IN__ const ansi_t*   addr,
  __CR_IN__ int16u          port,
  __CR_IN__ int16u          lport
    )
{
    sint_t  num;
    ansi_t* str;
    ansi_t* bak;
    byte_t* dat;
    ansi_t  ret[64];

    /* 查找是否有空闲连接 */
    num = simcom_socket_find();
    if (num < 0)
        return (NULL);

    /* 先关一遍 (这里的状态可能和模块不同步) */
    simcom_socket_close((socket_t)(&s_simcom[num]));

    /* 环形队列分配 */
    dat = (byte_t*)mem_malloc(SIMCOM_BUF_SIZE);
    if (dat == NULL)
        return (NULL);

    /* 打开连接 */
    if (lport == 0)
        lport = SIMCOM_UDP_PORT(num);
    str = str_fmtA("AT+CIPOPEN=%u,\"UDP\",,,%u\r", num, lport);
    if (str == NULL)
        goto _failure;
    bak = at_iorw(ret, sizeof(ret), str, SIMCOM_AT_TOUT);
    mem_free(str);
    if (bak == NULL || str_strA(bak, "ERROR") != NULL)
        goto _failure;
    bak = str_strA(bak, "+CIPOPEN: ");
    if (bak == NULL)
    {
_retry: /* 等待返回 */
        bak = at_wait(ret, sizeof(ret), SIMCOM_DEF_TOUT);
        if (bak == NULL || str_strA(bak, "ERROR") != NULL)
            goto _failure;
        bak = str_strA(bak, "+CIPOPEN: ");
        if (bak == NULL)
            goto _retry;
    }
    bak += 10;
    if (num != (uint_t)(*bak) - 0x30)
        goto _failure;
    if (bak[2] != CR_AC('0'))
        goto _failure;

    /* 连接成功 */
    s_simcom[num].head = 0;
    s_simcom[num].tail = 0;
    s_simcom[num].fifo_buffer = dat;
    s_simcom[num].recv_tout = 120000UL;
    s_simcom[num].send_tout = 120000UL;
    s_simcom[num].remote_port = port;
    s_simcom[num].remote_addr = str_dupA(addr);
    return ((socket_t)(&s_simcom[num]));

_failure:
    mem_free(dat);
    return (NULL);
}

/*
=======================================
    创建客户端 UDP 套接字
=======================================
*/
CR_API socket_t
simcom_client_udp_open (
  __CR_IN__ const ansi_t*   addr,
  __CR_IN__ int16u          port
    )
{
    return (simcom_client_udp_open_int(addr, port, 0));
}

/*
=======================================
    创建客户端 UDP 套接字 (绑本地地址)
=======================================
*/
CR_API socket_t
simcom_client_udp_open2 (
  __CR_IN__ const ansi_t*   addr,
  __CR_IN__ int16u          port,
  __CR_IN__ const ansi_t*   laddr,
  __CR_IN__ int16u          lport
    )
{
    CR_NOUSE(laddr);
    return (simcom_client_udp_open_int(addr, port, lport));
}

/*
=======================================
    关闭套接字连接
=======================================
*/
CR_API void_t
simcom_socket_close (
  __CR_IN__ socket_t    netw
    )
{
    ansi_t* bak;
    ansi_t  str[64];
    /* ---------- */
    sSIMCOM*    real;

    /* 先发送关闭命令 */
    real = (sSIMCOM*)netw;
    chr_cpyA(str, "AT+CIPCLOSE=", 12);
    str[12] = (ansi_t)(real->linknum + 0x30);
    str[13] = CR_AC('\r');
    str[14] = 0x00;
    bak = at_iorw(str, sizeof(str), str, SIMCOM_AT_TOUT);
    if (bak != NULL) {
        bak = str_strA(bak, "+CIPCLOSE: ");
        if (bak == NULL)
            at_wait(str, sizeof(str), SIMCOM_DEF_TOUT);
    }

    /* 需要时复位结构成员 */
    if (real->fifo_buffer == NULL)
        return;
    SAFE_FREE(real->remote_addr);
    real->recv_tout = 120000UL;
    real->send_tout = 120000UL;
    real->remote_port = 0;
    real->head = real->tail = 0;
    mem_free(real->fifo_buffer);
    real->fifo_buffer = NULL;
}

/*
=======================================
    在 TCP 套接字上发送数据
=======================================
*/
CR_API uint_t
simcom_socket_tcp_send (
  __CR_IN__ socket_t        netw,
  __CR_IN__ const void_t*   data,
  __CR_IN__ uint_t          size
    )
{
    uint_t  idx;
    ansi_t* str;
    ansi_t* bak;
    ansi_t  ret[64];
    /* ---------- */
    sSIMCOM*    real;

    /* 有效性判断 */
    real = (sSIMCOM*)netw;
    if (real->fifo_buffer == NULL || (sint_t)size < 0)
        return (CR_U_ERROR);

    /* 判断连接是否断开 */
    if (simcom_is_close(real))
        return (CR_U_ERROR);

    /* 分块发送 */
    if (size == 0)
        return (0);
    str = str_fmtA("AT+CIPSEND=%u,1024\r", real->linknum);
    if (str == NULL)
        return (CR_U_ERROR);
    idx = size / 1024;
    for (; idx != 0; idx--) {
        at_flush();
        at_send(str, 18);
        at_send(data, 1024);
        bak = at_wait(ret, sizeof(ret), SIMCOM_AT_TOUT);
        if (bak == NULL || str_strA(bak, "ERROR") != NULL)
            goto _failure;
        bak = str_strA(bak, "+CIPSEND: ");
        if (bak == NULL)
        {
_retry1:    /* 等待返回 */
            bak = at_wait(ret, sizeof(ret), SIMCOM_TIMEOUT(real->send_tout));
            if (bak == NULL || str_strA(bak, "ERROR") != NULL)
                goto _failure;
            bak = str_strA(bak, "+CIPSEND: ");
            if (bak == NULL)
                goto _retry1;
        }
        bak += 10;
        if (real->linknum != (uint_t)(*bak) - 0x30)
            goto _failure;
        bak = str_chrA(bak + 2, CR_AC(','));
        if (bak == NULL)
            goto _failure;
        if (str2intA(bak + 1, NULL) != 1024)
            goto _failure;
        data = (byte_t*)data + 1024;
    }
    mem_free(str);

    /* 发送剩余 */
    idx = size % 1024;
    if (idx != 0) {
        str = str_fmtA("AT+CIPSEND=%u,%u\r", real->linknum, idx);
        if (str == NULL)
            return (CR_U_ERROR);
        at_flush();
        at_send_str(str);
        at_send(data, idx);
        mem_free(str);
        bak = at_wait(ret, sizeof(ret), SIMCOM_AT_TOUT);
        if (bak == NULL || str_strA(bak, "ERROR") != NULL)
            return (CR_U_ERROR);
        bak = str_strA(bak, "+CIPSEND: ");
        if (bak == NULL)
        {
_retry2:    /* 等待返回 */
            bak = at_wait(ret, sizeof(ret), SIMCOM_TIMEOUT(real->send_tout));
            if (bak == NULL || str_strA(bak, "ERROR") != NULL)
                return (CR_U_ERROR);
            bak = str_strA(bak, "+CIPSEND: ");
            if (bak == NULL)
                goto _retry2;
        }
        bak += 10;
        if (real->linknum != (uint_t)(*bak) - 0x30)
            return (CR_U_ERROR);
        bak = str_chrA(bak + 2, CR_AC(','));
        if (bak == NULL)
            return (CR_U_ERROR);
        if (str2intA(bak + 1, NULL) != idx)
            return (CR_U_ERROR);
    }
    return (size);

_failure:
    mem_free(str);
    return (CR_U_ERROR);
}

/*
=======================================
    在 UDP 套接字上发送数据
=======================================
*/
CR_API uint_t
simcom_socket_udp_send (
  __CR_IN__ socket_t        netw,
  __CR_IN__ const ansi_t*   addr,
  __CR_IN__ int16u          port,
  __CR_IN__ const void_t*   data,
  __CR_IN__ uint_t          size
    )
{
    ansi_t* str;
    ansi_t* bak;
    ansi_t  ret[64];
    /* ---------- */
    sSIMCOM*    real;

    /* 有效性判断 */
    real = (sSIMCOM*)netw;
    if (real->fifo_buffer == NULL ||
        (sint_t)size < 0 || size > 1500)
        return (CR_U_ERROR);

    /* 一包发送 */
    if (size == 0)
        return (0);
    if (addr == NULL) {
        if (real->remote_addr == NULL)
            return (CR_U_ERROR);
    }
    else {
        TRY_FREE(real->remote_addr);
        real->remote_addr = str_dupA(addr);
        if (real->remote_addr == NULL)
            return (CR_U_ERROR);
        real->remote_port = port;
    }
    str = str_fmtA("AT+CIPSEND=%u,%u,\"%s\",%u\r", real->linknum,
                    size, real->remote_addr, real->remote_port);
    if (str == NULL)
        return (CR_U_ERROR);
    at_flush();
    at_send_str(str);
    at_send(data, size);
    mem_free(str);
    bak = at_wait(ret, sizeof(ret), SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "ERROR") != NULL)
        return (CR_U_ERROR);
    bak = str_strA(bak, "+CIPSEND: ");
    if (bak == NULL)
    {
_retry: /* 等待返回 */
        bak = at_wait(ret, sizeof(ret), SIMCOM_TIMEOUT(real->send_tout));
        if (bak == NULL || str_strA(bak, "ERROR") != NULL)
            return (CR_U_ERROR);
        bak = str_strA(bak, "+CIPSEND: ");
        if (bak == NULL)
            goto _retry;
    }
    bak += 10;
    if (real->linknum != (uint_t)(*bak) - 0x30)
        return (CR_U_ERROR);
    bak = str_chrA(bak + 2, CR_AC(','));
    if (bak == NULL)
        return (CR_U_ERROR);
    if (str2intA(bak + 1, NULL) != size)
        return (CR_U_ERROR);
    return (size);
}

/*
---------------------------------------
    从模块拉数据
---------------------------------------
*/
static void_t
simcom_pull_data (
  __CR_IO__ sSIMCOM*    netw
    )
{
    uint_t  idx;
    ansi_t* str;
    ansi_t* bak;
    ansi_t* tmp;
    uint_t  size;
    uint_t  count;
    uint_t  total;
    ansi_t  ret[64];

    /* 自己的队列已满 */
    total = simcom_rx_size(netw);
    if (total >= SIMCOM_BUF_SIZE - 1)
        return;

    /* 获取剩余数据长度 */
    chr_cpyA(ret, "AT+CIPRXGET=4,", 14);
    ret[14] = (ansi_t)(netw->linknum + 0x30);
    ret[15] = CR_AC('\r');
    ret[16] = 0x00;
    bak = at_iorw(ret, sizeof(ret), ret, SIMCOM_AT_TOUT);
    if (bak == NULL)
        return;
    bak = str_strA(bak, "+CIPRXGET: 4,");
    if (bak == NULL)
        return;
    bak += 13;
    if (netw->linknum != (uint_t)(*bak) - 0x30)
        return;
    count = str2intA(bak + 2, NULL);
    if (count == 0)
        return;

    /* 计算剩余空间与实际拉取的长度 */
    total = SIMCOM_BUF_SIZE - 1 - total;
    if (count > total)
        count = total;

    /* 分块拉取 */
    str = str_fmtA("AT+CIPRXGET=2,%u,1024\r", netw->linknum);
    if (str == NULL)
        return;
    tmp = (ansi_t*)mem_malloc(1024 + 64);
    if (tmp == NULL)
        goto _failure1;
    idx = count / 1024;
    for (; idx != 0; idx--) {
        bak = at_iorw(tmp, 1024 + 64, str, SIMCOM_AT_TOUT);
        if (bak == NULL)
            goto _failure2;
        bak = str_strA(bak, "+CIPRXGET: 2,");
        if (bak == NULL)
            goto _failure2;
        bak += 13;
        if (netw->linknum != (uint_t)(*bak) - 0x30)
            goto _failure2;
        if (str2intA(bak + 2, NULL) != 1024)
            goto _failure2;

        /* 将数据放入自己的队列 */
        bak = str_strA(bak + 2, "\r\n");
        if (bak == NULL)
            goto _failure2;
        for (bak += 2, size = 1024; size != 0; size--) {
            netw->fifo_buffer[netw->tail++] = *bak++;
            if (netw->tail >= SIMCOM_BUF_SIZE)
                netw->tail = 0;
        }
    }
    mem_free(str);

    /* 拉取剩余 */
    size = count % 1024;
    if (size != 0) {
        str = str_fmtA("AT+CIPRXGET=2,%u,%u\r", netw->linknum, size);
        if (str != NULL) {
            bak = at_iorw(tmp, 1024 + 64, str, SIMCOM_AT_TOUT);
            if (bak == NULL)
                goto _failure2;
            bak = str_strA(bak, "+CIPRXGET: 2,");
            if (bak == NULL)
                goto _failure2;
            bak += 13;
            if (netw->linknum != (uint_t)(*bak) - 0x30)
                goto _failure2;
            if (str2intA(bak + 2, NULL) != size)
                goto _failure2;

            /* 将数据放入自己的队列 */
            bak = str_strA(bak + 2, "\r\n");
            if (bak == NULL)
                goto _failure2;
            for (bak += 2; size != 0; size--) {
                netw->fifo_buffer[netw->tail++] = *bak++;
                if (netw->tail >= SIMCOM_BUF_SIZE)
                    netw->tail = 0;
            }
            mem_free(str);
        }
    }
    mem_free(tmp);
    return;

_failure2:
    mem_free(tmp);
_failure1:
    mem_free(str);
    return;
}

/*
=======================================
    获取接收缓冲的数据长度
=======================================
*/
CR_API bool_t
simcom_socket_input_size (
  __CR_IN__ socket_t    netw,
  __CR_OT__ uint_t*     size
    )
{
    sSIMCOM*    real;

    /* 有效性判断 */
    real = (sSIMCOM*)netw;
    if (real->fifo_buffer == NULL)
        return (FALSE);

    /* 拉数据并返回自己队列的长度 */
    simcom_pull_data(real);
    *size = simcom_rx_size(real);
    return (TRUE);
}

/*
=======================================
    获取接收缓冲的数据长度 (阻塞)
=======================================
*/
CR_API bool_t
simcom_socket_input_size2 (
  __CR_IN__ socket_t    netw,
  __CR_OT__ uint_t*     size
    )
{
    uint_t      leng;
    int32u      base;
    sSIMCOM*    real;

    /* 有效性判断 */
    real = (sSIMCOM*)netw;
    if (real->fifo_buffer == NULL)
        return (FALSE);

    /* 没有数据就一直阻塞到超时 */
    base = timer_get32();
    for (;;)
    {
        /* 返回缓冲区里的数据长度 */
        simcom_socket_input_size(netw, &leng);

        /* 判断超时 */
        if (leng == 0) {
            if (timer_delta32(base) > real->recv_tout)
                return (FALSE);
            SIMCOM_YIELD
            continue;
        }
        break;
    }
    *size = leng;
    return (TRUE);
}

/*
=======================================
    在 TCP 套接字上预读数据
=======================================
*/
CR_API uint_t
simcom_socket_tcp_peek (
  __CR_IN__ socket_t    netw,
  __CR_OT__ void_t*     data,
  __CR_IN__ uint_t      size
    )
{
    uint_t      temp;
    uint_t      leng;
    int32u      base;
    sSIMCOM*    real;

    /* 有效性判断 */
    real = (sSIMCOM*)netw;
    if (real->fifo_buffer == NULL ||
        (sint_t)size < 0 || size > SIMCOM_BUF_SIZE)
        return (CR_U_ERROR);

    /* 直到数据搬完或超时为止 */
    temp = 0;
    base = timer_get32();
    for (;;)
    {
        /* 返回缓冲区里的数据长度 */
        simcom_socket_input_size(netw, &leng);

        /* 长度有变化就更新计时器 */
        if (temp != leng) {
            temp = leng;
            base = timer_get32();
        }

        /* 数据足够就出去了 */
        if (leng >= size) {
            leng = size;
            break;
        }

        /* 判断连接是否断开 */
        if (simcom_is_close(real))
            return (CR_SOCKET_CLOSED);

        /* 判断超时 */
        if (timer_delta32(base) > real->recv_tout) {
            if (leng == 0)
                return (CR_SOCKET_TIMEOUT);
            break;
        }
        SIMCOM_YIELD
    }

    /* 复制需要的数据 (不够的情况下有多少复制多少) */
    temp = SIMCOM_BUF_SIZE - real->head;
    if (leng <= temp) {
        mem_cpy(data, &real->fifo_buffer[real->head], leng);
    }
    else {
        mem_cpy(data, &real->fifo_buffer[real->head], temp);
        data = (byte_t*)data + temp;
        mem_cpy(data, &real->fifo_buffer[0], leng - temp);
    }
    return (leng);
}

/*
=======================================
    在 TCP 套接字上接收数据
=======================================
*/
CR_API uint_t
simcom_socket_tcp_recv (
  __CR_IN__ socket_t    netw,
  __CR_OT__ void_t*     data,
  __CR_IN__ uint_t      size
    )
{
    uint_t      tots;
    uint_t      blks;
    uint_t      back;
    sSIMCOM*    real;

    /* 分块接收 */
    real = (sSIMCOM*)netw;
    blks = size / SIMCOM_BUF_SIZE;
    for (tots = 0; blks != 0; blks--)
    {
        /* 出错直接返回 */
        back = simcom_socket_tcp_peek(netw, data, SIMCOM_BUF_SIZE);
        if ((sint_t)back < 0)
            return (back);

        /* 扔掉一堆数据 */
        real->head += back;
        if (real->head >= SIMCOM_BUF_SIZE)
            real->head -= SIMCOM_BUF_SIZE;
        tots += back;

        /* 是否断流了 */
        if (back != SIMCOM_BUF_SIZE)
            return (tots);
        data = (byte_t*)data + back;
    }

    /* 接收剩余 */
    blks = size % SIMCOM_BUF_SIZE;
    if (blks != 0)
    {
        /* 出错直接返回 */
        back = simcom_socket_tcp_peek(netw, data, blks);
        if ((sint_t)back < 0)
            return (back);

        /* 扔掉一堆数据 */
        real->head += back;
        if (real->head >= SIMCOM_BUF_SIZE)
            real->head -= SIMCOM_BUF_SIZE;
        tots += back;
    }
    return (tots);
}

/*
=======================================
    在 UDP 套接字上接收数据
=======================================
*/
CR_API uint_t
simcom_socket_udp_recv (
  __CR_IN__ socket_t    netw,
  __CR_OT__ void_t*     data,
  __CR_IN__ uint_t      size
    )
{
    return (simcom_socket_tcp_recv(netw, data, size));
}

/*
=======================================
    在 UDP 套接字上预读数据
=======================================
*/
CR_API uint_t
simcom_socket_udp_peek (
  __CR_IN__ socket_t    netw,
  __CR_OT__ void_t*     data,
  __CR_IN__ uint_t      size
    )
{
    return (simcom_socket_tcp_peek(netw, data, size));
}

/*
=======================================
    设置套接字超时参数
=======================================
*/
CR_API void_t
simcom_socket_set_timeout (
  __CR_IN__ socket_t    netw,
  __CR_IN__ int32s      wr_time,
  __CR_IN__ int32s      rd_time
    )
{
    sSIMCOM*    real;

    /* 有效性判断 */
    real = (sSIMCOM*)netw;
    if (real->fifo_buffer == NULL)
        return;
    real->recv_tout = rd_time;
    real->send_tout = wr_time;
}

/*
=======================================
    设置 TCP 断线检测参数
=======================================
*/
CR_API bool_t
simcom_socket_tcp_set_alive (
  __CR_IN__ socket_t    netw,
  __CR_IN__ uint_t      idle,
  __CR_IN__ uint_t      interval,
  __CR_IN__ uint_t      count
    )
{
    ansi_t* str;
    ansi_t* bak;
    ansi_t  ret[64];

    /* 参数过滤 */
    if ((int32u)idle < 60000UL)
        idle = 1;
    else
    if ((int32u)idle > 7200000UL)
        idle = 120;
    else
        idle /= 60000;
    if (count < 1)
        count = 1;
    else
    if (count > 10)
        count = 10;

    /* 发送 KeepAlive 指令 */
    str = str_fmtA("AT+CTCPKA=1,%u,%u\r", idle, count);
    if (str == NULL)
        return (FALSE);
    bak = at_iorw(ret, sizeof(ret), str, SIMCOM_AT_TOUT);
    mem_free(str);
    if (bak == NULL || str_strA(bak, "ERROR") != NULL)
        return (FALSE);
    CR_NOUSE(netw);
    CR_NOUSE(interval);
    return (TRUE);
}

/*****************************************************************************/
/*                                 SIM7100                                   */
/*****************************************************************************/

/*
=======================================
    SIM7100 上线拨号
=======================================
*/
CR_API bool_t
sim7100_online (
  __CR_IN__ const ansi_t*   apn,
  __CR_IN__ int32u          timeout
    )
{
    ansi_t* str;
    ansi_t* bak;
    int32u  base;
    ansi_t  ret[64];

    /* 把网络先关了 */
    simcom_socket_free();

    /* 网络注册 */
    bak = at_iorw(ret, sizeof(ret), "AT+CGREG=1\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);
    base = timer_get32();
    for (;;)
    {
        /* 不停询问是否注册成功 */
        bak = at_iorw(ret, sizeof(ret), "AT+CGREG?\r", SIMCOM_AT_TOUT);
        if (bak == NULL)
            return (FALSE);
        bak = str_strA(bak, "+CGREG: 1,");
        if (bak != NULL) {
            bak += 10;
            if (*bak == 0x31 || *bak == 0x35)
                break;
        }
        if (timer_delta32(base) > timeout)
            return (FALSE);
        SIMCOM_YIELD
    }

    /* 设置 APN 参数 */
    if (apn != NULL) {
        str = str_fmtA("AT+CGDCONT=1,\"IP\",\"%s\",,0,0\r", apn);
        if (str == NULL)
            return (FALSE);
        bak = at_iorw(ret, sizeof(ret), str, SIMCOM_AT_TOUT);
        mem_free(str);
        if (bak == NULL || str_strA(bak, "OK") == NULL)
            return (FALSE);
    }

    /* 减少超时时间 */
    bak = at_iorw(ret, sizeof(ret), "AT+CIPTIMEOUT=4000,4000,4000\r",
                  SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);

    /* 非透传模式 */
    bak = at_iorw(ret, sizeof(ret), "AT+CIPMODE=0\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);

    /* 手动接收模式 */
    bak = at_iorw(ret, sizeof(ret), "AT+CIPRXGET=1\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);

    /* 打开网络 */
    bak = at_iorw(ret, sizeof(ret), "AT+NETOPEN\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);
    base = timer_get32();
    for (;;)
    {
        /* 不停询问是否打开成功 */
        bak = at_iorw(ret, sizeof(ret), "AT+NETOPEN?\r", SIMCOM_AT_TOUT);
        if (bak == NULL)
            return (FALSE);
        bak = str_strA(bak, "+NETOPEN: 1");
        if (bak != NULL)
            break;
        if (timer_delta32(base) > timeout)
            return (FALSE);
        SIMCOM_YIELD
    }

    /* 拨号成功 */
    return (TRUE);
}

/*****************************************************************************/
/*                                 SIM6320                                   */
/*****************************************************************************/

/*
=======================================
    SIM6320 上线拨号
=======================================
*/
CR_API bool_t
sim6320_online (
  __CR_IN__ const ansi_t*   user,
  __CR_IN__ const ansi_t*   password,
  __CR_IN__ int32u          timeout
    )
{
    ansi_t* str;
    ansi_t* bak;
    int32u  base;
    ansi_t  ret[64];

    /* 把网络先关了 */
    simcom_socket_free();

    /* 网络注册 */
    bak = at_iorw(ret, sizeof(ret), "AT+CGREG=1\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);
    base = timer_get32();
    for (;;)
    {
        /* 不停询问是否注册成功 */
        bak = at_iorw(ret, sizeof(ret), "AT+CGREG?\r", SIMCOM_AT_TOUT);
        if (bak == NULL)
            return (FALSE);
        bak = str_strA(bak, "+CGREG: 1,");
        if (bak != NULL) {
            bak += 10;
            if (*bak == 0x31 || *bak == 0x35)
                break;
        }
        if (timer_delta32(base) > timeout)
            return (FALSE);
        SIMCOM_YIELD
    }

    /* 设置用户密码参数 */
    if (user != NULL || password != NULL) {
        if (user == NULL)
            user = "";
        if (password == NULL)
            password = "";
        bak = at_iorw(ret, sizeof(ret), "AT+CSOCKAUTHNV=1\r", SIMCOM_AT_TOUT);
        if (bak == NULL || str_strA(bak, "OK") == NULL)
            return (FALSE);
        str = str_fmtA("AT+CSOCKAUTH=,,\"%s\",\"%s\"\r", user, password);
        if (str == NULL)
            return (FALSE);
        bak = at_iorw(ret, sizeof(ret), str, SIMCOM_AT_TOUT);
        mem_free(str);
        if (bak == NULL || str_strA(bak, "OK") == NULL)
            return (FALSE);
    }

    /* 减少超时时间 */
    bak = at_iorw(ret, sizeof(ret), "AT+CIPTIMEOUT=4000,4000,4000\r",
                  SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);

    /* 非透传模式 */
    bak = at_iorw(ret, sizeof(ret), "AT+CIPMODE=0\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);

    /* 手动接收模式 */
    bak = at_iorw(ret, sizeof(ret), "AT+CIPRXGET=1\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);

    /* 打开网络 */
    bak = at_iorw(ret, sizeof(ret), "AT+NETOPEN\r", SIMCOM_AT_TOUT);
    if (bak == NULL || str_strA(bak, "OK") == NULL)
        return (FALSE);
    base = timer_get32();
    for (;;)
    {
        /* 不停询问是否打开成功 */
        bak = at_iorw(ret, sizeof(ret), "AT+NETOPEN?\r", SIMCOM_AT_TOUT);
        if (bak == NULL)
            return (FALSE);
        bak = str_strA(bak, "+NETOPEN: 1");
        if (bak != NULL)
            break;
        if (timer_delta32(base) > timeout)
            return (FALSE);
        SIMCOM_YIELD
    }

    /* 拨号成功 */
    return (TRUE);
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
