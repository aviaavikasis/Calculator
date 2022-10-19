#include <assert.h> /* assert */
#include <stdio.h> /* printf */
#include "calc.h"

void CalcPlusTest(void);
void CalcMinusTest(void);
void CalcMultTest(void);
void CalcDivideTest(void);
void CalcPowTest(void);
void CalcParenTest(void);
void GeneralTest(void);
void InvalidParenTest(void);

int main(void)
{
	CalcPlusTest();
	CalcMinusTest();
	CalcMultTest();
	CalcDivideTest();
	CalcPowTest();
	CalcParenTest();
	GeneralTest();
	InvalidParenTest();
	printf("\n\n->->->->->->->->-> success!!! <-<-<-<-<-<-<-<-<-<\n\n\n");
	return 0;	
}


void CalcPlusTest(void)
{
	char *expression = "45+25+10+10";
	status_ty status = SUCCESS;
	assert(90.00 == Calculate(expression , &status));
	assert(SUCCESS == status);	
}

void CalcMinusTest(void)
{
	char *expression = "45.4-25.5+10-10";
	status_ty status = SUCCESS;
	assert(19.90 == Calculate(expression , &status));
	assert(SUCCESS == status);	
}

void CalcMultTest(void)
{
	char *expression = "3* 4*2";
	status_ty status = SUCCESS;
	assert(24 == Calculate(expression , &status));
	
	expression = "3 * 2 -    5";
	assert(1 == Calculate(expression , &status));
	assert(SUCCESS == status);
	
	expression = "((8";
	Calculate(expression , &status);
}


void CalcDivideTest(void)
{
	char *expression = "12/4/2/2";
	status_ty status = SUCCESS;
	assert(0.75 == Calculate(expression , &status));

	expression = "6/ 2-5";
	assert(-2 == Calculate(expression , &status));
	assert(SUCCESS == status);	
}


void CalcPowTest(void)
{
	char *expression = "2^3^2*3";
	status_ty status = SUCCESS;
	assert(1536 == Calculate(expression , &status));
	assert(SUCCESS == status);	
}

void CalcParenTest(void)
{

	char *expression = "(3-2)+(4*3)";
	status_ty status = SUCCESS;

	assert(13 == Calculate(expression , &status));
	assert(SUCCESS == status);	
	
	expression = "2*3+(5+7-5)";
	assert(13 == Calculate(expression , &status));
	assert(SUCCESS == status);	
	
	expression = "{2*3} +[5 +7-5]";
	assert(13 == Calculate(expression , &status));
	assert(SUCCESS == status);	
}

void GeneralTest(void)
{
	char *expression = "2*3+(2^2)^(1^3)";
	status_ty status = SUCCESS;
	assert(10 == Calculate(expression , &status));
	assert(SUCCESS == status);
	
	expression = "5+2-(3*5/2-4)-{(5+5+5+5)-3}";  
	assert(-13.5 == Calculate(expression , &status));
	assert(SUCCESS == status);	
	
	expression = "(5+((4*2)*(2+3)))";
	assert(45 == Calculate(expression , &status));
	assert(SUCCESS == status);	
}

void InvalidParenTest(void)
{
	char *expression = "2*3+2^2)^(1^3)";
	status_ty status = SUCCESS;
	Calculate(expression , &status);
	assert(INVALID_EXPRESSION == status);
	
	expression = "(5+2))";
	Calculate(expression , &status);
	assert(INVALID_EXPRESSION == status);	
	
	expression = "5+2-(3*5/2-4)-{(5+5+5+5)-3]";
	Calculate(expression , &status);
	assert(INVALID_EXPRESSION == status);
}



























