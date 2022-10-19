/*****************************************************   
 * Author : Avia Avikasis                            *
 * Reviewer: Maayan                                  *
 * 23/07/2022                                        *
 * Description : imlementation of calculator         *
 *                                                   *
 *****************************************************/
#include <assert.h> /* assert */
#include <stdlib.h> /* calloc , free */
#include <math.h>   /* pow */
#include <ctype.h>  /* isdigit */

#include "stack.h"  /* stack functions */
#include "calc.h"   /* calc function declarations */

#define OPERANDS_STACK_SIZE 100
#define OPERATORS_STACK_SIZE 100
#define CHAR_SIZE 1
#define WORD_SIZE sizeof(size_t)
#define LUT_SIZE 4
#define ASCII_SIZE 256
#define PLUS 43
#define MINUS 45
#define MULT 42
#define DIVIDE 47
#define POW 94
#define OPEN_ROUND_PAREN 40
#define CLOSE_ROUND_PAREN 41
#define OPEN_SQUARE_PAREN 91
#define CLOSE_SQUARE_PAREN 93
#define OPEN_CURLY_PAREN 123
#define CLOSE_CURLY_PAREN 125
#define SPACE 32
#define UNUSED(x) (void)x

typedef struct calc
{
	stack_ty *operators_stack;
	stack_ty *operands_stack;
	status_ty status;
}calculator_ty;

typedef enum 
{
	WAIT_FOR_NUM = 0,
 	WAIT_FOR_OPERATOR = 1,
 	ERROR = 2,
 	FINITE_STATE = 3
}states_ty;

typedef enum
{
	SUCCESS_FUNC = 0,
	FAIL = 1
}stat_status_ty;

typedef int(*state_func)(char*, char**, calculator_ty*);
typedef status_ty (*operate_func)(stack_ty *);

/* lut for all states transitions handlers */
static state_func states_lut[LUT_SIZE][ASCII_SIZE] = {0};
static int operators_perior_lut[100] = {0};
static operate_func operate_funcs_lut[100] = {0};

/* the main function of calculator */
double Calculate(const char *expression, status_ty *o_status);

/* transitions handlers functions */
static int BaseOperationsHandler(char *expression , char **next_input,
														 calculator_ty *calc);
static int InsertOperandHandler(char *expression , char **next_input,
														 calculator_ty *calc);
static int InvalidOperandHandler(char *expression , char **next_input,
														 calculator_ty *calc);
static int InvalidOperatorHandler(char *expression , char **next_input,
														 calculator_ty *calc);
static int CloseParenHandler(char *expression , char **next_input,
 														 calculator_ty *calc);
static int OpenParenHandler(char *expression , char **next_input, 
														 calculator_ty *calc);
static int SuccessHandler(char *expression , char **next_input,
														 calculator_ty *calc);

/* hendlers help functions */
static int OperatorPerior(char operator);
static int IsHeigherPriority(char operator1 , char operator2);
static char InverseParen(char paren);
static int InverseParenIsFound(char paren, stack_ty *stack);
static void Assert(char *expression , char **next_input, calculator_ty *calc);
static void ExecuteOperatorsTillOpenParenFound(char paren,
												 stack_ty *stack,
												 calculator_ty *calc);											

/* calculate functions */
static status_ty CalculateOneOperation(calculator_ty *calc);
static status_ty CalculateAllOperations(calculator_ty *calc);

/* create, init and destroy functions  */
static status_ty CreateCalc(calculator_ty *calc, status_ty *o_status);
static void InitStatesHandlersLut(void);
static void InitOperatorsPeriorLut(void);
static void InitOperateFunctionsLut(void);
static void DestroyCalc(calculator_ty *calc);

/* access to the transitions handlers lut */
static states_ty StatesLutAccess(states_ty state,
								 const char *expression, 
								 char **next_input,
								 calculator_ty *calc);
								 
/* operate functions */
static status_ty AddEvaluate(stack_ty *operands_stack);
static status_ty SubEvaluate(stack_ty *operands_stack);
static status_ty MultEvaluate(stack_ty *operands_stack);
static status_ty DivideEvaluate(stack_ty *operands_stack);
static status_ty PowEvaluate(stack_ty *operands_stack);
static status_ty InvalidOperate(stack_ty *stack);



/*---------------------- calculate -------------------------*/


double Calculate(const char *expression, status_ty *o_status)
{
	calculator_ty calc = {0};
	status_ty create_status = 0;
	double ret_value = 0;
	char *next_input = NULL;
	states_ty cur_state = WAIT_FOR_NUM;
	
	assert(NULL != expression);
	assert(NULL != o_status);
	
	create_status = CreateCalc(&calc, o_status);
	if(INSUFFICIENT_MEMORY_FAIL == create_status)
	{
		*o_status = INSUFFICIENT_MEMORY_FAIL;
		return 0;
	}
	
	next_input = (char*)expression;
	
	InitStatesHandlersLut();
	InitOperatorsPeriorLut();
	InitOperateFunctionsLut();

	while(ERROR != cur_state && FINITE_STATE != cur_state)
	{
		cur_state = StatesLutAccess(cur_state, expression, &next_input, &calc);
		expression = next_input;
	}

	*o_status = calc.status;
	if(!StackIsEmpty(calc.operands_stack))
	{
		ret_value = *(double*)StackPeek(calc.operands_stack);
	}
	DestroyCalc(&calc);
	
	return (SUCCESS == *o_status) ? ret_value : 0.00;
}


static status_ty CreateCalc(calculator_ty *calc, status_ty *o_status)
{
	assert(NULL != calc);
	
	calc->operators_stack = StackCreate(OPERATORS_STACK_SIZE, CHAR_SIZE);
	if(NULL == calc->operators_stack)
	{
		return INSUFFICIENT_MEMORY_FAIL; 
	}
	
	calc->operands_stack = StackCreate(OPERANDS_STACK_SIZE, WORD_SIZE);
	if(NULL == calc->operands_stack)
	{
		StackDestroy(calc->operators_stack);
		return INSUFFICIENT_MEMORY_FAIL; 
	}
	
	calc->status = *o_status;
	
	return SUCCESS;
}


static void DestroyCalc(calculator_ty *calc)
{
	StackDestroy(calc->operators_stack);
	StackDestroy(calc->operands_stack);
}


static void InitOperatorsPeriorLut(void)
{
	operators_perior_lut[MULT] = 1;
	operators_perior_lut[DIVIDE] = 1;
	operators_perior_lut[POW] = 2;
	operators_perior_lut[OPEN_ROUND_PAREN] = -1;
	operators_perior_lut[CLOSE_ROUND_PAREN] = -1;
	operators_perior_lut[OPEN_SQUARE_PAREN] = -1;
	
	operators_perior_lut[CLOSE_SQUARE_PAREN] = -1;
	operators_perior_lut[OPEN_CURLY_PAREN] = -1;
	operators_perior_lut[CLOSE_CURLY_PAREN] = -1;
}

static void InitOperateFunctionsLut(void)
{
	int i = 0;
	for( ; i < 100 ; ++i)
	{
		operate_funcs_lut[i] = &InvalidOperate;	
	}
	operate_funcs_lut[PLUS] = &AddEvaluate;
	operate_funcs_lut[MINUS] = &SubEvaluate;
	operate_funcs_lut[MULT]	= &MultEvaluate;
	operate_funcs_lut[DIVIDE] = &DivideEvaluate;
	operate_funcs_lut[POW] = PowEvaluate;
}


static states_ty StatesLutAccess(states_ty state,
								 const char *expression, 
								 char **next_input,
								 calculator_ty *calc)
{
	return states_lut[state][(int)*expression]
					((char*)expression, next_input, calc);	
}



/*------------------ hendlers help functions----------------*/


static int OperatorPerior(char operator)
{
	return operators_perior_lut[(int)operator];
}


static int IsHeigherPriority(char operator1 , char operator2)
{
	return (OperatorPerior(operator1) >= OperatorPerior(operator2));
}


static char InverseParen(char paren)
{
	switch(paren)
	{
		case CLOSE_ROUND_PAREN:
			return OPEN_ROUND_PAREN;
		
		case CLOSE_SQUARE_PAREN:
			return OPEN_SQUARE_PAREN;
			
		case CLOSE_CURLY_PAREN:
			return OPEN_CURLY_PAREN;
	}
	return 0;
}


static void ExecuteOperatorsTillOpenParenFound(char paren,
												 stack_ty *stack,
												 calculator_ty *calc)
{
	while(!StackIsEmpty(stack) && paren != *(char*)StackPeek(stack))
	{
		CalculateOneOperation(calc);
	}
}


static int InverseParenIsFound(char paren, stack_ty *stack)
{
	assert(NULL != stack);
	
	if(!StackIsEmpty(stack))
	{
		return (paren == *(char*)StackPeek(stack));
	}
	return 0;
}


static void Assert(char *expression , char **next_input, calculator_ty *calc)
{
	assert(NULL != expression);
	assert(NULL != next_input);
	assert(NULL != calc);
}


/*--------------------- handlers functions ---------------------*/


static int InvalidOperandHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	Assert(expression, next_input, calc);
	UNUSED(expression);
	UNUSED(next_input);
	
	calc->status = INVALID_EXPRESSION;
	return ERROR;
}


static int InvalidOperatorHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	Assert(expression, next_input, calc);
	UNUSED(expression);
	UNUSED(next_input);
	
	calc->status = INVALID_EXPRESSION;
	return ERROR;
}
	

static int CloseParenHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	char paren = 0;
	char inverse_paren = 0;
	Assert(expression, next_input, calc);
	
	paren = *expression;
	inverse_paren = InverseParen(paren); /* open paren that mach to paren */
	
	ExecuteOperatorsTillOpenParenFound(inverse_paren,
										 calc->operators_stack, calc);
	
	if(!InverseParenIsFound(inverse_paren, calc->operators_stack))
	{
		calc->status = INVALID_EXPRESSION;
		return ERROR;
	}
	
	StackPop(calc->operators_stack);
	++(*next_input);
	
	return WAIT_FOR_OPERATOR;	
}


static int OpenParenHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	char paren = 0;
	
	Assert(expression, next_input, calc);
	UNUSED(next_input);
	
	paren = *expression;
	
	StackPush(calc->operators_stack , &paren);
	++(*next_input);
	
	return WAIT_FOR_NUM;	
}


static int BaseOperationsHandler(char *expression, char **next_input,
														 calculator_ty *calc)
{
	char operation = 0;
	status_ty status = SUCCESS;
	Assert(expression, next_input, calc);
	
	operation = *expression;
	
	UNUSED(next_input);
	
	while(POW != operation && !StackIsEmpty(calc->operators_stack) && 
		IsHeigherPriority(*(char*)StackPeek(calc->operators_stack) , operation))
	{
		status = CalculateOneOperation(calc);
		if(SUCCESS != status)
		{
			calc->status = status;
			return ERROR;
		}
	}
	
	StackPush(calc->operators_stack , &operation);
	++(*next_input);
	
	return WAIT_FOR_NUM;
}


static int InsertOperandHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	double num = 0;
	Assert(expression, next_input, calc);
	UNUSED(calc);

	num = strtod(expression, next_input);
	StackPush(calc->operands_stack , &num);
	
	return WAIT_FOR_OPERATOR;
}


static int OperandSpaceHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	Assert(expression, next_input, calc);
	UNUSED(calc);
	
	++(*next_input);
	
	return WAIT_FOR_NUM;
}

static int OperatorSpaceHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	Assert(expression, next_input, calc);
	UNUSED(calc);
	
	++(*next_input);

	return WAIT_FOR_OPERATOR;
}

static int InsertUnaryOperatorHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	double num = 0;
	Assert(expression, next_input, calc);
		
	if(isdigit(*(expression + 1)))
	{
		num = strtod(expression++, next_input);
		StackPush(calc->operands_stack , &num);
		return WAIT_FOR_OPERATOR;			
	}
	calc->status = INVALID_EXPRESSION;
	return ERROR;
}


static int SuccessHandler(char *expression , char **next_input,
														 calculator_ty *calc)
{
	Assert(expression, next_input, calc);
	UNUSED(expression);
	UNUSED(next_input);
	
	calc->status = CalculateAllOperations(calc);

	return FINITE_STATE;	
}


/*---------------- init functions -------------------------*/


static void InitDigits(state_func* funcs_array)
{
	int i = '0';
	for( ; i <= '9' ; ++i)
	{
		funcs_array[i] = &InsertOperandHandler;
	}
}


static void InitStatesHandlersLut(void)
{
	int i = 0;
	
	for(i = 0 ; i < ASCII_SIZE ; ++i)
	{
		states_lut[WAIT_FOR_NUM][i] = &InvalidOperandHandler;
	}
	
	for(i = 0 ; i < ASCII_SIZE ; ++i)
	{
		states_lut[WAIT_FOR_OPERATOR][i] = &InvalidOperatorHandler;
	}
	
	InitDigits(states_lut[WAIT_FOR_NUM]);
	
	states_lut[WAIT_FOR_OPERATOR][SUCCESS] = &SuccessHandler;
	
	states_lut[WAIT_FOR_OPERATOR][PLUS] = &BaseOperationsHandler;
	states_lut[WAIT_FOR_OPERATOR][MINUS] = &BaseOperationsHandler;
	states_lut[WAIT_FOR_OPERATOR][MULT] = &BaseOperationsHandler;
	states_lut[WAIT_FOR_OPERATOR][DIVIDE] = &BaseOperationsHandler;
	states_lut[WAIT_FOR_OPERATOR][POW] = &BaseOperationsHandler;
	states_lut[WAIT_FOR_OPERATOR][SPACE] = &OperatorSpaceHandler;
	
	states_lut[WAIT_FOR_NUM][OPEN_ROUND_PAREN] = &OpenParenHandler;
	states_lut[WAIT_FOR_NUM][OPEN_SQUARE_PAREN] = &OpenParenHandler;
	states_lut[WAIT_FOR_NUM][OPEN_CURLY_PAREN] = &OpenParenHandler;
	states_lut[WAIT_FOR_NUM][PLUS] = &InsertUnaryOperatorHandler;
	states_lut[WAIT_FOR_NUM][MINUS] = &InsertUnaryOperatorHandler;
	states_lut[WAIT_FOR_NUM][SPACE] = &OperandSpaceHandler;
	
	states_lut[WAIT_FOR_OPERATOR][CLOSE_ROUND_PAREN] = &CloseParenHandler;
	states_lut[WAIT_FOR_OPERATOR][CLOSE_SQUARE_PAREN] = &CloseParenHandler;
	states_lut[WAIT_FOR_OPERATOR][CLOSE_CURLY_PAREN] = &CloseParenHandler;
	
}


/*--------------------- calculate help functions ------------------------*/

static status_ty AddEvaluate(stack_ty *stack)
{
	double temp_result = 0;
	double operand1 = 0;
	double operand2 = 0;
	
	assert(NULL != stack);
	
	operand1 = *(double*)StackPeek(stack);
	StackPop(stack);
	operand2 = *(double*)StackPeek(stack);
	StackPop(stack);
	
	temp_result = operand2 + operand1;

	StackPush(stack, &temp_result);
	
	return SUCCESS;
}

static status_ty SubEvaluate(stack_ty *stack)
{
	double temp_result = 0;
	double operand1 = 0;
	double operand2 = 0;
	
	assert(NULL != stack);
	
	operand1 = *(double*)StackPeek(stack);
	StackPop(stack);
	operand2 = *(double*)StackPeek(stack);
	StackPop(stack);
	
	temp_result = operand2 - operand1;

	StackPush(stack, &temp_result);
	
	return SUCCESS;
}

static status_ty MultEvaluate(stack_ty *stack)
{
	double temp_result = 0;
	double operand1 = 0;
	double operand2 = 0;
	
	assert(NULL != stack);
	
	operand1 = *(double*)StackPeek(stack);
	StackPop(stack);
	operand2 = *(double*)StackPeek(stack);
	StackPop(stack);
	
	temp_result = operand2 * operand1;

	StackPush(stack, &temp_result);
	
	return SUCCESS;
}

static status_ty DivideEvaluate(stack_ty *stack)
{
	double temp_result = 0;
	double operand1 = 0;
	double operand2 = 0;
	
	assert(NULL != stack);
	
	operand1 = *(double*)StackPeek(stack);
	StackPop(stack);
	operand2 = *(double*)StackPeek(stack);
	StackPop(stack);
	
	if(0 == operand1) /* divide by zero */
	{
		return INVALID_EXPRESSION;
	}
	temp_result = operand2 / operand1;

	StackPush(stack, &temp_result);
	
	return SUCCESS;
}


static status_ty PowEvaluate(stack_ty *stack)
{
	double temp_result = 0;
	double operand1 = 0;
	double operand2 = 0;
	
	assert(NULL != stack);
	
	operand1 = *(double*)StackPeek(stack);
	StackPop(stack);
	operand2 = *(double*)StackPeek(stack);
	StackPop(stack);
	
	temp_result = pow(operand2, operand1);

	StackPush(stack, &temp_result);
	
	return SUCCESS;
}


static status_ty InvalidOperate(stack_ty *stack)
{
	assert(NULL != stack);
	return INVALID_EXPRESSION;
}


static status_ty CalculateOneOperation(calculator_ty *calc)
{
	char *operator = NULL;
	status_ty status = SUCCESS;
	assert(NULL != calc);
	
	operator = StackPeek(calc->operators_stack);
	status = operate_funcs_lut[(int)*operator](calc->operands_stack);
	StackPop(calc->operators_stack);
	
	return status;
}


static status_ty CalculateAllOperations(calculator_ty *calc)
{
	status_ty status = SUCCESS;
	assert(NULL != calc);
	
	while(!StackIsEmpty(calc->operators_stack) && SUCCESS == status)
	{
		status = CalculateOneOperation(calc);
	}

	return status;	
}















































