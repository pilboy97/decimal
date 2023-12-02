#include "decimal.h"

#include <stdio.h>


void printDecimal ( Decimal x ) {
	for( int i = 0; i < x.len; i++ ) {
		printf ( "%02x", 0xFF & x.ptr [ i ] );
	}
	puts ( "" );
}
int main ( ) {
	Decimal x = decimal_new ( -123.1234 );
	Decimal y = decimal_new ( 1234.1234 );

	printf ( "%s\n", decimal_to_cstring ( x ) );
	printf ( "%s\n", decimal_to_cstring ( y ) );

	return 0;
}