#include "sudoku.h"

int main(){
    freopen("C:\\C2\\SUDOKU2\\input.txt","r", stdin);
    freopen("C:\\C2\\SUDOKU2\\output.txt","w", stdout);

    int sudokuSize = 0;
    scanf("%d\n", &sudokuSize);
        

    SuTable* Table = (SuTable*)malloc(sizeof(SuTable));
    //инициализация судоку таблицы и стека изменений
    SuTableInit(Table, sudokuSize);

    ChangeStack stack;
    ChangeStackInit(&stack, sudokuSize * sudokuSize * sudokuSize * 3);
    

    if (InitDomains(Table) == 0){
        printf("Решения не имеется, попробуйте ввести другое судоку -__-\n");
    } else {
        if (SudokuSolver(Table, &stack) == 1){
            printf("Решение еще как есть!!! Вот оно:\n");
            printSuTable(Table);
        } else {
            printf("Как жаль, но ваше судоку чуть-чуть не работает((\n");
        }
    }

    ChangeStackFree(&stack);
    SuTableFree(Table);

    fclose(stdin);
    fclose(stdout);

    return 0;
}
