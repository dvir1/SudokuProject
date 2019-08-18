#include <stdio.h>
#include <stdlib.h>
#include "Game.h"
#include <stdbool.h>

Puzzle puzzleStrct = {0, 0, 0, 0, 0, 0, 0};
Puzzle *puzzle = &puzzleStrct;

int getNumOfRowInBlock()
{
    return (puzzle->blockNumRow);
}

int getNumOfColInBlock()
{
    return (puzzle->blockNumCol);
}

int getBlockNumOfCells()
{
    return (puzzle->blockNumOfCells);
}

int getNumOfCells()
{
    return (puzzle->numOfCells);
}

/*
 * initialize puzzle fields - free allocated memory
 * pre-condition: puzzle->board!=NULL
 */
void cleanPuzzle()
{
    int i;
    for (i = 0; i < puzzle->blockNumOfCells; i++)
    {
        free(puzzle->board[i]);
    }
    /* ensure that memory was allocated for board before.
     * why we need it:
     * assuming loading a file failed (non-exist file), we don't need to free the board again
     * because we didn't try to allocate memory */
    if (puzzle->blockNumOfCells>0)
    	free(puzzle->board);
    puzzle->blockNumCol = 0;
    puzzle->blockNumRow = 0;
    puzzle->blockNumOfCells = 0;
    puzzle->numOfCells = 0;
    puzzle->numOfEmptyCells = 0;
}

bool fillBoard(FILE* fp, Mode mode) {
	int i, j, N=puzzle->blockNumOfCells, newValue;
	char ch;
	Cell *cell;

	for(i=0;i<N;i++){
		for(j=0;j<N;j++){
			cell = getCell(j+1, i+1);
			if (fscanf(fp, "%d", &newValue)!=1)
				return false;
			if (!isNumInRange(newValue, 0, N))
				return false;
			/*printf("%d\n", newValue);*/
			if (newValue!=0) {
				puzzle->numOfEmptyCells--;
				updateCollisions(j+1, i+1, newValue);
				cell->value = newValue;
				if(fscanf(fp, "%c", &ch)==1){
					if (ch=='.' && mode==Solve) /* cell is not fixed when loading in edit mode */
						cell->fixed=1;
				}
			}
		}
	}
	/*
	solve C:\\sudoku\\in8.txt should fail
	if (hasErroneousFixedCells() && mode==Solve)
		return false;
	*/
	return true;
}

/*
 * return true if assignment to board succeeded,
 * otherwise return false (failure in this function is treated as function failed, parameter 5 in parser.c)
 * notes:
 * https://moodle.tau.ac.il/mod/forum/discuss.php?d=88124
 */
bool load(char* filepath, Mode mode) {
	FILE* fp;
	int m,n;

	fp = fopen(filepath, "r");
	if (fp == NULL) {
		printError(ReadingFileFailed, NULL, 0, 0);
		return false;
	}

	if (fscanf(fp, "%d %d", &m, &n)!=2) {
		printError(ReadingFileFailed, NULL, 0, 0);
		fclose(fp);
		return false;
	}

	if (m*n>99){
		fclose(fp);
		printError(BigBoard, NULL,0,0);
		return false;
	}

	createBoard(m, n);
	if (!fillBoard(fp, mode)) {
		if (puzzle->board!=NULL)
			cleanPuzzle();
		printError(IllegalBoard, NULL, 0, 0);
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}

/*
 * notes:
 * https://moodle.tau.ac.il/mod/forum/discuss.php?d=87711
 */
bool save(char* filepath, Mode mode) {
	FILE *fp = fopen(filepath, "w");
	int N=puzzle->blockNumOfCells, i, j;
	Cell *cell;

	if (fp == NULL){
		fclose(fp);
		return false;
	}
	fprintf(fp, "%d %d\n", puzzle->blockNumRow, puzzle->blockNumCol);
	for(i=0;i<N;i++){
			for(j=0;j<N;j++){
				cell = getCell(j+1,i+1);
				if (j>0){
					fprintf(fp, " ");
				}
				fprintf(fp, "%d", cell->value);
				if (cell->fixed==1 || (mode==Edit && cell->value!=0)){
					fprintf(fp, ".");
				}
			}
			fprintf(fp, "\n");
	}

	if (fclose(fp)!=0)
		return false;

	return true;
}

/* return a pointer to cell <x,y> */
Cell* getCell(int x, int y)
{
    int row = y - 1;
    int col = x - 1;
    return (&(puzzle->board[row][col]));
}

/* load puzzle for solve mode */
bool solve(char *filepath, Mode mode)
{
    if (puzzle->board != NULL)
    {
        cleanPuzzle();
    }

    return load(filepath, mode);
}

/* create new empty puzzle board */
void createBoard(int blockNumOfRows, int blockNumOfCols)
{
	int i, j;

    puzzle->blockNumRow = blockNumOfRows;
    puzzle->blockNumCol = blockNumOfCols;
    puzzle->blockNumOfCells = blockNumOfCols * blockNumOfRows;
    puzzle->numOfCells = puzzle->blockNumOfCells * puzzle->blockNumOfCells;
    puzzle->numOfEmptyCells = puzzle->numOfCells;
    puzzle->numOfErroneous = 0;

    puzzle->board = (Cell **)calloc(puzzle->blockNumOfCells, sizeof(Cell *));
    if (puzzle->board == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        exit(0);
    }
    for (i = 0; i < puzzle->blockNumOfCells; i++)
    {
        puzzle->board[i] = (Cell *)calloc(puzzle->blockNumOfCells, sizeof(Cell));
        if (puzzle->board[i] == NULL) /* calloc failed */
        {
            for (j = 0; j < i; j++)
            {
                free(puzzle->board[j]);
            }
            free(puzzle->board);
            printError(MemoryAllocFailed, NULL, 0, 0);
            exit(0);
        }
    }
}

/* create new empty 9x9 puzzle */
bool editNew()
{
    if (puzzle->board != NULL)
    {
        cleanPuzzle();
    }
    createBoard(3, 3);
    return true;
}

/* load puzzle for edit mode */
bool editFile(char *filepath, Mode mode)
{
    if (puzzle->board != NULL)
    {
        cleanPuzzle();
    }
    return (load(filepath, mode));
}

/* 
 * print puzzle board
 * if mark=1 it marks the erroneous cells
 */
void printBoard(int mark)
{
    Cell *cell;
    int cellRow, cellCol;
    int i, j, k, r, l;
    int M = puzzle->blockNumRow; /* num of blocks in a row */
    int N = puzzle->blockNumCol; /* num of cells in a block row */
    int C = 4;           /* num of chars every cell take to print */
    int numOfdashes = M * (N * C + 1) + 1;

    for (l = 0; l < numOfdashes; l++) /* print first dashes row */
    {
        printf("-");
    }
    printf("\n");

    for (k = 0; k < N; k++) /* for every row of blocks */
    {
        for (r = 0; r < M; r++) /* for every row of a block */
        {
            for (j = 0; j < M; j++) /* for every col of blocks */
            {
                printf("|");
                for (i = 0; i < N; i++) /* for every col of a block */
                {
                    cellRow = (k * M) + r + 1;
                    cellCol = (j * N) + i + 1;
                    cell = getCell(cellCol, cellRow);
                    if (cell->value == 0) /* empty cell */
                    {
                        printf("   ");
                    }
                    else
                    {
                        printf(" %2d", cell->value);
                    }
                    if (cell->fixed) /* fixed cell */
                    {
                        printf(".");
                    }
                    else
                    {
                        if (mark && cell->numOfCollisions) /* erroneous cell */
                        {
                            printf("*");
                        }
                        else
                        {
                            printf(" ");
                        }
                    }
                }
            }
                printf("|\n");
            }
            for (l = 0; l < numOfdashes; l++)
            {
                printf("-");
            }
            printf("\n");
        }
        return;
    }

/* 
 * set cell <x,y> value to z
 * update collisions with the previous value and the new value
 * update the number of empty cells
 * if all the cells are not empty and game mode is solve
 * check if the puzzle can be solved  
 * print a message and update game mode according to the check
 *  */
Move* set(int x, int y, int z, Mode mode)
{
    Move *head = NULL;
    Cell *cell;
    cell = getCell(x, y);
    if (mode == Solve && cell->fixed)
    {
        printf("Error: cell is fixed\n");
        return head;
    }
    if (cell->value==z)
    {
        return head;
    }
    if (cell->value == 0)
    {
        if (z != 0)
        {
            puzzle->numOfEmptyCells--;
        }
    }
    else
    {
        if (z == 0)
        {
            puzzle->numOfEmptyCells++;
        }
    }
    updateCollisions(x, y, z);
    addToList(&head, x, y, cell->value, z);
    cell->value = z;
    return head;
}

bool isSolved()
    {
	return (!(puzzle->numOfEmptyCells) && !isErroneous() ? true : false);
}

/* 
 * substruct cell num of collision by 1
 * and update puzzle's num of erroneous accordingly
 */
void subCollision(Cell *cell)
{
    ((cell->numOfCollisions)--);
    if (!(cell->numOfCollisions))
    {
        ((puzzle->numOfErroneous)--);
    }
}

/* 
 * add 1 to cell num of collision
 * and update puzzle's num of erroneous accordingly
 */
void addCollision(Cell *cell)
{
    if (!(cell->numOfCollisions))
    {
        ((puzzle->numOfErroneous)++);
    }
    ((cell->numOfCollisions)++);
}

/* update all the collision of cell <x,y> row */
void updateRowCollisions(int x, int y, int newValue)
{
    int i;
    Cell *cell = getCell(x, y);
    Cell *colCell;
    int oldValue = cell->value;
    /*if (oldValue==newValue)
    {
        return;
    }
    cell->numOfCollisions = 0; *//* initialize cell <x,y> num of collisions */
    for (i=0; i<puzzle->blockNumOfCells; i++)
    {
    	if (i+1 == x)
    	{
    		continue;
    	}
        colCell = getCell(i+1,y);
        if ((colCell->value == oldValue) && (oldValue != 0)) /* collision with the old value */
        {
            subCollision(colCell);
        }
        if ((colCell->value == newValue) && (newValue != 0)) /* collision with the new value */
        {
            addCollision(cell);
            addCollision(colCell);
        }
    }
}

/* update all the collision of cell <x,y> col */
void updateColCollisions(int x, int y, int newValue)
{
    int i;
    Cell *cell = getCell(x, y);
    Cell *colCell;
    int oldValue = cell->value;
    /*if (oldValue==newValue)
    {
        return;
    }
    cell->numOfCollisions = 0;*/ /* initialize cell <x,y> num of collisions */
    for (i=0; i<puzzle->blockNumOfCells; i++)
    {
    	if (i+1 == y)
    	{
    		continue;
    	}
        colCell = getCell(x,i+1);
        if ((colCell->value == oldValue) && (oldValue != 0)) /* collision with the old value */
        {
            subCollision(colCell);
        }
        if ((colCell->value == newValue) && (newValue != 0)) /* collision with the new value */
        {
            addCollision(cell);
            addCollision(colCell);
        }
    }
}

/* 
 * return the first row in the block 
 * @pre: row is 0-based
 */
int firstRowInBlock(int row, int blockNumRows)
{
	return (row / blockNumRows) * blockNumRows;
}

/*
 * return the first col in the block 
 * @pre: col is 0-based
 */
int firstColInBlock(int col, int blockNumCols)
{
	return (col / blockNumCols) * blockNumCols;
}

/* update all the collision of cell <x,y> block */
void updateBlockCollisions(int x, int y, int newValue)
{
    int i, j, firstRow, firstCol;
    Cell *cell = getCell(x, y);
    Cell *colCell;
    int oldValue = cell->value;
    /*if (oldValue==newValue)
    {
        return;
    }
    cell->numOfCollisions = 0;*/ /* initialize cell <x,y> num of collisions */
    
    firstRow = firstRowInBlock(y-1, puzzle->blockNumRow); /* index of first row in the block */
	firstCol = firstColInBlock(x-1, puzzle->blockNumCol); /* index of first column in the block */

    for (i = firstRow; i < firstRow + puzzle->blockNumRow; i++)
	{
    	if (i+1==y) /* do not check the same row twice */
    		continue;
		for (j = firstCol; j < firstCol + puzzle->blockNumCol; j++)
		{
			if (j+1==x) /* do not check the same column twice */
				continue;

			colCell = getCell(j+1, i+1); 
            if ((colCell->value == oldValue) && (oldValue != 0)) /* collision with the old value */
            {
                subCollision(colCell);
            }
            if ((colCell->value == newValue) && (newValue != 0)) /* collision with the new value */
            {
                addCollision(cell);
                addCollision(colCell);
            }
		}
	}
}

/* update collisions of the new and old values */
void updateCollisions(int x, int y, int newValue)
{
    Cell* cell=getCell(x,y);
    if (cell->numOfCollisions)
    {
        puzzle->numOfErroneous--;
    }
    cell->numOfCollisions=0;
    updateRowCollisions(x, y, newValue);
    updateColCollisions(x, y, newValue);
    updateBlockCollisions(x, y, newValue);
}

bool isErroneous() {
    return (puzzle->numOfErroneous > 0 ? true : false);
}

/* check if the board is solvable */
bool validate(bool printResult)
{
    if (ILPSolvable(puzzle))
    {
    	if (printResult)
    		printf("Validation passed: board is solvable\n");
        return true;
    }
    else
    {
    	if (printResult)
    		printf("Validation failed: board is unsolvable\n");
        return false;
    }
}

/* 
 * fill empty cells using LP
 * fill only legal values with score greater than threshold
 */
Move* guess(float threshold, Mode mode)
{
    Move *head = NULL;
    Puzzle *LPSolution;
    Cell *cell;
    Move* m;
    int i, j;
    
    LPSolution = LPSolver(puzzle, threshold);
    for (i = 0; i < puzzle->blockNumOfCells; i++)
    {
        for (j = 0; j < puzzle->blockNumOfCells; j++)
        {
            cell = getCell(i + 1, j + 1);
            if (!(cell->fixed))
            {
            	m=set(i + 1, j + 1, LPSolution->board[j][i].value, mode);
                concat(&head, &m);
        }
    }
    }
    return head;
}

void hint(int x, int y)
{
    Puzzle *ILPSolution;
    Cell *cell;
    int value;
    cell = getCell(x, y);
    if (cell->fixed)
    {
        printf("Error: cell <%d,%d> is fixed\n", x, y);
        return;
    }
    if (cell->value)
    {
        printf("Error: cell <%d,%d> already contains a value\n", x, y);
    }
    else
    {
        ILPSolution = ILPSolver(puzzle);
        value = ILPSolution->board[y - 1][x - 1].value;
        printf("Hint: set cell <%d,%d> to %d\n", x, y, value);
    }
    return;
}

void guessHint(int x, int y)
{
    Cell *cell;
    float *values;
    int i;
    cell = getCell(x, y);
    if (cell->fixed)
    {
        printf("Error: cell <%d,%d> is fixed\n", x, y);
        return;
    }
    if (cell->value)
    {
        printf("Error: cell <%d,%d> already contains a value\n", x, y);
    }
    else
    {
        values = (float *)calloc(puzzle->blockNumOfCells, sizeof(float));
        values = LPCellValues(puzzle, 0, cell, values);
        printf("Hint: cell <%d,%d> legal values are\n", x, y);
        for (i = 0; i < puzzle->blockNumOfCells; i++)
        {
            if (values[i] > 0)
            {
                printf("%d with score %f\n", i + 1, values[i]);
            }
        }
        free(values);
    }
    return;
}

void numSolution()
{
    int solNum;
    solNum = backtrack(puzzle);
    printf("The number of solutions is %d", solNum);
    return;
}

int numOfEmptyCells() {
	return puzzle->numOfEmptyCells;
}

/* values[0] = num of legal values
 * values[i] = 1 if i is legal value for cell <x,y> and 0 otherwise
 * if cell <x,y> value is v!=0 return values[0]=values[v]=1 and 0 anywhere else
 * does not assume values is initialized
 */
int *numOfCellSol(int x, int y, int *values)
{
    int i, j, value, firstCol, firstRow;
    Cell *cell, *colCell;

    cell = getCell(x, y);
    if (cell->value) /* cell is not empty */
    {
        values[0]=1;
        for (i=0; i<puzzle->blockNumOfCells; i++)
        {
            if (cell->value==i+1)
            {
                values[i+1]=1;
            }
            else
            {
                values[i+1]=0;
            }
            
        }
        return values; 
    }

    for (i=0; i<puzzle->blockNumOfCells; i++)
    {
        values[i+1] = 1;
    }

    /* update values according to col */
    for (i=0; i<puzzle->blockNumOfCells; i++)
    {
        cell = getCell(x, i+1);
        value = cell->value;
        values[value] = 0;
    }

    /* update values according to row */
    for (i=0; i<puzzle->blockNumOfCells; i++)
    {
        cell = getCell(i+1, y);
        value = cell->value;
        values[value] = 0;
    }

    /* update values according to block */
    firstCol = firstColInBlock(x-1, puzzle->blockNumCol);
    firstRow = firstRowInBlock(y-1, puzzle->blockNumRow);
    for (i = firstRow; i < firstRow + puzzle->blockNumRow; i++)
	{
		for (j = firstCol; j < firstCol + puzzle->blockNumCol; j++)
		{
			colCell = getCell(j+1, i+1); 
            value = colCell->value;
            values[value] = 0;
		}
	}

    /* update values[0] according to the number of valid cell sol */
    value = 0;
    for (i=0; i<puzzle->blockNumOfCells; i++)
    {
        if (values[i+1])
        {
            value++;
        }
    }
    values[0] = value;

    return values;
}

Move* autoFill(Mode mode)
{
    Move *head = (Move*)calloc(1, sizeof(Move));
    int i, j, k, value;
    Cell *cell;
    int *values = (int *)calloc((puzzle->blockNumOfCells) + 1, sizeof(int));
    Puzzle newPuzzle = {0, 0, 0, 0, 0, 0, 0};
    Puzzle *toFill = &newPuzzle; /* create new empty puzzle */
    Move* m;

    toFill->board = (Cell **)calloc(toFill->blockNumOfCells, sizeof(Cell *)); /* create empty board to the new puzzle */
    if (toFill->board == NULL)
    { /* calloc failed */
        printError(MemoryAllocFailed, NULL,0,0);
    }
    for (i = 0; i < toFill->blockNumOfCells; i++)
    {
        toFill->board[i] = (Cell *)calloc(toFill->blockNumOfCells, sizeof(Cell));
        if (toFill->board[i] == NULL)
        { /* calloc failed */
            for (j = 0; j < i; j++)
            {
                free(toFill->board[j]);
            }
            free(toFill->board);
            printError(MemoryAllocFailed, NULL,0,0);
        }
    }

    for (i = 0; i < puzzle->blockNumOfCells; i++)
    { /* fill the new puzzle board with obvious values only */
        for (j = 0; j < puzzle->blockNumOfCells; j++)
        {
            cell = getCell(i, j);
            if (!(cell->value))
            {
                values = numOfCellSol(i, j, values);
                if (values[0] == 1)
                {
                    for (k = 1; k < puzzle->blockNumOfCells; k++)
                    {
                        if (values[k] == 1)
                        {
                            value = k;
                            break;
                        }
                    }
                    toFill->board[j][i].value = value;
                }
            }
        }
    }

    for (i = 0; i < puzzle->blockNumOfCells; i++)
    { /* fill the original puzzle board with the obvious values from the new puzzle board */
        for (j = 0; j < puzzle->blockNumOfCells; j++)
        {
            cell = getCell(i, j);
            if (!(cell->value))
            {
                value = toFill->board[j][i].value;
                if (value)
                {
                	m=set(i + 1, j + 1, value, mode);
                    concat(&head, &m);
                }
            }
        }
    }

    for (i = 0; i < toFill->blockNumOfCells; i++)
    { /* free allocated memory */
        free(toFill->board[i]);
    }
    free(toFill->board);
    free(values);

    return head;
}

void Exit()
{
	if (puzzle->board!=NULL)
		cleanPuzzle();
    printf("Exiting...\n");
    exit(0);
}
