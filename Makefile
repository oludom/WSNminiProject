CONTIKI_PROJECT = broadcasting_node
all: $(CONTIKI_PROJECT)


MAKE_MAC = MAKE_MAC_TSCH

MAKE_NET = MAKE_NET_NULLNET


# contiki ng shell
MODULES += os/services/shell


WERROR = 0

UIP_CONF_IPV6 = 0

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
