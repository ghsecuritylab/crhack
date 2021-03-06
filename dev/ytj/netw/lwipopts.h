/**
  ******************************************************************************
  * @file    lwipopts.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   lwIP Options Configuration.
  *          This file is based on lwip\opt.h
  *          and contains the lwIP configuration for the STM32F107 demonstration.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    0

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  1

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (13 * 1024)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           10
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        4
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        4
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 4
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        12

#if 0
/* Enable mem_malloc() and mem_free() protect. */
#define SYS_ARCH_DECL_PROTECT(lev)
#define SYS_ARCH_PROTECT(lev)   __disable_irq()
#define SYS_ARCH_UNPROTECT(lev) __enable_irq()
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT  1
#endif

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          10

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       1500


/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255
#define LWIP_TCP_KEEPALIVE      1

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
/* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */
#define TCP_MSS                 (1500 - 40)

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (2 * TCP_MSS)

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        ((6 * TCP_SND_BUF) / TCP_MSS)

/* TCP receive window. */
#define TCP_WND                 (2 * TCP_MSS)


/* ---------- ICMP options ---------- */
#define LWIP_ICMP               1


/* ---------- IGMP options ---------- */
#define LWIP_IGMP               1


/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               1


/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255


/* ---------- Statistics options ---------- */
#define LWIP_STATS              0
#define LWIP_PROVIDE_ERRNO      1


/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/*
The STM32F107 allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#define CHECKSUM_BY_HARDWARE
#if defined(CHECKSUM_BY_HARDWARE)
    #define CHECKSUM_GEN_IP     0
    #define CHECKSUM_GEN_UDP    0
    #define CHECKSUM_GEN_TCP    0
    #define CHECKSUM_GEN_ICMP   0
    #define CHECKSUM_CHECK_IP   0
    #define CHECKSUM_CHECK_UDP  0
    #define CHECKSUM_CHECK_TCP  0
    #define CHECKSUM_CHECK_ICMP 0
#else
    #define CHECKSUM_GEN_IP     1
    #define CHECKSUM_GEN_UDP    1
    #define CHECKSUM_GEN_TCP    1
    #define CHECKSUM_GEN_ICMP   1
    #define CHECKSUM_CHECK_IP   1
    #define CHECKSUM_CHECK_UDP  1
    #define CHECKSUM_CHECK_TCP  1
    #define CHECKSUM_CHECK_ICMP 1
#endif

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN            0

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET             0

/*
   ---------------------------------------
   ---------- Debugging options ----------
   ---------------------------------------
*/
#if 0 && defined(YTJ_DEBUG)
    #include <stdio.h>
    #include <stdlib.h>
    #define LWIP_DEBUG
    #define LWIP_PLATFORM_DIAG(x) \
                do { printf x; } while (0)
    /* --------------------------------- */
    #define ETHARP_DEBUG        LWIP_DBG_ON
    #define NETIF_DEBUG         LWIP_DBG_ON
    #define PBUF_DEBUG          LWIP_DBG_ON
    #define API_LIB_DEBUG       LWIP_DBG_ON
    #define API_MSG_DEBUG       LWIP_DBG_ON
    #define SOCKETS_DEBUG       LWIP_DBG_ON
    #define ICMP_DEBUG          LWIP_DBG_ON
    #define IGMP_DEBUG          LWIP_DBG_ON
    #define INET_DEBUG          LWIP_DBG_ON
    #define IP_DEBUG            LWIP_DBG_ON
    #define IP_REASS_DEBUG      LWIP_DBG_ON
    #define RAW_DEBUG           LWIP_DBG_ON
    #define MEM_DEBUG           LWIP_DBG_ON
    #define MEMP_DEBUG          LWIP_DBG_ON
    #define SYS_DEBUG           LWIP_DBG_ON
    #define TIMERS_DEBUG        LWIP_DBG_ON
    #define TCP_DEBUG           LWIP_DBG_ON
    #define TCP_INPUT_DEBUG     LWIP_DBG_ON
    #define TCP_FR_DEBUG        LWIP_DBG_ON
    #define TCP_RTO_DEBUG       LWIP_DBG_ON
    #define TCP_CWND_DEBUG      LWIP_DBG_ON
    #define TCP_WND_DEBUG       LWIP_DBG_ON
    #define TCP_OUTPUT_DEBUG    LWIP_DBG_ON
    #define TCP_RST_DEBUG       LWIP_DBG_ON
    #define TCP_QLEN_DEBUG      LWIP_DBG_ON
    #define UDP_DEBUG           LWIP_DBG_ON
    #define TCPIP_DEBUG         LWIP_DBG_ON
    #define SLIP_DEBUG          LWIP_DBG_ON
    #define DHCP_DEBUG          LWIP_DBG_ON
    #define AUTOIP_DEBUG        LWIP_DBG_ON
    #define DNS_DEBUG           LWIP_DBG_ON
    #define IP6_DEBUG           LWIP_DBG_ON
#endif

#endif /* __LWIPOPTS_H__ */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
