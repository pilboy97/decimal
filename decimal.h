#ifndef __DECIMAL_H__
#define __DECIMAL_H__

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "panic.h"

// first byte means sign
// if value is negative, it sets 0xFF.
// if not, it sets 0x00
// store 1 digit as 4bit number, store 2 numbers by 1byte
// ex 0.5      -> | 0000 0000 | | 0000 1111 | | 0101 0000 |
// ex2 -0.75   -> | 1111 1111 | | 0000 1111 | | 0111 0101 |
// ex3 1.225   -> | 0000 0000 | | 0001 1111 | | 0010 0010 | | 0101 0000 |
// ex4 -25.123 -> | 1111 1111 | | 0010 0101 | | 1111 0001 | | 0010 0011 |

typedef struct Decimal {
	const char* ptr;
	int len;
	int fpoint;
} Decimal;
typedef struct Decimal_format {
	int integer;
	int fraction;
} Decimal_format;
typedef struct Decimal_div_result {
	Decimal Q;
	Decimal R;
} Decimal_div_result;

#define decimal_new(X) __decimal_new(#X)

Decimal __decimal_new ( const char* str ); // make new decimal variable
Decimal decimal_from_int ( int x );
Decimal decimal_from_longlong ( long long x );

const char* decimal_to_cstring ( Decimal x ); // cast decimal type to cstring
Decimal decimal_make_format ( Decimal x, int integer, int fraction ); // decimal number formatting
													// ex 0.5 to (integer:2, fraction:3) -> 00.500

int decimal_digit ( Decimal x, int n ); // 10^n's digit of decimal variable
void decimal_set_digit ( Decimal x, int n, int value );

int __decimal_read ( Decimal x, int n );
void __decimal_write ( char* ptr, int loc, int value );
int __decimal_len ( Decimal_format fmt );
bool decimal_is_negative ( Decimal x ); // return true if x is negative, return false if not.

Decimal_format decimal_format ( Decimal x );

Decimal decimal_negative ( Decimal x );
int decimal_compare ( Decimal x, Decimal y ); // compare two decimal variables : -1 x < y, 0 x == y, 1 x > y

void decimal_free ( Decimal x );

Decimal decimal_trim ( Decimal x );
Decimal decimal_shift ( Decimal x, int n );

Decimal decimal_add ( Decimal x, Decimal y ); // add two decimal variables
Decimal decimal_sub ( Decimal x, Decimal y ); // subtract two decimal variables
Decimal decimal_mul ( Decimal x, Decimal y );
//Decimal_div_result decimal_div ( Decimal x, Decimal y, int p );
#endif
