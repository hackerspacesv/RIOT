/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "shell.h"
#include "msg.h"
#include "xtimer.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/netif/conf.h"
#include "net/gnrc/netapi.h"

#ifndef UHCPC_ETHOS_IF_ADDRESS
#error "Please set UHCPC_ETHOS_IF_ADDRESS in the Makefile before compiling this test."
#endif

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

int main(void)
{
    ipv6_addr_t ipv6_addrs[GNRC_NETIF_IPV6_ADDRS_NUMOF];
    ipv6_addr_t addr;
    int res;

    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("RIOT border router example application");

    /* wait for the ipv6 prefix to be configured by uhcpc */
    xtimer_usleep(2000000);

    /* remove the current auto-configured address to make space for the global one */
    ipv6_addr_from_str(&addr, "fe80::2");
    res = gnrc_netapi_get(7, NETOPT_IPV6_ADDR, 0, ipv6_addrs, sizeof(ipv6_addrs));
    for (unsigned i = 0; i < (res / sizeof(ipv6_addr_t)); i++) {
        /* only remove the address that is not used by uhcpc */
        if (!ipv6_addr_equal(&ipv6_addrs[i], &addr)) {
          gnrc_netapi_set(7, NETOPT_IPV6_ADDR_REMOVE, 0, &ipv6_addrs[i], sizeof(ipv6_addrs[i]));
          break;
        }
    }

    /* set the global address to the ethos network segment so it becomes routable */
    ipv6_addr_from_str(&addr, UHCPC_ETHOS_IF_ADDRESS);
    gnrc_netapi_set(7, NETOPT_IPV6_ADDR, 64 << 8, &addr, sizeof(addr));

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
