#+-------------------------------------------------------------------------------------------------+
#| GNU Make script for the border router based on hssv-atsamr21-breakout with CDC-ECM.             |
#|                                                                                                 |
#| This is based on the Makefile from the gnrc_boder_router example.                               |
#| Modified by: Joksan Alvarado.                                                                   |
#+-------------------------------------------------------------------------------------------------+

# name of your application
APPLICATION = hssv_atsamr21_breakout_border_router

# If no BOARD is found in the environment, use this default:
BOARD ?= hssv-atsamr21-breakout

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Include the USB CDC-ECM network interface. Adjust the amount of network interfaces accordingly.
USEMODULE += usbus_cdc_ecm

#include the USB CDC-ACM serial interface.
USEMODULE += stdio_cdc_acm

#Put the AT86RF233 radio in basic mode to enable communication with software-based radios such as
#NRF52840.
CFLAGS += -DAT86RF2XX_BASIC_MODE

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for 6LoWPAN border router
USEMODULE += gnrc_sixlowpan_border_router_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Optionally include RPL as a routing protocol. When includede gnrc_uhcpc will
# configure the node as a RPL DODAG root when receiving a prefix.
#USEMODULE += gnrc_rpl

# Optionally include DNS support. This includes resolution of names at an
# upstream DNS server and the handling of RDNSS options in Router Advertisements
# to auto-configure that upstream DNS server.
#USEMODULE += sock_dns

# include UHCP client
USEMODULE += gnrc_uhcpc

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# As there is an 'Kconfig' we want to explicitly disable Kconfig by setting
# the variable to empty
SHOULD_RUN_KCONFIG ?=

include $(RIOTBASE)/Makefile.include

.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS make -C $(RIOTTOOLS)

# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk
