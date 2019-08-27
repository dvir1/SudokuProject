#include "ILP.h"
#include "gurobi_c.h"

int ***variables=NULL;

int addVars(GRBmodel **model, GRBenv **env, int numOfVarsToAdd, double *obj, char *vtype)
{
    int error = 0;
    double upperBound=1.0;
    error = GRBaddvars(*model, numOfVarsToAdd, 0, NULL, NULL, NULL, obj, NULL, &upperBound, vtype, NULL);
    if (error)
    {
        printf("ERROR %d GRBaddvars(): %s\n", error, GRBgeterrormsg(*env));
        return -1;
    }
    return 0;
}

void initVariables(int blockNumOfCells)
{
    int i, j, k, l;

    variables = (int ***)calloc(blockNumOfCells, sizeof(int **));
    if (variables == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        exit(0);
    }

    for (i = 0; i < blockNumOfCells; i++)
    {
        variables[i] = (int **)calloc(blockNumOfCells, sizeof(int *));
        if (variables[i] == NULL) /* calloc failed */
        {
            for (j = 0; j < i; j++)
            {
                free(variables[j]);
            }
            free(variables);
            printError(MemoryAllocFailed, NULL, 0, 0);
            exit(0);
        }
    }

    for (i = 0; i < blockNumOfCells; i++)
    {
        variables[i] = (int **)calloc(blockNumOfCells, sizeof(int *));
        if (variables[i] == NULL) /* calloc failed */
        {
            for (k = 0; k < i; k++)
            {
                for (l = 0; l < blockNumOfCells; l++)
                {
                    free(variables[k][l]);
                }
                free(variables[k]);
            }
            free(variables);
            printError(MemoryAllocFailed, NULL, 0, 0);
            exit(0);
        }
        else
        {
            for (j = 0; j < blockNumOfCells; j++)
            {
                variables[i][j] = (int *)calloc(blockNumOfCells, sizeof(int));
                if (variables[i][j] == NULL) /* calloc failed */
                {
                    for (l = 0; l < j; l++)
                    {
                        free(variables[i][l]);
                    }
                    free(variables[i]);
                    for (k = 0; k < i; k++)
                    {
                        for (l = 0; l < blockNumOfCells; l++)
                        {
                            free(variables[k][l]);
                        }
                        free(variables[k]);
                    }
                    free(variables);
                    printError(MemoryAllocFailed, NULL, 0, 0);
                    exit(0);
                }
            }
        }
    }
}

int setIntAttr(GRBmodel **model, GRBenv **env)
{
    int error = 0;
    error = GRBsetintattr(*model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
    if (error)
    {
        printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(*env));
        return -1;
    }
    return 0;
}

void freeVariables(int blockNumOfCells)
{
    int i, j /*, k*/;
    if (variables==NULL)
    	return;
    for (i = 0; i < blockNumOfCells; i++)
    {
        for (j = 0; j < blockNumOfCells; j++)
        {
            free(variables[i][j]);
        }
        free(variables[i]);
    }
    free(variables);
}

bool inArray(int val, double *array, int N)
{
	int i=0;

	for (i=0;i<N;i++)
	{
		if (array[i]==val)
			return true;
	}

	return false;
}

void randomCoefficients(double *array, int N)
{
	int i=0, r=0;

	do
	{
		r=(rand()%(N-i))+1;
		if (!inArray(r,array,N))
		{
			array[i]=r;
			i++;
		}
	}
	while(i<N);
}

int updateVariables(Puzzle *puzzle, bool isILP, GRBmodel **model, GRBenv **env)
{
    int i=0, j=0, k=0, cnt = 1, error = 0;
    int blockNumOfCells = puzzle->blockNumOfCells;
    double *obj=NULL, upperBound=1.0; /* obj is coefficients array */
    double *upperBoundPtr = (isILP ? NULL : &upperBound);
    int *values=NULL; /* array of legal values of cell */
    char *vtype=NULL; /* binary/continuous */
    Cell *cell=NULL;
    int maxNumOfVars = blockNumOfCells*blockNumOfCells*blockNumOfCells;

    print("A");
    printf("blockNumOfCells=%d\n", blockNumOfCells);
    values = (int *)calloc(blockNumOfCells + 1, sizeof(int));
    if (values == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        exit(0);
    }
    print("B");
    obj = (double *)calloc(maxNumOfVars, sizeof(double));
    if (obj == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        free(values);
        exit(0);
    }
    print("C");
    vtype = (char *)calloc(maxNumOfVars, sizeof(char));
    if (vtype == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        free(values);
        free(obj);
        exit(0);
    }
    print("D");
    for (i = 1; i < blockNumOfCells + 1; i++)
    {
        for (j = 1; j < blockNumOfCells + 1; j++)
        {
            cell = getCell(puzzle, i, j);
            if (!(cell->value))
            {
                values = numOfCellSol(puzzle, i, j, values);
                for (k = 1; k < blockNumOfCells + 1; k++)
                {
                    if (values[k])
                    {
                        variables[i - 1][j - 1][k - 1] = cnt;

                        /*if (isILP)
                        {
                            obj[cnt - 1] = 1;
                            vtype[cnt - 1] = GRB_BINARY;
                        }
                        else
                        {
                            obj[cnt - 1] = randomCoefficient(blockNumOfCells);
                            vtype[cnt - 1] = GRB_CONTINUOUS;
                        }*/
                        cnt++;
                    }
                }
            }
        }
    }
    cnt--;
    if (isILP)
    {
    	for(i=0; i<cnt; i++)
    	{
    		obj[i] = 1.0;
    		vtype[i] = GRB_BINARY;
    	}
    }
    else
    {
    	for(i=0; i<cnt; i++)
    	{
    		vtype[i] = GRB_CONTINUOUS;
    	}
    	randomCoefficients(obj, cnt);
    	for(i=0; i<cnt; i++)
		{
			printf("%f, ", obj[i]);
		}
    	printf("\n");
    }
    print("E");
    printf("*model=%d, cnt=%d, 3rd=%d, 4th=NULL, 5th=NULL 6th=NULL, obj=%f, 8th=NULL, upperBound=%f, vtype=%c, 11th=NULL\n", (*model==NULL?0:1), cnt, 0, obj[cnt-1], (upperBoundPtr==NULL?0:*upperBoundPtr), vtype[cnt-1]);
    error = GRBaddvars(*model, cnt, 0, NULL, NULL, NULL, obj, NULL, upperBoundPtr, vtype, NULL);
	if (error)
	{
		printf("ERROR %d GRBaddvars(): %s\n", error, GRBgeterrormsg(*env));
		free(values);
		free(obj);
		free(vtype);
		return -1;
	}
    print("F");
    if (setIntAttr(model, env) == -1)
    {
        free(values);
        free(obj);
        free(vtype);
        return -1;
    }
    print("G");
    free(values);
    free(obj);
    free(vtype);
    return cnt;
}

int addConstraint(GRBmodel *model, GRBenv *env, int numOfVars, int *ind, double *val, char *consName)
{
    int error = 0;
    error = GRBaddconstr(model, numOfVars, ind, val, GRB_EQUAL, 1.0, consName);
    if (error)
    {
        printf("ERROR %d 1st GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
        return -1;
    }
    /*error = GRBaddconstr(model, numOfVars, ind, val, GRB_LESS_EQUAL, 1.0, consName);
	if (error)
	{
		printf("ERROR %d 1st GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
		return -1;
	}
    error = GRBaddconstr(model, numOfVars, ind, val, GRB_GREATER_EQUAL, 1.0, consName);
	if (error)
	{
		printf("ERROR %d 1st GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
		return -1;
	}*/
    return 0;
}

int addCellsConstraints(Puzzle *puzzle, GRBmodel *model, GRBenv *env)
{
    int i, j, k, num = 0, cnt = 0;
    int blockNumOfCells = puzzle->blockNumOfCells;
    char temp[9] = {0};
    int *ind;
    double *val;
    print("a");
    ind = (int *)calloc(blockNumOfCells, sizeof(int));
    if (ind == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        exit(0);
    }
    print("b");
    val = (double *)calloc(blockNumOfCells, sizeof(double));
    if (val == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        free(ind);
        exit(0);
    }
    print("c");
    for (i = 1; i < blockNumOfCells + 1; i++)
    {
        for (j = 1; j < blockNumOfCells + 1; j++)
        {
            for (k = 1; k < blockNumOfCells + 1; k++)
            {
                if (variables[i - 1][j - 1][k - 1])
                {
                    ind[cnt] = variables[i - 1][j - 1][k - 1] - 1;
                    val[cnt] = 1;
                    cnt++;
                }
            }
            sprintf(temp, "a%d", num);
            if (addConstraint(model, env, cnt, ind, val, temp) == -1)
            {
                free(ind);
				free(val);
				return -1;
            }
            num++;
            cnt = 0;
        }
    }
    print("d");
    free(ind);
    free(val);
    return 0;
}

int addRowsConstraints(Puzzle *puzzle, GRBmodel *model, GRBenv *env)
{
    int i, j, k, num = 0, cnt = 0;
    int blockNumOfCells = puzzle->blockNumOfCells;
    char temp[9] = {0};
    int *ind;
    double *val;
    ind = (int *)calloc(blockNumOfCells, sizeof(int));
    if (ind == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        exit(0);
    }
    val = (double *)calloc(blockNumOfCells, sizeof(double));
    if (val == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        free(ind);
        exit(0);
    }
    for (j = 1; j < blockNumOfCells + 1; j++)
    {
        for (k = 1; k < blockNumOfCells + 1; k++)
        {
            for (i = 1; i < blockNumOfCells + 1; i++)
            {
                if (variables[i - 1][j - 1][k - 1])
                {
                    ind[cnt] = variables[i - 1][j - 1][k - 1] - 1;
                    val[cnt] = 1;
                    cnt++;
                }
            }
            sprintf(temp, "b%d", num);
            if (addConstraint(model, env, cnt, ind, val, temp) == -1)
            {
                free(ind);
				free(val);
				return -1;
            }
            num++;
            cnt = 0;
        }
    }
    free(ind);
    free(val);
    return 0;
}

int addColsConstraints(Puzzle *puzzle, GRBmodel *model, GRBenv *env)
{
    int i, j, k, num = 0, cnt = 0;
    int blockNumOfCells = puzzle->blockNumOfCells;
    char temp[9] = {0};
    int *ind;
    double *val;
    ind = (int *)calloc(blockNumOfCells, sizeof(int));
    if (ind == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        exit(0);
    }
    val = (double *)calloc(blockNumOfCells, sizeof(double));
    if (val == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        free(ind);
        exit(0);
    }
    for (i = 1; i < blockNumOfCells + 1; i++)
    {
        for (k = 1; k < blockNumOfCells + 1; k++)
        {
            for (j = 1; j < blockNumOfCells + 1; j++)
            {
                if (variables[i - 1][j - 1][k - 1])
                {
                    ind[cnt] = variables[i - 1][j - 1][k - 1] - 1;
                    val[cnt] = 1;
                    cnt++;
                }
            }
            sprintf(temp, "c%d", num);
            if (addConstraint(model, env, cnt, ind, val, temp) == -1)
            {
                free(ind);
				free(val);
				return -1;
            }
            num++;
            cnt = 0;
        }
    }
    free(ind);
    free(val);
    return 0;
}

int addBlocksConstraints(Puzzle *puzzle, GRBmodel *model, GRBenv *env)
{
    int i, j, k, n, m, num = 0, index, cnt = 0;
    int blockNumOfCells = puzzle->blockNumOfCells;
    int blockNumOfCols = puzzle->blockNumCol;
    int blockNumOfRows = puzzle->blockNumRow;
    char temp[9] = {0};
    int *ind;
    double *val;
    ind = (int *)calloc(blockNumOfCells, sizeof(int));
    if (ind == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        exit(0);
    }
    val = (double *)calloc(blockNumOfCells, sizeof(double));
    if (val == NULL) /* calloc failed */
    {
        printError(MemoryAllocFailed, NULL, 0, 0);
        free(ind);
        exit(0);
    }
    for (n = 0; n < blockNumOfRows; n++)
    {
        for (m = 0; m < blockNumOfCols; m++)
        {
            for (k = 1; k < blockNumOfCells + 1; k++)
            {
                for (i = 1; i < blockNumOfCols + 1; i++)
                {
                    for (j = 1; j < blockNumOfRows + 1; j++)
                    {
                        index = variables[(n * blockNumOfCols) + (i - 1)][(m * blockNumOfRows) + (j - 1)][k - 1];
                        if (index)
                        {
                            ind[cnt] = index - 1;
                            val[cnt] = 1;
                            cnt++;
                        }
                    }
                }
                sprintf(temp, "d%d", num);
                if (addConstraint(model, env, cnt, ind, val, temp) == -1)
                {
                    free(ind);
                    free(val);
                    return -1;
                }
                num++;
                cnt = 0;
            }
        }
    }
    free(ind);
    free(val);
    return 0;
}

int findSolution(Puzzle *puzzle, bool isILP, int *numOfVariables, double *sol)
{
    GRBenv *env = NULL;
    GRBenv **envPtr=&env;
    GRBmodel *model = NULL;
    GRBmodel **modelPtr = &model;
    int error=0;
    int optimstatus=0;
    int success=0;
    double objval=0;

    /* if statement added to avoid unused error (pedantic-error) */
	if (puzzle==NULL && isILP==true && numOfVariables==NULL && sol==NULL)
		return 0;

    /* Create environment - log file is mip1.log */
	error = GRBloadenv(&env, "mip1.log");
	if (error) {
		printf("ERROR %d GRBloadenv(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}

	error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
	if (error) {
		printf("ERROR %d GRBsetintparam(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}

	/* Create an empty model named "mip1" */
	error = GRBnewmodel(env, &model, "mip1", 0, NULL, NULL, NULL, NULL, NULL);
	if (error) {
		printf("ERROR %d GRBnewmodel(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}

	/* Add variables */
	initVariables(puzzle->blockNumOfCells);

	/* Change objective sense to maximization */
	print("1");
	*numOfVariables = updateVariables(puzzle, isILP, modelPtr, envPtr);
	print("2");

	sol = (double *)calloc(*numOfVariables, sizeof(double));
	if (sol == NULL) /* calloc failed */
	{
		printError(MemoryAllocFailed, NULL, 0, 0);
		exit(0);
	}
	print("3");
	/* update the model - to integrate new variables */
	error = GRBupdatemodel(model);
	if (error) {
		printf("ERROR %d GRBupdatemodel(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}
	print("4");
	/* add constraints to model */
	if (addCellsConstraints(puzzle, model, env) == -1)
	{
		success = -1;
		goto END;
	}
	print("5");
	if (addRowsConstraints(puzzle, model, env) == -1)
	{
		success = -1;
		goto END;
	}
	print("6");
	if (addColsConstraints(puzzle, model, env) == -1)
	{
		success = -1;
		goto END;
	}
	print("7");
	if (addBlocksConstraints(puzzle, model, env) == -1)
	{
		success = -1;
		goto END;
	}
	print("8");
	/* Optimize model - need to call this before calculation */
	error = GRBoptimize(model);
	if (error) {
		printf("ERROR %d GRBoptimize(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}
	print("9");
	/* Write model to 'mip1.lp' - this is not necessary but very helpful */
	error = GRBwrite(model, "mip1.lp");
	if (error) {
		printf("ERROR %d GRBwrite(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}
	print("10");
	/* Get solution information */
	error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
	if (error) {
		printf("ERROR %d GRBgetintattr(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}
	print("11");
	/* get the objective -- the optimal result of the function */
	/* not necessary but may be harmful to gurobi using */
	/*error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
	if (error) {
		printf("ERROR %d GRBgetdblattr(): %s\n", error, GRBgeterrormsg(env));
		return false;
	}*/

	/* print results */
	printf("\nOptimization complete\n");

	/* solution found */
	if (optimstatus == GRB_OPTIMAL) {
		success = 1;
		printf("Optimal objective: %.4e\n", objval);
	}
	/* no solution found */
	else if (optimstatus == GRB_INF_OR_UNBD) {
		success = 0;
		printf("Model is infeasible or unbounded\n");
		goto END;
	}
	/* error or calculation stopped */
	else {
		printf("Optimization was stopped early\n");
		success = -1;
		goto END;
	}

	print("beginning of if (success)");
	/* get the solution - the assignment to each variable */
	/* 3-- number of variables, the size of "sol" should match */
	error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, *numOfVariables, sol);
	if (error) {
		printf("ERROR %d GRBgetdblattrarray(): %s\n", error, GRBgeterrormsg(env));
		success = -1;
		goto END;
	}
	print("end of if (success)");

	print("12");

	/* IMPORTANT !!! - Free model and environment */

	END:
	if (model!=NULL)
		GRBfreemodel(model);
	if (env!=NULL)
		GRBfreeenv(env);

	return success;
}

/* run ILP and return if there is a solution or not */
int ILPSolvable(Puzzle *puzzle)
{
	int numOfVariables;
	double *sol = NULL;
	int success = 0;
	print("in ILPSolvable");
	success = findSolution(puzzle, true, &numOfVariables, sol);
	print("after findSolution");
	if (sol!=NULL)
		free(sol);
	freeVariables(puzzle->blockNumOfCells);
	print("after freeVariables");
	return success;
}

void fillIntSolution(Puzzle *puzzle, double *sol, Mode mode)
{
	Move *dummyMove=NULL;
    int i, j, k, index, cellSol;
    int blockNumOfCells = puzzle->blockNumOfCells;
    for (i = 1; i < blockNumOfCells + 1; i++)
    {
        for (j = 1; j < blockNumOfCells + 1; j++)
        {
            for (k = 1; k < blockNumOfCells + 1; k++)
            {
                index = variables[i - 1][j - 1][k - 1];
                if (index)
                {
                    cellSol = sol[index - 1];
                    if (cellSol)
                    {
                    	dummyMove = setCell(puzzle, i, j, k, mode);
                    	deleteList(dummyMove);
                    }
                }
            }
        }
    }
}

/* run ILP and return a solution */
Puzzle* ILPSolver(Puzzle *puzzle)
{
	int numOfVariables /*, value=0, index, k*/;
	double *sol = NULL;
	int success;
	success = findSolution(puzzle, true, &numOfVariables, sol);
	if (success == 1)
	{
		fillIntSolution(puzzle, sol, Edit);
	}
	if(sol!=NULL)
		free(sol);
	freeVariables(puzzle->blockNumOfCells);
	return puzzle;
}

/* 
 * run LP and return a new puzzle such that
 * all empty un-fixed cell values with a score
 * of threshold or greater are filled.
 * if several values hold for the same cell the value
 * is randomly chosen according to the score as the probability.
 * fills only legal values
 */
Move* LPSolver(Puzzle *puzzle, double threshold)
{
    if (puzzle==NULL && threshold != 0)
    {
        return NULL;
    }
    return NULL;
}

/* 
 * return a list such that in index i there is the
 * probability that the value of cell is i+1 
 */
double* LPCellValues(Puzzle *puzzle, double threshold, int x, int y, double *values)
{
    if(puzzle==NULL && threshold==0.0 && x==0 && y==0 && values==NULL)
    	return values;
    return NULL;
}

int ILPCellSolver(Puzzle *puzzle, int x, int y)
{
	if (puzzle==NULL && x==0 && y==0)
		return 0;
	return 0;
}
