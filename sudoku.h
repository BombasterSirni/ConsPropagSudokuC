#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//--------------------СТРУКТУРЫ ДЛЯ СУДОКУ, СТЕКА ИЗМЕНЕНИЙ------------------------------------------------------
typedef struct SuCell{
    int value;
    unsigned long long domain;
}SuCell;

typedef struct SuTable{
    int rowColSize; //размерность судоку
    int blockSize; // размерность блока судоку
    int cntOfEmpty; //количество пустых клеток
    struct SuCell** Sudoku; //массив судоку
}SuTable;

typedef struct Change{
    int row, column; //сохр.координаты
    int oldValue; //сохр.значение
    unsigned long long oldDomain; //сохр.домен
    int cntOfEmpty; //сохр.кол-во пустых клеток
}Change;

typedef struct ChangeStack{
    Change* changes; //сам массив стека 
    int capacity; //емкость
    int top; //индекс вершины
}ChangeStack;

//--------------------------------------------------------------------------------------------------------------------



//Функции инициализации и освобождения памяти для судоку
void SuTableInit(SuTable* Table, int cntOfCells);
void SuTableFree(SuTable* Table);

//Функции для стека изменений
void ChangeStackInit(ChangeStack* stack, int capacity);
void ChangeStackPush(ChangeStack* stack, int row, int column, int oldValue, unsigned long long oldDomain, int oldCntOfEmpty);
int ChangesStackSize(ChangeStack* stack);
void ChangesStackBackTrack(ChangeStack* stack, SuTable* Table, int backtrackTop);
void ChangeStackFree(ChangeStack* stack);


//Логика судоку
int InitDomains(SuTable* Table);
int cntOfCand(unsigned long long domain);
int editDomain(int value, SuTable* Table, int row, int column, ChangeStack* stack);
int editAdjDomains(int value, SuTable* Table, int row, int column, ChangeStack* stack);
int propagateConstraints(SuTable* Table, ChangeStack* stack);
int MRV_Degree(SuTable* Table, int* row, int* column);
int SudokuSolver(SuTable* Table, ChangeStack* stack);

//Функции для вывода судоку в файл
void printNlayer(int cellCnt, char specific);
void printNumRow(SuCell* TableRow, int cellCnt);
void printSuTable(SuTable* Table);

int getIntLen(int num);

#endif