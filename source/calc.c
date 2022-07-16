/*========================== LIBRARIES && MACROS =============================*/
#include <stdlib.h> /* malloc, free */
#include <assert.h> /* assert */
#include <ctype.h> /* is digit */
#include <math.h> /* pow */

#include "stack.h" /* Stacks */
#include "calc.h" /* Declerations */

#define ZERO_DIVISION (0xDEADBEEF)
#define STACK_CAPACITY (30)
#define NUM_OF_OPERATORS (128)
#define ODD_NUM (1)
#define ASCII_CHAR_TO_NUM (48)
#define NUM_OF_DIGITS (10)


typedef enum state
{
    WAIT_FOR_NUM_STATE,
    WAIT_FOR_OP_STATE,
    ERROR_STATE,
    FINAL_STATE
}state_t;

typedef enum event
{
    RECEIVE_PLUS_EVENT = '+',
    RECEIVE_MINUS_EVENT = '-',
    RECEIVE_MULT_EVENT = '*',
    RECEIVE_DIV_EVENT = '/',
    RECEIVE_POW_EVENT = '^',
    RECEIVE_CLOSE_PAR_EVENT = ')' ,
    RECEIVE_OPEN_PAR_EVENT = '(',
    RECEIVE_WHITE_SPACE_EVENT = ' ',
    END_OF_STRING_EVENT = '\0'
} event_t;

typedef enum math_priority
{
    PLUS = 1,
    MINUS = 1,
    MUL = 2, 
    DIV = 2,
    POW = 3
}math_priority_t; 


typedef state_t(*event_handler_t[FINAL_STATE][NUM_OF_OPERATORS])
                (calc_t *calc, char **expression, double *res);
typedef double(*op_handler_t)(double, double);


typedef struct operator_priority
{
    op_handler_t operator_table;
    math_priority_t op_priority;
}operator_priority_t;

struct calc
{
    stack_t *num_stack;
    stack_t *op_stack;
    state_t curr_state;
    event_handler_t state_machine;
    operator_priority_t priority_table[NUM_OF_OPERATORS];
};

/*=============================== LUT FUNCS ==================================*/

static void InitLut(event_handler_t state_machine);
static void InitOpLut(operator_priority_t *priority_table);

/*============================== HELPER FUNCS ================================*/

static calculator_status_t GetStatusFromCurrState(calc_t *calc, double *res);
static double Calc2Numbers(calc_t *calc);
static void EmptyCalcStack(calc_t *calc, double *res);

/*============================= HANDLER FUNCS ================================*/

static state_t NumHandler(calc_t *calc, char **expression, double *res);
static state_t OpHandler(calc_t *calc, char **expression, double *res);
static state_t ErrorHandler(calc_t *calc, char **expression, double *res);
static state_t CloseParHandler(calc_t *calc, char **expression, double *res);
static state_t OpenParHandler(calc_t *calc, char **expression, double *res);
static state_t EndOfStringHandler(calc_t *calc, char **expression, double *res);
static state_t WhiteSpaceHandler(calc_t *calc, char **expression, double *res);

/*=============================== MATH FUNCS  ================================*/

static double Multiplication(double num1, double num2);
static double Division(double num1, double num2);
static double Sum(double num1, double num2);
static double Subtract(double num1, double num2);
static double Power(double base, double exp);

/*============================ HEADER FUNCS  =================================*/

calc_t *CalcCreate()
{
    calc_t *calc = (calc_t *)malloc(sizeof(calc_t));
    if (NULL == calc)
    {
        return (NULL);
    }
    calc->num_stack = StackCreate(STACK_CAPACITY, sizeof(double));
    if(NULL == calc->num_stack)
    {
        free(calc);
        return (NULL);
    }
    calc->op_stack = StackCreate(STACK_CAPACITY, sizeof(char));
    if (NULL == calc->op_stack)
    {
        free(calc->num_stack);
        free(calc);
        return (NULL);
    }

    calc->curr_state = WAIT_FOR_NUM_STATE;
    InitLut(calc->state_machine);
    InitOpLut(calc->priority_table);
    
    return (calc);
}

void CalcDestroy(calc_t *calc)
{
    assert (NULL != calc);
    StackDestroy(calc->num_stack);
    StackDestroy(calc->op_stack);
    free(calc);
}

calculator_status_t Calculate(calc_t *calc, const char *expression, double *res)
{
    assert(NULL != calc);
    assert(NULL != expression);
    assert(NULL != res);
    
    calc->curr_state = WAIT_FOR_NUM_STATE;

    while (ERROR_STATE != calc->curr_state && FINAL_STATE != calc->curr_state)
    { 
        calc->curr_state = (*calc->state_machine[calc->curr_state][(int)*expression])
                            (calc, (char **)&expression, res);
    }

    EmptyCalcStack(calc, res);

    return (GetStatusFromCurrState(calc, res));
}


/*============================= HANDLER FUNCS ================================*/

static state_t NumHandler(calc_t *calc, char **expression, double *res)
{
    double num = 0;

    assert (NULL != expression);
    assert (NULL != calc);
    (void)res;

    num = strtod(*expression, expression);

    StackPush(calc->num_stack, (void *)&num);
    return (WAIT_FOR_OP_STATE);

}

static state_t OpHandler(calc_t *calc, char **expression, double *res)
{
    double ans = 0;
    (void)res;

    while ((!StackIsEmpty(calc->op_stack)) && 
            (calc->priority_table[(int)(**expression)].op_priority <= 
            calc->priority_table[(int)(*(char *)StackPeek(calc->op_stack))].op_priority))
    {
        ans = Calc2Numbers(calc);
        
        if(ZERO_DIVISION == ans)
        {
            return (ERROR_STATE);
        }
    }

    StackPush(calc->op_stack, *expression);
    ++(*expression);
    return (WAIT_FOR_NUM_STATE);
}

static state_t ErrorHandler(calc_t *calc, char **expression, double *res)
{
    (void)calc;
    (void)expression;
    (void)res;
    return (ERROR_STATE);   
}

static state_t CloseParHandler(calc_t *calc, char **expression, double *res)
{
    double ans = 0;

    (void)res;

    while ((!StackIsEmpty(calc->op_stack)) && 
            *(char *)StackPeek(calc->op_stack) != RECEIVE_OPEN_PAR_EVENT)
    {
        ans = Calc2Numbers(calc);
        if(ans == ZERO_DIVISION)
        {
            return (ERROR_STATE);
        }
    }
    /* got ')' without '(' */
    if (StackIsEmpty(calc->op_stack))
    {
        return (ERROR_STATE);
    }

    StackPop(calc->op_stack);
    ++(*expression);
    return (WAIT_FOR_OP_STATE);
}

static state_t OpenParHandler(calc_t *calc, char **expression, double *res)
{
    assert (NULL != expression);
    assert (NULL != calc);
    (void)res;

    StackPush(calc->op_stack, *expression);
    ++(*expression);
    return (WAIT_FOR_NUM_STATE);
}

static state_t EndOfStringHandler(calc_t *calc, char **expression, double *res)
{
    double ans = 0;
    (void)expression;
    (void)res;

    if (ODD_NUM != (StackGetSize(calc->num_stack) - StackGetSize(calc->op_stack)))
    {
        return (ERROR_STATE);   
    }
    while (1 < StackGetSize(calc->num_stack))
    {
        ans = Calc2Numbers(calc);
        if(ans == ZERO_DIVISION)
        {
            return (ERROR_STATE);
        }
    }
    
    return (FINAL_STATE);
}

static state_t WhiteSpaceHandler(calc_t *calc, char **expression, double *res)
{
    (void)res;
    
    while (' ' == **expression)
    {
        ++(*expression);
    }

    return (calc->curr_state);
}

/*=============================== MATH FUNCS  ================================*/

static double Multiplication(double num1, double num2)
{
    return (num1 * num2);
}

static double Division(double num1, double num2)
{
    if (0 == num2)
    {
        return (ZERO_DIVISION);
    }
    return (num1 / num2);
}

static double Sum(double num1, double num2)
{
    return (num1 + num2);
}

static double Subtract(double num1, double num2)
{
    return (num1 - num2);
}

static double Power(double base, double exp)
{
    return(pow(base, exp));
}

/*=============================== LUT FUNCS ==================================*/

static void InitLut(event_handler_t state_machine)
{
    size_t i = 0, j = 0;

    for (i = 0; i < FINAL_STATE; ++i)
    {
        for (j = 0; j < NUM_OF_OPERATORS; ++j)
        {
            state_machine[i][j] = ErrorHandler;
        }
    }
    for (i = 0; i < NUM_OF_DIGITS; ++i)
    {
        state_machine[WAIT_FOR_NUM_STATE][ASCII_CHAR_TO_NUM + i] = NumHandler;
    }

    state_machine[WAIT_FOR_NUM_STATE][RECEIVE_PLUS_EVENT] = NumHandler;
    state_machine[WAIT_FOR_NUM_STATE][RECEIVE_MINUS_EVENT] = NumHandler;
    state_machine[WAIT_FOR_NUM_STATE][RECEIVE_OPEN_PAR_EVENT] = OpenParHandler;
    state_machine[WAIT_FOR_NUM_STATE][RECEIVE_WHITE_SPACE_EVENT] = WhiteSpaceHandler;
    state_machine[WAIT_FOR_NUM_STATE][END_OF_STRING_EVENT] = EndOfStringHandler;
   
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_PLUS_EVENT] = OpHandler;
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_MINUS_EVENT] = OpHandler;
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_MULT_EVENT] = OpHandler;
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_DIV_EVENT] = OpHandler;
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_POW_EVENT] = OpHandler;
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_CLOSE_PAR_EVENT] = CloseParHandler;
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_OPEN_PAR_EVENT] = OpenParHandler;
    state_machine[WAIT_FOR_OP_STATE][RECEIVE_WHITE_SPACE_EVENT] = WhiteSpaceHandler;
    state_machine[WAIT_FOR_OP_STATE][END_OF_STRING_EVENT] = EndOfStringHandler;
}

static void InitOpLut(operator_priority_t *priority_table)
{ 
    size_t i = 0;
    for (i = 0; i < NUM_OF_OPERATORS; ++i)
    {
        priority_table[i].operator_table = NULL;
        priority_table[i].op_priority = 0;
    }
    priority_table['+'].operator_table = Sum;
    priority_table['+'].op_priority = PLUS;

    priority_table['-'].operator_table = Subtract;
    priority_table['-'].op_priority = MINUS;

    priority_table['*'].operator_table = Multiplication;
    priority_table['*'].op_priority = MUL;

    priority_table['/'].operator_table = Division;
    priority_table['/'].op_priority = DIV;
    
    priority_table['^'].operator_table = Power;
    priority_table['^'].op_priority = POW;
    
}

/*============================== HELPER FUNCS ================================*/

static calculator_status_t GetStatusFromCurrState(calc_t *calc, double *res)
{
    if (ERROR_STATE == calc->curr_state)
    {
        if (ZERO_DIVISION == *res)
        {
            *res = 0;
            return (MATH_ERROR);   
        }
        *res = 0;
        return (SYNTAX_ERROR);
    }
    else
    {
        return (SUCCESS);
    }
}

static double Calc2Numbers(calc_t *calc)
{
    double num1 = 0, num2 = 0, ans = 0;
    unsigned char op = ' ';

    op = *(char *)StackPop(calc->op_stack);
    num2 = *(double *)StackPop(calc->num_stack);
    num1 = *(double *)StackPop(calc->num_stack);

    ans = calc->priority_table[op].operator_table(num1, num2);
    
    StackPush(calc->num_stack, (void *)&ans);

    return (ans);
}


static void EmptyCalcStack(calc_t *calc, double *res)
{
    if (!StackIsEmpty(calc->num_stack))
    {
        *res = *(double *)StackPop(calc->num_stack);
    }
    
    while(!StackIsEmpty(calc->num_stack))
    {
        StackPop(calc->num_stack);
    }
    while(!StackIsEmpty(calc->op_stack))
    {
        StackPop(calc->op_stack);
    }
}