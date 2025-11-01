# Assembler
C-language assembler for a predefined assembly language. A university project. This program receives a list of '.as' files (through the command-line) and, should the code be valid, creates a '.ob' file with the code encoded to hex. In addition, should the code refer to other files, the program will create '.ent' and '.ext' files that list those references. 
If the code is invalid, the program will print the first error of each line.
