Valgrind
========

This file describes what Valgrind is, how to set it up and how to use it.

Introduction
------------

`Valgrind <https://valgrind.org/>`_ is a collection of tools for debugging and profiling Linux programs.
It includes a bunch of tools, such as cache and heap profilers, multithreading debuggers, and memory debuggers.

The tool this file will focus on is called Memcheck, which is a tool to find memory management issues.

Setup
-----

The process to install Valgrind is the same as any other program.
The recommended way to install it is via the distribution's package manager,
looking for the package named ``valgrind``.

To use a specific version of Valgrind that is not provided by the maintainers of the distribution being used,
the recommended way would be to download `the source code <https://sourceware.org/pub/valgrind/>`_
of that specific version and compile it by using the instructions provided with the source code.

Usage
-----

To invoke ``valgrind`` for a specific program, use this command:

.. code-block:: bash

    valgrind <program> <program arguments>


Using Valgrind without any tool specified via the ``--tool`` option defaults to using Memcheck.

Suppressions
^^^^^^^^^^^^

Using Valgrind on bigger projects, like this one, and running GUI applications
can produce quite a lot of noise from the libraries not being clean of issues of their own.
To help manage this issue, Valgrind has a feature called suppressions
that helps by suppressing errors that are not interesting to us.

To generate the suppressions for all the errors that Valgrind's tools find
use the ``--gen-suppressions=all`` option in Valgrind
(Valgrind options are passed into it *before* the call to the program).
This will generate a suppression for each error the tool finds.
By saving the supressions into a file one can then use those suppressions in the future
by supplying the file it's saved in by using the ``--supressions=<filename>`` option.

More information about suppression
""""""""""""""""""""""""""""""""""

- `Suppressing errors <https://valgrind.org/docs/manual/manual-core.html#manual-core.suppress>`_

Limitations
-----------

Not a static analysis tool
^^^^^^^^^^^^^^^^^^^^^^^^^^

Since all the tools in Valgrind are not static analysers,
they can only detect issues in the code that is being executed.
Tools like Memcheck or Helgrind not outputting anything means that
there have been no issues detected only in the paths the program actually took,
and unless the execution has somehow passed through all the possible code path combinations,
one can never be certain from running these tools that all the issues in the code have been detected.

Unknown instructions
^^^^^^^^^^^^^^^^^^^^

Since Valgrind analyses what instructions are being run on the CPU,
it might sometimes come across x86 instructions that it does not know of yet.
In that case, the execution of the program is immediately halted,
with the hex dump of the instruction that caused the program to halt printed in the console.
Using a disassembler `(online version) <https://defuse.ca/online-x86-assembler.htm>`_ can help figure out
if it is a legitimate instruction or if the memory has been corrupted in such a way
that execution had jumped to a memory location that was not meant to be code or
that the memory the program has been executing at has been overwritten.

In the case of LimeSuite2, there are cases where Valgrind runs into unknown instructions.
The unknown instructions, at the point of this being written
(using Valgrind 3.22.0 and GCC 12.3.0),
seem to be coming from the AVX-512 instruction sets.
Disabling the compilation of that instruction set
(which can be done in GCC via the ``-mno-avx512f`` compiler flag)
forces the compiler to output potentially slower, but still valid code for the program
without containing the instructions that Valgrind does not recognise,
thus still allowing for analysis to be done.

More information
----------------

- `Valgrind manual <https://valgrind.org/docs/manual/manual.html>`_
