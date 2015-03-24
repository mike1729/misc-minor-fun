#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expr.h"

enum expr_kind {
    /* constants */
    EXPR_CONST,
    /* variables */
    EXPR_VAR,
    /* unary */
    EXPR_PLUS,
    EXPR_MINUS,
    EXPR_SQRT,
    EXPR_SIN,
    EXPR_COS,
    /* binary */
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_MIN,
    EXPR_MAX
};

struct expr {
    enum expr_kind      kind;
    union {
        float           constant;
        unsigned char   variable;
        struct {
            struct expr *   inner;
        }               unary;
        struct {
            struct expr *   left;
            struct expr *   right;
        }               binary;
    };
};

static char const * expr_parse_whole(struct expr * expr, char const * text);

static char const * expr_parse_atom(struct expr * expr, char const * text) {
    int shift = 0;
    sscanf(text, " %f%n", &expr->constant, &shift);
    if (shift) {
        expr->kind = EXPR_CONST;
        return text+shift;
    }
    sscanf(text, " x%hhu%n", &expr->variable, &shift);
    if (shift) {
        expr->kind = EXPR_VAR;
        return text+shift;
    }
    sscanf(text, " +%n", &shift);
    if (shift) {
        expr->kind = EXPR_PLUS;
        expr->unary.inner = (struct expr *) malloc(sizeof(struct expr));
        return expr_parse_atom(expr->unary.inner, text+shift);
    }
    sscanf(text, " -%n", &shift);
    if (shift) {
        expr->kind = EXPR_MINUS;
        expr->unary.inner = (struct expr *) malloc(sizeof(struct expr));
        return expr_parse_atom(expr->unary.inner, text+shift);
    }
    sscanf(text, " sqrt%n", &shift);
    if (shift) {
        expr->kind = EXPR_SQRT;
        expr->unary.inner = (struct expr *) malloc(sizeof(struct expr));
        return expr_parse_atom(expr->unary.inner, text+shift);
    }
    sscanf(text, " sin%n", &shift);
    if (shift) {
        expr->kind = EXPR_SIN;
        expr->unary.inner = (struct expr *) malloc(sizeof(struct expr));
        return expr_parse_atom(expr->unary.inner, text+shift);
    }
    sscanf(text, " cos%n", &shift);
    if (shift) {
        expr->kind = EXPR_COS;
        expr->unary.inner = (struct expr *) malloc(sizeof(struct expr));
        return expr_parse_atom(expr->unary.inner, text+shift);
    }
    sscanf(text, " min (%n", &shift);
    if (shift) {
        expr->kind = EXPR_MIN;
        expr->binary.left = (struct expr *) malloc(sizeof(struct expr));
        text = expr_parse_whole(expr->binary.left, text+shift);
        shift = 0;
        sscanf(text, " ,%n", &shift);
        if (!shift)
            return 0;
        expr->binary.right = (struct expr *) malloc(sizeof(struct expr));
        text = expr_parse_whole(expr->binary.right, text+shift);
        shift = 0;
        sscanf(text, " )%n", &shift);
        if (!shift)
            return 0;
        return text+shift;
    }
    sscanf(text, " max (%n", &shift);
    if (shift) {
        expr->kind = EXPR_MAX;
        expr->binary.left = (struct expr *) malloc(sizeof(struct expr));
        text = expr_parse_whole(expr->binary.left, text+shift);
        shift = 0;
        sscanf(text, " ,%n", &shift);
        if (!shift)
            return 0;
        expr->binary.right = (struct expr *) malloc(sizeof(struct expr));
        text = expr_parse_whole(expr->binary.right, text+shift);
        shift = 0;
        sscanf(text, " )%n", &shift);
        if (!shift)
            return 0;
        return text+shift;
    }
    sscanf(text, " (%n", &shift);
    if (shift) {
        text = expr_parse_whole(expr, text+shift);
        shift = 0;
        sscanf(text, " )%n", &shift);
        if (!shift)
            return 0;
        return text+shift;
    }
    return 0;
}

static char const * expr_parse_term(struct expr * expr, char const * text) {
    text = expr_parse_atom(expr, text);
    for (;;) {
        int shift = 0;
        sscanf(text, " *%n", &shift);
        if (shift) {
            struct expr * left = (struct expr *) malloc(sizeof(struct expr));
            *left = *expr;
            expr->kind = EXPR_MUL;
            expr->binary.left = left;
            expr->binary.right = (struct expr *) malloc(sizeof(struct expr));
            text = expr_parse_atom(expr->binary.right, text+shift);
            continue;
        }
        sscanf(text, " /%n", &shift);
        if (shift) {
            struct expr * left = (struct expr *) malloc(sizeof(struct expr));
            *left = *expr;
            expr->kind = EXPR_DIV;
            expr->binary.left = left;
            expr->binary.right = (struct expr *) malloc(sizeof(struct expr));
            text = expr_parse_atom(expr->binary.right, text+shift);
            continue;
        }
        break;
    }
    return text;
}

static char const * expr_parse_whole(struct expr * expr, char const * text) {
    text = expr_parse_term(expr, text);
    for (;;) {
        int shift = 0;
        sscanf(text, " +%n", &shift);
        if (shift) {
            struct expr * left = (struct expr *) malloc(sizeof(struct expr));
            *left = *expr;
            expr->kind = EXPR_ADD;
            expr->binary.left = left;
            expr->binary.right = (struct expr *) malloc(sizeof(struct expr));
            text = expr_parse_term(expr->binary.right, text+shift);
            continue;
        }
        sscanf(text, " -%n", &shift);
        if (shift) {
            struct expr * left = (struct expr *) malloc(sizeof(struct expr));
            *left = *expr;
            expr->kind = EXPR_SUB;
            expr->binary.left = left;
            expr->binary.right = (struct expr *) malloc(sizeof(struct expr));
            text = expr_parse_term(expr->binary.right, text+shift);
            continue;
        }
        break;
    }
    return text;
}

struct expr * expr_create(char const * text) {
    struct expr * expr = (struct expr *) malloc(sizeof(struct expr));
    expr_parse_whole(expr, text);
    return expr;
}

void expr_destroy(struct expr * expr) {
    switch (expr->kind) {
        case EXPR_PLUS:
        case EXPR_MINUS:
        case EXPR_SQRT:
        case EXPR_SIN:
        case EXPR_COS:
            expr_destroy(expr->unary.inner);
            break;
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_MIN:
        case EXPR_MAX:
            expr_destroy(expr->binary.left);
            expr_destroy(expr->binary.right);
            break;
    }
    free(expr);
}



/*static*/ void expr_eval_one(float * result, struct expr * expr, float const * vars) {
    float inner, left, right;
    switch (expr->kind) {
        case EXPR_CONST:
            *result = expr->constant;
            break;
        case EXPR_VAR:
            *result = vars[expr->variable];
            break;
        case EXPR_PLUS:
            expr_eval_one(&inner, expr->unary.inner, vars);
            *result = inner;
            break;
        case EXPR_MINUS:
            expr_eval_one(&inner, expr->unary.inner, vars);
            *result = -inner;
            break;
        case EXPR_SQRT:
            expr_eval_one(&inner, expr->unary.inner, vars);
            *result = sqrtf(inner);
            break;
        case EXPR_SIN:
            expr_eval_one(&inner, expr->unary.inner, vars);
            *result = sinf(inner);
            break;
        case EXPR_COS:
            expr_eval_one(&inner, expr->unary.inner, vars);
            *result = cosf(inner);
            break;
        case EXPR_ADD:
            expr_eval_one(&left, expr->binary.left, vars);
            expr_eval_one(&right, expr->binary.right, vars);
            *result = left + right;
            break;
        case EXPR_SUB:
            expr_eval_one(&left, expr->binary.left, vars);
            expr_eval_one(&right, expr->binary.right, vars);
            *result = left - right;
            break;
        case EXPR_MUL:
            expr_eval_one(&left, expr->binary.left, vars);
            expr_eval_one(&right, expr->binary.right, vars);
            *result = left * right;
            break;
        case EXPR_DIV:
            expr_eval_one(&left, expr->binary.left, vars);
            expr_eval_one(&right, expr->binary.right, vars);
            *result = left / right;
            break;
        case EXPR_MIN:
            expr_eval_one(&left, expr->binary.left, vars);
            expr_eval_one(&right, expr->binary.right, vars);
            *result = (left < right) ? left : right;
            break;
        case EXPR_MAX:
            expr_eval_one(&left, expr->binary.left, vars);
            expr_eval_one(&right, expr->binary.right, vars);
            *result = (left > right) ? left : right;
            break;
    }
}

float * expr_eval(struct expr * expr, unsigned long rows, unsigned long cols, float const * data) {
    float * results = (float *) malloc(rows * sizeof(float));
    for (unsigned long i = 0; i < rows; ++i)
        expr_eval_one(results + i, expr, data + (i*cols));
    return results;
}
