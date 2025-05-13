# dumlang

kind of a dumb language

## Wat?

`dum` is a silly language I made so that I could write a compiler for it.

## Why?

I wanted to learn how codegen works in practice. I haven't read any books on it yet, but my goal is to try and implement *something* and then improve it so that I can better understand the reasoning behind the best practices.
But my alterior motive is to have some fun figuring out how to make something I have no idea how to make.

## Blog Posts

I hope to make a few blog posts while writing this thing. Here's what I have so far:

- [Analogous Instructions in x86](https://stowell.dev/posts/2025-02-11-analogous-instructions-x86/)

Sometimes my commit messages have some retrospective and forward-thinking thoughts so feel free to check them out too.
They will hopefully make their way into future blog posts!

## Things I still want to do
- [ ] Control flow
    - [x] If statement
    - [x] While loop
    - [ ] Boolean expressions
    - [ ] For loop
- [ ] More integer data types (i32, u32, i8, u8, etc)
- [ ] Pointers
- [ ] Syscalls
- [ ] `_start` symbol (basically libcrt.so)
    - [ ] Pass args to main
- [ ] Pre-processor / multi-file support
- [ ] Structs
- [ ] Bootstrap time!!
