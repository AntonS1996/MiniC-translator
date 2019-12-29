# MiniC Translator
Translates MiniC code into language of atoms

MiniC Translator takes input from ".minic" file which contains the code of a program written on MiniC language and outputs ".atom" file. The last one contains a list of commands written on the language of atoms. Every command in this output file can be used to make a file for specific assembler (actually for i8080 assembler). After that the assembler is able to translate this file to a machine code for this processor (specifically for the Intel 8080 processor). So you can get the compiled file.

This program is written on C++.
