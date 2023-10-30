source_files = $(shell find src/ -name *.c)
header_files = $(shell find src/ -name *.h)
# object_files = $(patsubst src/%.c, build/%.o, $(source_files))

ldflags=-lz -lm  
cflags=-Wall

build/imgfun.o : src/imgfun.c
	mkdir -p $(dir $@)
	gcc $(cflags) -c $< -o $@ -I./src/

.PHONY: build
build: build/imgfun.o 
	mkdir -p dist/ 
	gcc build/imgfun.o $(ldflags) $() -o ./dist/imgfun