/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_UTILITY_H
#define CSDF_UTILITY_H

#include <stdbool.h>

unsigned int gcd(unsigned int a, unsigned int b);

unsigned int lcm(unsigned int a, unsigned int b);

typedef struct Rational
{
    unsigned int num;
    unsigned int den;
} Rational;

void set_rational_value(Rational *rational, unsigned int num, unsigned int den);

void reduce_rational_value(Rational *rational);

void set_reduce_rational_value(Rational *rational, unsigned int num, unsigned int den);

bool is_rational_zero(const Rational *rational);

unsigned int rational_eq(const Rational *rational, unsigned int num, unsigned int den);

#endif // CSDF_UTILITY_H
