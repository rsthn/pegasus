
CC = clang++
CCFLAGS = -Qunused-arguments -Wno-format-security -fcolor-diagnostics -fansi-escape-codes \
		  -std=c++17 -O1 -Wl,-s -static -pthread -static-libstdc++ -static-libgcc -Wno-switch -g

all: psxt

psxt: src/psxt.cpp
	$(CC) $(CCFLAGS) $< -o $@ -lasr

run: psxt
	./psxt
