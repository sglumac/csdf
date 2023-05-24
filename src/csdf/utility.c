/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "utility.h"

unsigned int gcd(unsigned int a, unsigned int b)
{
    int temp;
    while (b != 0)
    {
        temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

unsigned int lcm(unsigned int a, unsigned int b)
{
    unsigned int result = (a * b) / gcd(a, b);
    return result;
}

void set_rational_value(Rational *rational, unsigned int num, unsigned int den)
{
    rational->num = num;
    rational->den = den;
}

void reduce_rational_value(Rational *rational)
{
    unsigned int div = gcd(rational->num, rational->den);
    rational->num /= div;
    rational->den /= div;
}

void set_reduce_rational_value(Rational *rational, unsigned int num, unsigned int den)
{
    set_rational_value(rational, num, den);
    reduce_rational_value(rational);
}

bool is_rational_zero(const Rational *rational)
{
    return rational->num == 0;
}

unsigned int rational_eq(const Rational *rational, unsigned int num, unsigned int den)
{
    return rational->num * den == rational->den * num;
}
