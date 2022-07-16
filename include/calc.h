#ifndef __CALC_H_
#define __CALC_H_

typedef enum calculator_status
{
    SUCCESS,
    MATH_ERROR,
    SYNTAX_ERROR
} calculator_status_t;

typedef struct calc calc_t;


/* DESCRIPTION:
 * Function creates a calculator, with max storage space 
 * of 30 operators and 30 double variables
 *
 * PARAMS:
 * capacity     - the max capacity in elements of the stack to be created
 * element_size - size of each element to be stored in the array
 *         
 * RETURN:
 * Returns a pointer to the calculator, or NULL on error
 */

calc_t *CalcCreate();


/* DESCRIPTION:
 * Function destroys the calculator
 *
 * PARAMS:
 * calc - pointer to the calculator to be destroyed
 *         
 * RETURN:
 * void
 */
void CalcDestroy(calc_t *calc);


/* DESCRIPTION:
 * Function performs calculations based on expression received.
 *
 * PARAMS:
 * calc -       pointer to the calculator that determines math rules.
 * expression - pointer to string to perform calculations on.
 * res -        result of calculations.
 *         
 * RETURN:
 * status, defined by enum calculator_status.
 */

calculator_status_t Calculate(calc_t *calc, const char *expression, double *res);

#endif /*__CALC_H_*/