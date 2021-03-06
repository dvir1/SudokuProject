/*
 * how to use:
 * 1. create Stack object s
 * 2. call init(&s)
 * 3. check !isEmpty before calling pop
 * 4. notice: always use a pointer to Stack object as argument
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Auxilary.h"

/* 
 * Stack Module - Header
 * define stack node and stack
 * manage all stack operations
 */

/* define stack node */
typedef struct StackNode
{
	int col;
	int row;
	int *options;
	struct StackNode *next;
} StackNode;

/* define stack */
typedef struct Stack
{
	int size; /* size of stack */
	StackNode *top;
} Stack;

void init(Stack *stk);

bool push(Puzzle *puzzle, int col, int row, Stack *stk);

int pop(Stack *stk);

int topCol(Stack *stk);

int topRow(Stack *stk);

int topOption(Stack *stk);

bool isEmpty(Stack *stk);

#endif
