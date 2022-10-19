/*****************************************************   
 * Author : Avia Avikasis                            *
 * Reviewer: Maayan                                  *
 * 23/07/2022                                        *
 * Description : api of calculator                   *
 *                                                   *
 *****************************************************/
#ifndef __ILRD_OL127_128_CALCULATOR_H__
#define __ILRD_OL127_128_CALCULATOR_H__

typedef enum {
    SUCCESS                  = 0, 
    INVALID_EXPRESSION       = 1,
    INSUFFICIENT_MEMORY_FAIL = 2
}status_ty;

/*
DESCRIPTION : calculate mathematical expression , include 
operations : add, subtract, mult, divide and power.
PARAMETERS : expression(char*) , pointer to status_ty
vatiable, that need to declare by user
RETURN : the result of the calculate(double).
in additional , the status of calculate terminate
will update in the status variable.
ERRORS : in each case of invalid expreesion, the status
will update to INVALID_EXPRESSION. in case of allocate
error, it will update to INSUFFICIENT_MEMORY_FAIL.
in all error cases, the function return will be 0. 
expression with spaces is valid.
LIMITS : the function can claculate expressions
that contain till 100 operands and 100 operators.
*/
double Calculate(const char *expression, status_ty *o_status);

#endif /* __ILRD_OL127_128_CALCULATOR_H__ */
