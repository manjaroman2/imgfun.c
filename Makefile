source_files = $(shell find src/ -name *.c)
object_files = $(patsubst src/%.c, build/%.o, $(source_files))

flags = -Wall

$(object_files): build/%.o : src/%.c 
	mkdir -p $(dir $@)
	gcc $(flags) -c -I src/include $(patsubst build/%.o, src/%.c, $@) -o $@


.PHONY: build
build: $(object_files)
	mkdir -p dist/
	gcc $(flags) -o dist/img-fun $(object_files) -lz -lm
