#include "Stack.h"

/* 
 * Stack Module - Source
 * implement all stack operations
 */

/* initialize stk */
void init(Stack *stk) {
	stk->size = 0;
	stk->top = NULL;
}

/* insert new stack node to the top of stk */
bool push(Puzzle *puzzle, int col, int row, Stack *stk) {
	StackNode *p = NULL;
	int *values = (int*)calloc(puzzle->blockNumOfCells+1, sizeof(int));
	p = (StackNode*)calloc(1, sizeof(StackNode));
	/*printf("13. calloc int *values(p<%d,%d>->options) - push, Stack\n", col, row);*/
	/*printf("14. calloc StackNode *p<%d,%d> - push, Stack\n", col, row);*/
	if (p != NULL) { /* otherwise calloc failed */
		p->col = col;
		p->row = row;
		p->options = numOfCellSol(puzzle, col, row, values);
		p->next = stk->top;
		stk->top = p;
		stk->size++;
		return true;
	}
	else
		return false;
}

/* delete stk top item and return its options[0] */
int pop(Stack *stk) {
	int option;
	StackNode *p = NULL;

	option = topOption(stk);
	free(stk->top->options);
	/*printf("13. free int *values(p<%d,%d>->options) - pop, Stack\n", p->col, p->row);*/
	p = stk->top;
	stk->top = stk->top->next;
	stk->size--;
	/*printf("14. free StackNode *p<%d,%d> - pop, Stack\n", p->col, p->row);*/
	free(p);

	return option;
}

/* return stk top item's col */
int topCol(Stack *stk) {
	if (stk->top == NULL)
	{
		return 0;
	}
	return (stk->top->col);
}

/* return stk top item's row */
int topRow(Stack *stk) {
	if (stk->top == NULL)
	{
		return 0;
	}
	return (stk->top->row);
}
/* return stk top item's options[0] */
int topOption(Stack *stk) {
	return (stk->top->options[0]);
}

/* return true iff stk is empty */
bool isEmpty(Stack *stk) {
	return (stk->size == 0 ? true : false);
}
