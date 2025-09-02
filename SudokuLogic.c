#include "sudoku.h"

//-------------------------------------ИНИЦИАЛИЗАЦИЯ ДОСКИ СУДОКУ--------------------------------------------------------
void SuTableInit(SuTable* Table, int cntOfCells){
    Table->rowColSize = cntOfCells;
    Table->blockSize = (int)sqrt(cntOfCells);
    Table->cntOfEmpty = 0;

    Table->Sudoku = (SuCell**)malloc(sizeof(SuCell*) * cntOfCells);
    for(int i = 0; i < cntOfCells; i++){

        Table->Sudoku[i] = (SuCell*)calloc(cntOfCells, sizeof(SuCell));
        
        for(int j = 0; j < cntOfCells; j++){
            int placeVal = 0;
            scanf("%d ", &placeVal);
            
            //устанавливаем начальное значение
            Table->Sudoku[i][j].value = placeVal;

            //устанавливаем базовые домены: домен только из 1 - это для пустых, домен с одной 1 - это для уже заполненных
            if(placeVal == 0){
                Table->Sudoku[i][j].domain = (1ULL << cntOfCells) - 1;
                Table->cntOfEmpty++;
            }else{
                Table->Sudoku[i][j].domain = 1ULL << (placeVal - 1);
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------ФУНКЦИЯ ПОДСЧЁТА КАНДИДАТОВ--------------------------------------------------------------
int cntOfCand(unsigned long long domain){ 
    return __builtin_popcountll(domain); //builtin-функция (встроенная) из GCC для подсчета единичных битов в машинных словах
    //заменил цикл O(N) на вот эту функцию с O(1) временем выполнения
}
//-----------------------------------------------------------------------------------------------------------------


//---------------------------------ФУНКЦИЯ УСТАНОВКИ НАЧАЛЬНЫХ ОГРАНИЧЕНИЙ ----------------------------------------------------
int InitDomains(SuTable* Table){

    int rowColSize = Table->rowColSize;
    int blockSize = Table->blockSize;   

    //проходим по каждой клетке таблицы
    for(int row = 0; row < rowColSize; row++){
        for(int col = 0; col < rowColSize; col++){
            //если она пустая, то будем ставить ей ограничения в зависимости от соседей (в столбце, строке и блоке)
            if(Table->Sudoku[row][col].value == 0){
               
                //установка ограничений со строки
                for(int c = 0; c < rowColSize; c++){
                    if(Table->Sudoku[row][c].value != 0){
                        int fullCellV = Table->Sudoku[row][c].value;
                        Table->Sudoku[row][col].domain &= ~(1ULL << (fullCellV - 1));
                    }
                }

                //установка ограничений со столбца
                for(int r = 0; r < rowColSize; r++){
                    if(Table->Sudoku[r][col].value != 0){
                        int fullCellV = Table->Sudoku[r][col].value;
                        Table->Sudoku[row][col].domain &= ~(1ULL << (fullCellV - 1));
                    }
                }

                //установка ограничений с блока
                int blkRow_St = (row / blockSize) * blockSize;
                int blkCol_St = (col / blockSize) * blockSize;
                for (int curr_row = blkRow_St; curr_row < blkRow_St + blockSize; curr_row++){

                    for (int curr_col = blkCol_St; curr_col < blkCol_St + blockSize; curr_col++){

                        if(Table->Sudoku[curr_row][curr_col].value != 0){
                            int fullCellV = Table->Sudoku[curr_row][curr_col].value;
                            Table->Sudoku[row][col].domain &= ~(1ULL << (fullCellV - 1));
                        }
                    }
                }

                //Проверка на случай, если мы при начальных значениях обнулили совсем домен клетки
                //ну то есть судоку нерешаем, если хоть у одной клетки пустой нет кандидата на подстановку
                if(Table->Sudoku[row][col].domain == 0){
                    return 0;
                }

            }
        }
    }

    return 1;
}
//-------------------------------------------------------------------------------------------------------

//----------------------------------------ФУНКЦИЯ ИЗМЕНЕНИЯ ДОМЕНА ДЛЯ КЛЕТКИ----------------------------------------------------
int editDomain(int value, SuTable* Table, int row, int column, ChangeStack* stack){

    //ну домен занятой клетки мы не меняем
    if (Table->Sudoku[row][column].value != 0){
        return 1;
    }

    // isEmptyCand будет равен 0, если мы пытаемся удалить кандидата, который уже был удален из данного домена
    unsigned long long isEmptyCand = Table->Sudoku[row][column].domain & (1ULL << (value - 1));
    if (isEmptyCand == 0){
        return 1;
    }

    //сохраняем изменение домена в стек изменений
    ChangeStackPush(stack, row, column, Table->Sudoku[row][column].value, Table->Sudoku[row][column].domain, Table->cntOfEmpty);

    // создаем маску на удаление конкретного кандидата из домена (бита из машинного слова/64-битового двоич.числа)
    unsigned long long candRemoveMask = ~(1ULL << (value - 1));
    Table->Sudoku[row][column].domain &= candRemoveMask;

    //если домен занулился, то так не должно быть - это конфликт
    if (Table->Sudoku[row][column].domain == 0){
        return 0;
    }
    return 1;
}
//----------------------------------------------------------------------------------------------------------------------------------------


//---------------------------ФУНКЦИЯ ЛОКАЛЬНОГО ФОРСИРОВАНИЯ (ИЗМЕНЕНИЯ) ДОМЕНОВ ДЛЯ СОСЕДНИХ КЛЕТОК--------------------------------------
// данную функцию вызываем сразу после подстановки в пустую клетку [row][column] значения для редактирования доменов всех соседних клеток
int editAdjDomains(int value, SuTable* Table, int row, int column, ChangeStack* stack){
    int rowColSize = Table->rowColSize;
    int blockSize = Table->blockSize;

    //фиксируем индекс текущей вершины в стеке изменений - так называемый checkpoint
    int backtrackTop = ChangesStackSize(stack);

    // Столбец и строка, пропускаем текущую клетку, так как ф-ия editAdjDomains вызывается после подстановки в текущ.клетку значения
    // а мы в editDomain рассматриваем только пустые клетки; а здесь мы изменяем домены соседних клеток в столбце/строке и если конфликт
    // то мы откатываемся до checkpoint
    for(int i = 0; i < rowColSize; i++){
        if(i != row && editDomain(value, Table, i, column, stack) == 0){
            ChangesStackBackTrack(stack, Table, backtrackTop);
            return 0;
        }
        if(i !=  column && editDomain(value, Table, row, i, stack) == 0){
            ChangesStackBackTrack(stack, Table, backtrackTop);
            return 0;
        }
    }
        
    // Блок; для блока принцип аналогичен
    int blkRow_St = (row / blockSize) * blockSize;
    int blkCol_St = (column / blockSize) * blockSize;
    for (int curr_row = blkRow_St; curr_row < blkRow_St + blockSize; curr_row++){
        for (int curr_col = blkCol_St; curr_col < blkCol_St + blockSize; curr_col++){
            if(editDomain(value, Table, curr_row, curr_col, stack) == 0){
                ChangesStackBackTrack(stack, Table, backtrackTop);
                return 0;
            }
        }
    }

    return 1;
}
//----------------------------------------------------------------------------------------------------------------------------


//--------------------------------ФУНКЦИЯ ГЛОБАЛЬНОГО ФОРСИРОВАНИЯ (ИЗМЕНЕНИЯ) ДОМЕНОВ ПО ВСЕМ КЛЕТКАМ--------------------------------------------------
// данную функцию вызываем, чтобы пройтись по всем клеткам таблицы (даже несколько раз), чтобы проверить, не обнулился ли домен какой-то пустой клетки
// потом подставить в нее (если у нее 1 кандидат остался) значение, и отредактировать домены всех ее соседей (пустых) + проверить на конфликт (внутри editAdjDomains)
// ну а прекратится это, когда изменять уже будет нечего (когда к обходу всей таблицы флаг changed останется равен 0)
int propagateConstraints(SuTable* Table, ChangeStack* stack){
    int rowColSize = Table->rowColSize;
    int changed = 1;

    while(changed == 1){ 
        changed = 0;
        for(int row = 0; row < rowColSize; row++){
            for(int col = 0; col < rowColSize; col++){
                if(Table->Sudoku[row][col].value == 0){

                    if(cntOfCand(Table->Sudoku[row][col].domain) == 0){
                        return 0;
                    }
                
                    if(cntOfCand(Table->Sudoku[row][col].domain) == 1){
                        int value = __builtin_ctzll(Table->Sudoku[row][col].domain) + 1; //builtin-функция (встроенная), которая подсчитывает количество ведущих нулей
                        //это количество ведущих нулей + 1 (номер первой единицы в машинном слове + 1) - это и есть значение кандидата в данную клетку 
                        unsigned long long domain = Table->Sudoku[row][col].domain;

                        //кладем текущ.состояние в стек до подстановки
                        ChangeStackPush(stack, row, col, 0, domain, Table->cntOfEmpty);
                        //подставляем
                        Table->Sudoku[row][col].value = value;
                        Table->Sudoku[row][col].domain = 1ULL << (value - 1);
                        Table->cntOfEmpty--;

                        //редактируем домены и там же внутри (если конфликт) - откатываемся
                        if(editAdjDomains(value, Table, row, col, stack) == 0){
                            return 0;
                        }
                        changed = 1;
                    }

                }
            }
        }
    }
    return 1;
}
//-------------------------------------------------------------------------------------------------------------------------------


//------------------ФУНКЦИЯ ПОИСКА КЛЕТКИ С НАИМЕНЬШИМ КОЛИЧЕСТВОМ КАНДИДАТОВ---------------------------
int MRV_Degree(SuTable* Table, int* row, int* column){

    int blockSize = Table->blockSize;
    int rowColSize = Table->rowColSize;
    int bestDegree = -1;
    int candMinCnt = rowColSize + 1;

    *row = -1;
    *column = -1;

    // у каждой пустой клетки считаем так называемое DegreeCnt - количество хороших пустых клеток соседей
    // в случае ограничения которых мы получим множество клеток с малым количество кандидатов. В моем случае, хорошие <=> кол-во кандидатов <= 3
    // вместо того, чтобы за O(N^3) пройти по всем  <=N^2 пустым клетками и за <=N-итераций подсчитывать DegreeCnt 
    // можно заранее ввести 3 массива (из которых в сумме по номеру столбцы, строки и блока у конкретной клетки) для хранения количества "хороших" клеток
    // и спустя данный цикл за O(N^2) мы уже пройдемся еще раз аз O(N^2) по всем пустым клеткам и для каждой из них посчитаем DegreeCnt

    int* smallCandCntRow = (int*)calloc(rowColSize, sizeof(int));
    int* smallCandCntCol = (int*)calloc(rowColSize, sizeof(int));
    int* smallCandCntBlock = (int*)calloc(rowColSize, sizeof(int));

    //первая проходка по всем пустым клеткам
    for(int curr_row = 0; curr_row < rowColSize; curr_row++){
        for(int curr_col = 0; curr_col < rowColSize; curr_col++){
            if(Table->Sudoku[curr_row][curr_col].value == 0){

                int candCnt = cntOfCand(Table->Sudoku[curr_row][curr_col].domain);
                if(candCnt <= 3){
                    smallCandCntCol[curr_col]++;
                    smallCandCntRow[curr_row]++;
                    //надо найти индекс блока, найдем по такой формуле:
                    int blockId = (curr_row / blockSize) * blockSize + (curr_col / blockSize);
                    smallCandCntBlock[blockId]++;

                    //таким образом считаем среди всех клеток такие вот хорошие (с малым количество кандидатов)
                }

            }
        }
    }

    // а теперь пользуясь информацией для каждой пустой клетки из трех массивов, посчитаем для каждой клетки DegreeCnt
    for (int curr_row = 0; curr_row < rowColSize; curr_row++){
        for (int curr_col = 0; curr_col < rowColSize; curr_col++){
            if (Table->Sudoku[curr_row][curr_col].value == 0){
                int candCnt = cntOfCand(Table->Sudoku[curr_row][curr_col].domain);

                if (candCnt == 0){
                    free(smallCandCntBlock);
                    free(smallCandCntCol);
                    free(smallCandCntRow);
                    return 0;
                }
        
                
                int blockId = (curr_row / blockSize) * blockSize + (curr_col / blockSize);
                //число хороших соседних клеток DegreeCnt считается вот так:
                int DegreeCnt = smallCandCntRow[curr_row] + smallCandCntCol[curr_col] + smallCandCntBlock[blockId];
                //но если текущая клетка была среди тех "хороших" - то мы отнимаем от нее 2, так как она была учтена трижды
                if(candCnt <= 3){
                    DegreeCnt = DegreeCnt - 2;
                }

                // бОльший Приоритет в данной функции - это Эвристика Degree (клетка с наибольшим количество пустых клеток с малым кол-вом доменов)
                // меньший приоритет - это Эвристика Minimal Remaining Value (клетка с наименьшим количеством кандидатов)
                // в тяжелых судоку, особенно в 25x25 и 36x36 немного клеток с очевидно наименьшим кол-вом кандидатов
                // поэтому было принято решение (за 2 дня до сдачи XD) добавить на случай тяжелых судоку вторую Стратегию (Эвристику)
                if ((candCnt < candMinCnt && DegreeCnt == bestDegree) || (bestDegree < DegreeCnt)){
                    bestDegree = DegreeCnt;
                    candMinCnt = candCnt;
                    *row = curr_row;
                    *column = curr_col;
                }
                if (candMinCnt == 1){
                    free(smallCandCntBlock);
                    free(smallCandCntCol);
                    free(smallCandCntRow);
                    return 1;
                }
            }
        }
    }

    free(smallCandCntBlock);
    free(smallCandCntCol);
    free(smallCandCntRow);
    return candMinCnt <= rowColSize;
}
//------------------------------------------------------------------------------------------------------------





//------------------------------------Рекурсивный решатель Судоку----------------------------------------------
// функция запуска рекурсии грубо говоря
int SudokuSolver(SuTable* Table, ChangeStack* stack){

    //стандартный случай нахождения однозначного решения
    if (Table->cntOfEmpty == 0){
        return 1;
    }

    //здесь в этих переменных будут лежать индексы пустой клетки, подобранной с помощью функции MRV_Degree
    int curr_row, curr_column;
    if (MRV_Degree(Table, &curr_row, &curr_column) == 0){
        return 0;
    }
    
    // достаем домен этой клетки
    unsigned long long curr_domain = Table->Sudoku[curr_row][curr_column].domain;
    //и пытаемся подставить в нее каждого из её кандидатов (они отбираются за O(1) с помощью встроенной в gcc функции и из нее удаляются просто)
    // ну продолжаем, пока домен этой клетки не обнулим (это не конфликтная ситуация, мы работаем с копией)
    while(curr_domain){
        int value = __builtin_ctzll(curr_domain) + 1;
        unsigned long long candMask = 1ULL << (value - 1);
        curr_domain &= ~candMask;

        //достаем текущ вершину стека изменений
        int backtrackTop = ChangesStackSize(stack);
        //сохраняем текущее положение клетки и ее домен с кол-вом пустых клеток в стек (для будущих возможно откатов)
        ChangeStackPush(stack, curr_row, curr_column, 0, Table->Sudoku[curr_row][curr_column].domain, Table->cntOfEmpty);
        //подставляем
        Table->Sudoku[curr_row][curr_column].domain = candMask;
        Table->Sudoku[curr_row][curr_column].value = value;
        Table->cntOfEmpty--;

        //а здесь сначала редактируем домены всех соседних клеток (и ищем конфликты), а потом глобально проходимся еще по таблице и заполняем ее, пока можем
        //при этом заходя в рекурсию, ну и если SudokuSolver где-то вернет 1 (то есть cntOfEmpty = 0), то это верное решение
        //то есть от самого глубокого в рекурсии вызова SudokuSolver поднимаемся наверх
        if (editAdjDomains(value, Table, curr_row, curr_column, stack) == 1 && propagateConstraints(Table, stack) == 1){
            if (SudokuSolver(Table, stack) == 1){
                return 1;
            }
        }

        //ну а если все мы вылетели из editAdjDomains или из propagateConstraints, то есть где-то вернулся 0 (конфликт), то откатываемся
        ChangesStackBackTrack(stack, Table, backtrackTop);

    }

    //ну крайний случай совсем 
    return 0;
}

//----------------------------------------------------------------------------------------------------------------

//--------------------------------Освобождение памяти для таблицы судоку-----------------------------------------
void SuTableFree(SuTable* Table){
    for (int i = 0; i < Table->rowColSize; i++){
        free(Table->Sudoku[i]);
    }
    free(Table->Sudoku);
    free(Table);
}
//----------------------------------------------------------------------------------------------------------------