#ifndef AUXILARY_H_
#define AUXILARY_H_

#include "ErrorHandler.h"
#include "SingleLinkedList.h"
#include "DoublyLinkedList.h"
#include "Stack.h"

typedef enum Mode {Init, Edit, Solve} Mode;

typedef struct Cell {
	int fixed;
	int value;
	int numOfCollisions;
} Cell;
/*
typedef struct Move {
	int x;
	int y;
	int oldValue;
	int newValue;
	struct Move *nextMove;
} Move;
*/
/*
typedef struct Step {
	Move *move;
	struct Step *prevStep;
	struct Step *nextStep;
} Step;
*/
typedef struct Puzzle {
	Cell **board;
	int blockNumRow;
	int blockNumCol;
	int blockNumOfCells;
	int numOfCells;
	int numOfEmptyCells;
	int numOfErroneous;
} Puzzle;

bool isNumInRange(int num, int minNum, int maxNum);

#endif
