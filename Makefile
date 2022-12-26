proj:
	gcc API/*.c API/Core/*.c Helpers.c AssemblyTranslator.c AssmeblerMain.c -o build/AssemblerMain -Wall -g -ansi -pedantic -m32