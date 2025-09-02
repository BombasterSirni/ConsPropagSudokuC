gcc -c -O3 -march=native -g -Wall -Wextra ChangesStack.c
gcc -c -O3 -march=native -g -Wall -Wextra SudokuLogic.c
gcc -c -O3 -march=native -g -Wall -Wextra SudokuMain.c
gcc -c -O3 -march=native -g -Wall -Wextra outputFuncs.c


gcc -O3 -march=native -flto ChangesStack.o SudokuMain.o SudokuLogic.o outputFuncs.o -o sudoku