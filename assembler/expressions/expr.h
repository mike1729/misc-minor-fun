#ifndef EXPR_H_
#define EXPR_H_

/* An opaque representation of arithmetic expressions. */
struct expr;

/* Parses an expression given in infix notation. The primitives are:    *
 *   floating-point constants (as parseable by scanf("%f"))             *
 *   input variables x0, x1, ..., x15                                   *
 *   unary operators: + and -                                           *
 *   binary operators: +, -, * and /                                    *
 *   unary functions: sqrt(), sin(), cos()                              *
 *   binary functions: min(), max()                                     *
 * Returns an internal representation of the expression. The behaviour  *
 * is _undefined_ when the expression is syntactically incorrect.       */
struct expr * expr_create(char const * text);

/* Evaluates an expression on multiple data sets.                       *
 *   expr -- the internal representation of the expression to evaluate  *
 *   rows -- the number of data sets to process                         *
 *   cols -- the number of inputs in a single data set                  *
 *   data -- an array with rows*cols float entries, 16B-aligned         *
 * Returns a malloc()-allocated array of at least rows floats --        *
 * the results of evaluating expr on consecutive data sets.             *
 * The behaviour is _undefined_ when expr uses variables with indices   *
 * greater or equal to cols.                                            */
float * expr_eval(struct expr * expr,
                  unsigned long rows,
                  unsigned long cols,
                  float const * data);

/* Releases resources (memory, etc.) used by the representation expr.   */
void expr_destroy(struct expr * expr);

#endif // EXPR_H_
