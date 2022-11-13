# Copyright (c) 2022 Eric Johnson
# 
# This software is licensed for use under the terms of the BSD license.
# Please refer to the full text of the license, in the LICENSE file.

CC=cc
CFLAGS=-g -O2 -pipe -Wno-error=format-truncation -Wall -Wextra -Werror -Wpointer-arith -Wuninitialized -Wsign-compare -Wformat-security -Wsizeof-pointer-memaccess -Wno-pointer-sign -Wno-unused-result -Wimplicit-fallthrough -Wmisleading-indentation -fno-strict-aliasing -D_FORTIFY_SOURCE=2 -ftrapv -fzero-call-used-regs=all -fno-builtin-memset -fstack-protector-strong -fPIE  

LD=cc
LDFLAGS=-L. -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -fstack-protector-strong -pie 

TPROXYDOBJS=log.o pidfile.o failsafe.o

default: tproxyd

tproxyd: tproxyd.o $(TPROXYDOBJS)
	$(LD) -o $@ tproxyd.o $(TPROXYDOBJS)

clean:
	rm -f *.o