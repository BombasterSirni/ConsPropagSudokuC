#include "sudoku.h"


void ChangeStackInit(ChangeStack* stack, int capacity){ 
    //просто инициализация стека
    stack->changes = (Change*)malloc(sizeof(Change) * capacity);
    stack->capacity = capacity;
    stack->top = 0;
}

void ChangeStackPush(ChangeStack* stack, int row, int column, int oldValue, unsigned long long oldDomain, int oldCntOfEmpty){ 
    
    Change* change = &stack->changes[stack->top++]; //берем указатель на первую незанятую ячейку в стеке изменений
    change->row = row;
    change->column = column;
    change->oldValue = oldValue;
    change->oldDomain = oldDomain;
    change->cntOfEmpty = oldCntOfEmpty; // и заполняем ее
}


int ChangesStackSize(ChangeStack* stack){ // просто возвращаем индекс вершины стека
    return stack->top;
}


//функция, в которой мы из стека вытягиваем то положение, которое как-то сохранили под значением backtrackTop
void ChangesStackBackTrack(ChangeStack* stack, SuTable* Table, int backtrackTop){

    //последовательно идем с вершины, пока не дойдем до сохраненного состояния и мы его применим в конце цикла
    while (stack->top > backtrackTop){
        Change* change = &stack->changes[--stack->top];
        Table->Sudoku[change->row][change->column].value = change->oldValue;
        Table->Sudoku[change->row][change->column].domain = change->oldDomain;
        Table->cntOfEmpty = change->cntOfEmpty;
    }
}

// функция очистки стека
void ChangeStackFree(ChangeStack* stack){
    free(stack->changes);
}