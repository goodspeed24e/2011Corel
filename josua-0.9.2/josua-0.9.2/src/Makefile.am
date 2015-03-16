
bin_PROGRAMS = josua 
AM_CFLAGS = -Wall  @JOSUA_FLAGS@  @PTHREAD_CFLAGS@

josua_SOURCES =  josua.c    ualogic.c        rcfile.c \
callbackuac.c          callbackuas.c         ua_core.c \
transaction_mt.c       msg_req.c             msg_resp.c       \
port_net.c             udp_send.c            streams.c        \
tcp_tl.c               udp_tl.c

josua_LDFLAGS = -O  -Wall
josua_LDADD = -lfsmtl -losip  @JOSUA_LIB@ @PTHREAD_LIBS@ -L$(prefix)/lib

INCLUDES = -I$(top_srcdir) -I$(prefix)/include
EXTRA_DIST = josua.h  rcfile.h README