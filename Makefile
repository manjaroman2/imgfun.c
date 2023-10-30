source_files = $(shell find src/ -name *.c)
object_files = $(patsubst src/%.c, build/%.o, $(source_files))

flags = -Wall
libs= -lz -lm 

$(object_files): build/%.o : src/%.c
	mkdir -p $(dir $@)
	gcc $(flags) -c -I./external/spng -I./include/ $(patsubst build/%.o, src/%.c, $@) -o $@

.PHONY: build
build: $(object_files) spng.a
	mkdir -p dist/
	gcc $(flags) -o./dist/img-fun $(object_files) build/spng.a 

spng.a: spng.o
	ar rcs build/$@ build/$^ 

spng.o: external/spng/spng.c 
	gcc -c -o build/$@ $< -I./external/spng -lz -lm 

clean:
	rm -rf build/ dist/ 
