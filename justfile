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

outfile := "tyto.test"

source_files := "."

build:
	gcc \
		{{include_flags}} \
		{{std_flags}} \
		{{asan_flags}} \
		{{warn_flags}} \
		tyto.h test.c \
		-o {{outfile}}

alias t := test
test:
	just build && ./tyto.test 

