# UBSan

This file describes what UBSan is and how to use it.

## Introduction

UndefinedBehaviorSanitizer (UBSan) is a fast undefined behavior detector.
UBSan modifies the program at compile-time to catch various kinds of
undefined behavior during program execution.

## Setup

To use UBSan, add the `-fsanitize=undefined` option to both the compiling and linking steps.
This will compile the program with the sanitization tools enabled.

## Usage

When running a program that is compiled with UBSan enabled,
whenever certain types of undefined behaviour occurs,
there is a message printed out into the terminal the program is running in.

## Limitations

### Not a static analysis tool

Since UBSan is not a static analyzer, it can only detect issues in the code that is being executed.
No UBSan output only means means that there have been no issues detected only
in the paths the program actually took, and unless the execution has somehow
passed through all the possible code path combinations with all the possible variable values,
one can never be certain from running UBSan that all the issues in the code have been detected.

## More information

- [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
- GCC's [Program Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)