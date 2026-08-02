default:
	@just --list && just project 

PROJECT_NAME := "tyto.h"
DATE_CREATED := "June 12 2026"
DESCRIPTION := "A single header library for C."

project:
    @echo {{PROJECT_NAME}}
    @echo {{DATE_CREATED}}
    @echo {{DESCRIPTION}}

# git
alias g := git
git:
	git init

alias c := commit
commit MSG:
	git add . && git commit -m "{{MSG}}"

alias p := push
push MSG:
	git add . && git commit -m "{{MSG}}" && git push

# C flags
asan_flags := "-fsanitize=address,leak,undefined -ftrapv"
warn_flags := "-Wall -Wextra -Wshadow -Wconversion"
include_flags := "-Iincludes"
std_flags := "-std=c99"
linker_flags := "-lc"

outfile := "./builds/tyto.test"

# space seperated list.
source_files := "./tests/test_ty_thread_queue.c ./src/ty_thread_queue.c "

# For building projects.
# $(find {{source_files}} -name '*.c')

build:
	gcc \
		{{include_flags}} \
		{{std_flags}} \
		{{asan_flags}} \
		{{warn_flags}} \
		{{source_files}} \
		{{linker_flags}} \
		-o {{outfile}}

alias t := test
test:
	just build && {{outfile}} 

