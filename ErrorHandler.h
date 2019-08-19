#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_
#define MAX_INPUT_CHARS 258

typedef enum ErrorMsg {
	InvalidCommand, IndexOutOfBounds, TooLongInput, FunctionFailed, MemoryAllocFailed,
	ReadingFileFailed, WritingFileFailed, Erroneous, Validate, WrongNumOfParams, WrongNumOfParamsBounds,
	CommandFailed, WrongMode, ParamOutOfBounds, ParamIsNotNum, EmptyCellsParamRange,
	GenerationFailed, NoMoreUndo, NoMoreRedo, FixedCell, CellHasValue, BigBoard, IllegalBoard,
	DimNotPositive
} ErrorMsg;

void printError(ErrorMsg err, char* param1, int bound1, int bound2);

#endif /* ERRORHANDLER_H_ */
