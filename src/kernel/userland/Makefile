CC = x86_64-elf-gcc
AS = nasm
LD = x86_64-elf-ld

CFLAGS = -Wall -Wextra -std=gnu11 -ffreestanding -O2 -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -Ilibc
LDFLAGS = -m elf_x86_64 -nostdlib -static -no-pie -Ttext=0x40000000 --no-dynamic-linker -z text -z max-page-size=0x1000 -e _start

BIN_DIR = bin

LIBC_SOURCES = $(wildcard libc/*.c)
LIBC_OBJS = $(patsubst libc/%.c, $(BIN_DIR)/%.o, $(LIBC_SOURCES)) $(BIN_DIR)/crt0.o

APP_SOURCES = $(filter-out nanojpeg.c, $(wildcard *.c))
APP_ELFS = $(patsubst %.c, $(BIN_DIR)/%.elf, $(APP_SOURCES))

all: $(BIN_DIR) $(APP_ELFS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/crt0.o: crt0.asm
	$(AS) -f elf64 $< -o $@

$(BIN_DIR)/%.o: libc/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/nanojpeg.o: nanojpeg.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/viewer.elf: $(LIBC_OBJS) $(BIN_DIR)/viewer.o $(BIN_DIR)/nanojpeg.o
	$(LD) $(LDFLAGS) $^ -o $@

$(BIN_DIR)/%.elf: $(LIBC_OBJS) $(BIN_DIR)/%.o
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)
