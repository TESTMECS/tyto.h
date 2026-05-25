# Generic Justfile	
default:
	@just --list

alias c := commit
commit MSG:
	git add . && git commit -m "{{MSG}}" && git push 

asan_flags := "-fsanitize=address,leak,undefined -ftrapv"
warn_flags := "-Wall -Wextra -Wshadow -Wconversion"
include_flags := "-Iinclude"
std_flags := "-std=c99"

outfile := "main.out"

source_files := "."

build:
	gcc \
		{{include_flags}} \
		{{std_flags}} \
		{{asan_flags}} \
		{{warn_flags}} \
		$(find {{source_files}} -name "*.c") \
		-o {{outfile}}

# Specific to Tyto.h
alias t := test
test filename:
	gcc -std=c99 ./tests/{{filename}}.c -o ./tests/{{filename}}.out && ./tests/{{filename}}.out
	
# Specific to Tyto.h
alias d := docs
docs:
	lua otus.lua headerlibs/arena.h >> docs/arena.html &&
	lua otus.lua headerlibs/dynarray.h >> docs/dynarray.html &&
	lua otus.lua headerlibs/dynstring.h >> docs/dynstring.html &&
	lua otus.lua headerlibs/logging.h >> docs/logging.html &&
	lua otus.lua headerlibs/stringview.h >> docs/stringview.html &&
