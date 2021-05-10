CC=gcc
WARN=-std=c99 -Wpedantic -Wall -Wextra -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wswitch -Wswitch-enum -Wjump-misses-init -Wdouble-promotion -Wshadow -Wformat=2 -Werror=implicit-fallthrough=0 -Werror -fsanitize=address -fsanitize=undefined -fsanitize=leak
OPTIMIZE=-Ofast -march=native
DEFINE=-DLOG_USE_COLOR
INCLUDE=-I/include/
LINK=-lopenal -lsndfile
SRC=main.c log.c
BINARY=main

debug: $(SRC)
	$(CC) $(SRC) $(INCLUDE) $(DEFINE) $(LINK) $(WARN) -o $(BINARY)
release: $(SRC)
	$(CC) $(SRC) $(INCLUDE) $(DEFINE) $(LINK) $(OPTIMIZE) -o $(BINARY)
	strip $(BINARY)
