all: audio_editor
CC=gcc
SRC=src/*/*.c
BINARY=audio_editor
DEFINES=-DLOG_USE_COLOR
CFLAGS=-std=c99
WARNINGS=-Wpedantic -Wall -Wextra -Wduplicated-cond -Wduplicated-branches \
			-Wlogical-op -Wrestrict -Wnull-dereference -Wswitch -Wswitch-enum \
			-Wjump-misses-init -Wdouble-promotion -Wshadow -Wformat=2 \
			-Werror=implicit-fallthrough=0 -Werror
LINK=-lopenal -lsndfile

# Builder uses this to build the project
audio_editor: $(SRC)
	$(CC) $(SRC) -o $(BINARY) $(DEFINES) $(CFLAGS) $(WARNINGS) $(LINK)

# Builder will call this to remove the built binary
clean:
	rm -f audio_editor

# Builder will call this to install the application before running.
install:
	echo "Installing is not supported"

# Builder uses this target to run your application.
run:
	./audio_editor && echo $?
