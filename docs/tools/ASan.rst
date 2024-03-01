ASan
====

This file describes what ASan is and how to use it.

Introduction
------------

AddressSanitizer (ASan) is a fast memory error detector.

Setup
-----

To use ASan, add the ``-fsanitize=address`` option to both the compiling and linking steps.
This will compile the program with AddressSanitizer enabled.

Usage
-----

When running a program that is compiled with ASan enabled,
whenever there are invalid memory accesses, such as out-of-bounds reads and writes,
invalid frees, double frees, uses after free, and so on,
there is a message printed out into the terminal the program is running in.

Additionally, by default on Linux, it also enables `LeakSanitizer <https://clang.llvm.org/docs/LeakSanitizer.html>`_,
which checks for memory leaks at the end of the execution of the program,
and prints any found leaks into the terminal.

Limitations
-----------

Not a static analysis tool
^^^^^^^^^^^^^^^^^^^^^^^^^^

Since ASan is not a static analyzer, it can only detect issues in the code that is being executed.
That means any code paths that were not taken during the program execution are unchecked and may still contain problems.

More information
----------------

- `AddressSanitizer <https://clang.llvm.org/docs/AddressSanitizer.html>`_
- `Google's AddressSanitizer wiki <https://github.com/google/sanitizers/wiki/AddressSanitizer>`_
- GCC's `Program Instrumentation Options <https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html>`_