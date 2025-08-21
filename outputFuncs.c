#include "sudoku.h"

// Функция для получения длины числа в виде строки
int getIntLen(int num){
    char strNum[100];
    sprintf(strNum, "%d", num);
    int len = (int)strlen(strNum);
    return len;
}


//------------------------------------------------Отображение Таблички--------------------------------------------------
void printNlayer(int cellCnt, char specific){
    printf("     ");
    int lenOfNum = getIntLen(cellCnt);

    for (int i = 0; i < cellCnt; i++){

        printf("|");
        for (int j = 0; j < 2 + lenOfNum; j++){

            if (specific == '+'){

                printf("+");
            } else if (specific == '-'){

                printf("-");
            }
        }
    }
    printf("#\n");
}

void printNumRow(SuCell* TableRow, int cellCnt){

    int lenOfNum = getIntLen(cellCnt);
    int blockSize = (int)sqrt(cellCnt);
    for (int i = 0; i < cellCnt; i++){

        if(i % blockSize != 0){
            printf("+ ");
        }else{
            printf("I ");
        }


        int restSpaces = lenOfNum - getIntLen(TableRow[i].value);
        for (int j = 0; j < restSpaces; j++){

            printf(" ");
        }
        printf("%d ", TableRow[i].value);
    }
    printf("|\n");
}

void printSuTable(SuTable* Table){
    printf("     ");
    int cellCnt = Table->rowColSize;
    
    for(int i = 1; i <= cellCnt; i++){
        if(i == 0){
                printf("+");
            }else{
                printf("|");
            }
        
        if(Table->rowColSize >= 10){
            if(i < 10){
                printf("(0%d)", i);
            }else{
                printf("(%d)", i);
            }
        }else{
            printf("(%d)", i);
        }
    }
    printf("|\n");

    for (int i = 0; i < cellCnt; i++){

        if (i % (int)sqrt(cellCnt) == 0 && i != 0){

            printNlayer(cellCnt, '+');
        } else {
            printNlayer(cellCnt, '-');
        }
        if(i + 1 <= 9){
            printf(" (0%d)", i+1);
        }else{
            printf(" (%d)", i+1);
        }
        
        printNumRow(Table->Sudoku[i], cellCnt);
    }
    printNlayer(cellCnt, '-');
}
//---------------------------------------------------------------------------------------------------
