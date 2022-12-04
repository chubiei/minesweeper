# defined MSYSTEM = MinGW, otherwise OSX
ifdef MSYSTEM
include Makefile.MSYS
else
include Makefile.OSX
endif
