CONTIKI_PROJECT = master slave constant_jammer searching_jammer sweeping_jammer sequence_csma reset_sequence_csma
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES=shared.c jshared.c

PLATFORMS_EXCLUDE = nrf52dk

#MAKE_MAC = MAKE_MAC_TSCH
MAKE_MAC = MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

WERROR = 0

UIP_CONF_IPV6=0

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
