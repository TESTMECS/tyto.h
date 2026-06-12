default:
	@just --list && just project 

PROJECT_NAME := "tyto.h"
DATE_CREATED := "June 12 2026"
DESCRIPTION := "A single header library for C."

project:
    @echo {{PROJECT_NAME}}
    @echo {{DATE_CREATED}}
    @echo {{DESCRIPTION}}

alias g := git
git:
	git init

alias c := commit
commit MSG:
	git add . && git commit -m "{{MSG}}"

alias p := push
push MSG:
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

