all: psxt

psxt:
	$(CPP) -Isrc src\psxt.cpp asr.lib
	del *.tds

wclang:
	$(MAKE) -fmakefile-clang all

wclang64:
	$(MAKE) -fmakefile-clang64 all
