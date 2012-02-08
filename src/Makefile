#
# Copyright (c) Maxwell Dayvson <dayvson@gmail.com>
# Copyright (c) Tiago de Pádua <tiagopadua@gmail.com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

include ../defines.mk

MARCH:=$(shell uname -m)

# Standard VARS, may be overwritten by external parameters
CFLAGS = -I../include -lavformat -lavcodec -lavutil -lswscale
FLAGS = -Wc,-DMAJOR=$(MAJOR) -Wc,-DMINOR=$(MINOR) -Wc,-DPATCH=$(PATCH) -I../include
LIBS  = -lstdc++

.PHONY : all
all: apxs

#.PHONY : install
install: apxs
	cp -f .libs/$(LIBNAME) $(APACHE_MODULES_DIR)
	echo "Don't forget to edit httpd.conf"

.PHONY : clean
clean:
	rm -Rf $(BINNAME) *.o *~ *.la *.lo *.slo *.so *.loT .libs

# Enter a section for each target
apxs: 
	apxs -c $(FLAGS) module_main.c thumbnailer.c -o $(BINNAME) $(LIBS) 

test:
	gcc $(CFLAGS) thumbnailer.c ../test/main.c -o $(NAME)