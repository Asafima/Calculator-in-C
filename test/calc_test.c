#include <stdio.h> /* printf */
#include "calc.h" /* Calculator */

#define NUM_OF_TESTS (10)
#define TRUE (1)
#define FALSE (0)

void TestCalculator(void);

int main()

{
	TestCalculator();
	return 0;
}

void TestCalculator(void)
{ 
    double res = 0;
    size_t i = 0;
    int passed_test = TRUE;
    char *str[NUM_OF_TESTS] =
    {
        "  2 +   8", "9+8*3+-2^5", "8+8*3-2^", "2/0", "8++8*((3-2)*5)",
        "3-2)*5", "( 3- 2)*5+ 5*(4+4+4" , "((7*(2+5)+3))", ")  ", " -5 * (5 + 3) * 2"
    };
    double result[NUM_OF_TESTS] = {10, 1, 0, 0, 48, 0, 0, 52, 0, -80 };
    
    calculator_status_t ret_status[NUM_OF_TESTS] = 
    { SUCCESS, SUCCESS, SYNTAX_ERROR, MATH_ERROR, SUCCESS,
      SYNTAX_ERROR, SYNTAX_ERROR, SUCCESS, SYNTAX_ERROR, SUCCESS };

    calc_t *calculator = CalcCreate();
    for (i = 0; i < 10; ++i)
    {
        if (Calculate(calculator, str[i], &res) != ret_status[i])
        {
            passed_test = FALSE;
            break;
        }
        if(result[i] != res)
        {
            passed_test = FALSE;
            break;
        }
    }

    if (passed_test)
    {
        printf ("Calculator Working!                                 V\n");
    }
    else
    {
        printf ("Calculator Not Working!                             X\n");
    }
    CalcDestroy(calculator);
       
    
    
}
