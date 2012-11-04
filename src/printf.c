/*
 * Copyright (c) 2004,2012 Kustaa Nyholm / SpareTimeLabs
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this 
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *  
 * Neither the name of the Kustaa Nyholm or SpareTimeLabs nor the names of its 
 * contributors may be used to endorse or promote products derived from this software 
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "board.h"
#include "printf.h"

#define PRINTF_LONG_SUPPORT
#define PRINTF_FLOAT_SUPPORT

#define PRINTF_FLOAT_DIGITS 7

typedef void (*putcf) (void *, char);
static putcf stdout_putf;
static void *stdout_putp;

#ifdef PRINTF_LONG_SUPPORT

static void uli2a(unsigned long int num, unsigned int base, int uc, char *bf)
{
    int n = 0;
    unsigned int d = 1;
    while (num / d >= base)
        d *= base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}

static void li2a(long num, char *bf)
{
    if (num < 0) {
        num = -num;
        *bf++ = '-';
    }
    uli2a(num, 10, 0, bf);
}

#endif

//Return pointing at the 0 ending the string
static char* ui2a(unsigned int num, unsigned int base, int uc, char *bf)
{
    int n = 0;
    unsigned int d = 1;
    while (num / d >= base)
        d *= base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
    return bf;
}

static char* i2a(int num, char *bf)
{
    if (num < 0) {
        num = -num;
        *bf++ = '-';
    }
    return ui2a(num, 10, 0, bf);
}

#ifdef 	PRINTF_FLOAT_SUPPORT
//Convert a float to a string without using any floating point operations
static char* f2a( float input, unsigned int digits, char *bf ) {
	//temp value just to make me happy
	long val = *( long*)&input;
	//Extract mantissa the 1.xxxxx value
	unsigned long mantissa = (val & 0xffffff) | 0x800000;
	//Extract the base 2 exponent
	int exp2 = ( ( val >> 23) & 0xff ) - 127;
	//Check exp2 for value we can't handle to indicate errors
	if ( exp2 < -23 || exp2 >= 31 ) {
		//yes a # indicates a float error!
		bf[0] = '#';
		bf[1] = 0;
		return bf + 1;
	}
	//Create the integer and fractional part of the mantissa
	unsigned long intPart, fracPart;

	if (exp2 >= 23) {
		intPart = mantissa << (exp2 - 23);
		fracPart = 0;
	} else if (exp2 >= 0) {
	    intPart = mantissa >> (23 - exp2);
	    fracPart = (mantissa << (exp2 + 1)) & 0xFFFFFF;
	} else {
		 /* if (exp2 < 0) */
		intPart = 0;
		fracPart = (mantissa & 0xFFFFFF) >> -(exp2 + 1);
	}
	//negative start with a zero
	if( val < 0 )
		*bf++ = '-';
	//Output the integer part
	bf = ui2a( intPart, 10, 0, bf );
	//Nothing to do behind the decimal point
	if ( !digits )
		return bf;
	//output the fractional part
	*bf++ = '.';
	//check for max digit count
	if ( digits > PRINTF_FLOAT_DIGITS)
		digits = PRINTF_FLOAT_DIGITS;
	for( ; digits >0; digits-- ) {
		//Fraction makes up the lower 24 bits
		//Multiply it up to create the digits
		fracPart *= 10;
		*bf++ = ( fracPart >> 24 ) + '0';
		fracPart &= 0xFFFFFF;
	}
	//Finish string
	*bf = 0;
	return bf;
}

#endif


static int a2d(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        return -1;
}

static char a2i(char ch, char **src, int base, int *nump)
{
    char *p = *src;
    int num = 0;
    int digit;
    while ((digit = a2d(ch)) >= 0) {
        if (digit > base)
            break;
        num = num * base + digit;
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}


static void putchw(void *putp, putcf putf, int n, char z, char *bf)
{
    char fc = z ? '0' : ' ';
    char ch;
    char *p = bf;
    while (*p++ && n > 0)
        n--;
    while (n-- > 0)
        putf(putp, fc);
    while ((ch = *bf++))
        putf(putp, ch);
}

//putp is the data container
//putf is the function to output characters using the data container

void tfp_format(void *putp, putcf putf, char *fmt, va_list va)
{
	//Need enough space for float
	//1 negative indicator
	//10 digits for 32bit number
	//1 decimal point
	//PRINTF_FLOAT_DIGITS fraction digits
	//0 terminator
    char bf[ 1 + 10 + 1 + PRINTF_FLOAT_DIGITS + 1 ];
    char ch;

    while ((ch = *(fmt++))) {
        if (ch != '%')
            putf(putp, ch);
        else {
        	//Leading zero enabled
            char lz = 0;
#ifdef 	PRINTF_FLOAT_SUPPORT
            //How many digits to show, default will depend type
            char digits = -1;
#endif
#ifdef 	PRINTF_LONG_SUPPORT
            char lng = 0;
#endif
            //Minimum width
            int w = 0;
            ch = *(fmt++);
            if (ch == '0') {
                ch = *(fmt++);
                lz = 1;
            }
            //Check for any leading digits indicating minimum width
            if (ch >= '0' && ch <= '9') {
                ch = a2i(ch, &fmt, 10, &w);
            }
#ifdef 	PRINTF_LONG_SUPPORT
            if (ch == 'l') {
                ch = *(fmt++);
                lng = 1;
            }
#endif
#ifdef 	PRINTF_FLOAT_SUPPORT
            //Decimal seperator for indicating digits
            if ( ch == '.' ) {
                ch = *(fmt++);
            	//Simple check only supporting 0-9 digits
                if (ch >= '0' && ch <= '9') {
                	digits = ( ch - '0' );
                    ch = *(fmt++);
                }
            }
#endif
            switch (ch) {
            case 0:
                goto abort;
            case 'u':{
#ifdef 	PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a(va_arg(va, unsigned long int), 10, 0, bf);
                    else
#endif
                        ui2a(va_arg(va, unsigned int), 10, 0, bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
            case 'd':{
#ifdef 	PRINTF_LONG_SUPPORT
                    if (lng)
                        li2a(va_arg(va, unsigned long int), bf);
                    else
#endif
                        i2a(va_arg(va, int), bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
            case 'x':
            case 'X':
#ifdef 	PRINTF_LONG_SUPPORT
                if (lng)
                    uli2a(va_arg(va, unsigned long int), 16, (ch == 'X'), bf);
                else
#endif
                    ui2a(va_arg(va, unsigned int), 16, (ch == 'X'), bf);
                putchw(putp, putf, w, lz, bf);
                break;
            case 'c':
                putf(putp, (char) (va_arg(va, int)));
                break;
            case 's':
                putchw(putp, putf, w, 0, va_arg(va, char *));
                break;
#ifdef 	PRINTF_FLOAT_SUPPORT
            case 'f':
            	//float will always get passed as double as an argument
            	f2a( va_arg(va, double), digits >= 0 ? digits : PRINTF_FLOAT_DIGITS, bf );
                putchw(putp, putf, w, lz, bf);
                break;
#endif
            case '%':
                putf(putp, ch);
            default:
                break;
            }
        }
    }
  abort:;
}


void init_printf(void *putp, void (*putf) (void *, char))
{
    stdout_putf = putf;
    stdout_putp = putp;
}

void tfp_printf(char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    tfp_format(stdout_putp, stdout_putf, fmt, va);
    va_end(va);
    while (!uartTransmitEmpty());
}

static void putcp(void *p, char c)
{
    *(*((char **) p))++ = c;
}



void tfp_sprintf(char *s, char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    tfp_format(&s, putcp, fmt, va);
    putcp(&s, 0);
    va_end(va);
}
