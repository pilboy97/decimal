#include "decimal.h"

Decimal __decimal_new ( const char* str ) {
	Decimal ret;
	int len = strlen ( str );
	int bits;
	int start = 0;

	bool isNeg = false;

	if( str [ 0 ] == '-' ) {
		bits = 2 + ( len - 1 ) + 1;
		isNeg = true;
		start = 1;
	}
	else {
		bits = 2 + ( len ) + 1;
	}

	char* ptr = ( char* ) malloc ((bits + 1) / 2 );

	for( int i = 0; i < (bits+1) / 2; i++ ) {
		ptr [ i ] = 0;
	}

	if( isNeg ) {
		ptr [ 0 ] = 0xFF;
	}

	int loc = 2;
	for( int i = start; i < len; i++ ) {
		if( str [ i ] != '.' ) {
			int value = ( str [ i ] - '0' ) & 0xF;

			if( loc % 2 == 0 ) {
				ptr [ loc / 2 ] |= value << 4;
			} else {
				ptr [ loc / 2 ] |= value;
			}
			loc++;
		} else {
			if( loc % 2 == 0 ) {
				ptr [ loc / 2 ] |= 0xF0;
			} else {
				ptr [ loc / 2 ] |= 0xF;
			}
			loc++;
		}
	}

	ret.len = bits;
	ret.ptr = ptr;

	return ret;
}

int __decimal_read ( Decimal x, int n ) {
	int loc = n / 2;

	if( n % 2 == 0 ) {
		return ( ( x.ptr [ loc ] & 0xF0 ) >> 4 ) & 0xF;
	} else {
		return ( x.ptr [ loc ] & 0xF );
	}
}

const char* decimal_to_cstring ( Decimal x ) {
	char* str = malloc ( x.len + 2 );
	int header = 0;

	for( int i = 0; i < x.len; i++ ) {
		str [ i ] = 0;
	}

	if( x.ptr [ 0 ] != 0 ) {
		str [ header++ ] = '-';
	}

	for( int i = 2; i < x.len; i++ ) {
		int value = __decimal_read ( x, i );
		if( value == 0xF ) {
			str [ header++ ] = '.';
		} else {
			str [ header++ ] = __decimal_read ( x, i ) + '0';
		}
	}

	return str;
}
/*
Decimal decimal_format ( Decimal x, int integer, int fraction ) {
}

Decimal_format decimal_get_format ( Decimal x ) {
}
int decimal_digit ( Decimal x, int n ) {
}

bool decimal_is_negative ( Decimal x ) {
}



int decimal_compare ( Decimal x, Decimal y ) {

}
Decimal decimal_add ( Decimal x, Decimal y ) {

}
Decimal decimal_sub ( Decimal x, Decimal y ) {

}

*/