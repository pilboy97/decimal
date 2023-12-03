#include "decimal.h"

Decimal __decimal_new ( const char* str ) {
	Decimal ret;
	int len = strlen ( str );
	int bits;
	int start = 0;

	bool isNeg = false;

	if( str [ 0 ] == '-' ) {
		bits = 2 + ( len - 1 );
		isNeg = true;
		start = 1;
	}
	else {
		bits = 2 + ( len );
	}

	char* ptr = ( char* ) malloc ((bits + 1) / 2 );

	for( int i = 0; i < (bits+1) / 2; i++ ) {
		ptr [ i ] = 0;
	}

	if( isNeg ) {
		ptr [ 0 ] = 0xFF;
	}

	int loc = 2;
	int fpoint = -1;
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
			fpoint = i;
			if( loc % 2 == 0 ) {
				ptr [ loc / 2 ] |= 0xF0;
			} else {
				ptr [ loc / 2 ] |= 0xF;
			}
			loc++;
		}
	}

	if(fpoint != -1) {
		if(loc % 2 == 0) {
			ptr[loc / 2] |= 0xF0;
		} else {
			ptr[loc / 2] |= 0xF;
		}
		loc++;
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

Decimal decimal_format ( Decimal x, int integer, int fraction ) {
	Decimal ret;

	int len = integer + fraction + 1;
	char* str;
	int header = 0;

	if(decimal_is_negative(x)) {
		len+=2;
		str = (char*) malloc((len + 1) / 2 + 1);
		for(int i = 0; i < (len + 1) / 2 + 1; i++)
			str[i] = 0;
		str[0] = 0xFF;
		header = 2;
	} else {
		str = (char*) malloc((len + 1) / 2 + 1);
		for(int i = 0; i < (len + 1) / 2 + 1; i++)
			str[i] = 0;
	}


	for(int i = integer - 1; i >= 0; i--) {
		if(header % 2 == 0) {
			str[header / 2] |= decimal_digit(x, i) << 4;
		} else {
			str[header / 2] |= decimal_digit(x, i);
		}
		header++;
	}
	if(header % 2 == 0) {
		str[header / 2] |= 0xF0;
	} else {
		str[header / 2] |= 0xF;
	}
	header++;
	for(int i = 1; i <= fraction; i++) {
		if(header % 2 == 0) {
			str[header / 2] |= decimal_digit(x, -i) << 4;
		} else {
			str[header / 2] |= decimal_digit(x, -i);
		}
		header++;
	}

	ret.len = len;
	ret.ptr = str;
	return ret;
}

bool decimal_is_negative ( Decimal x ) {
	return x.ptr[0] != 0;
}

int decimal_digit ( Decimal x, int n ) {
	int fpoint = -1;

	for(int i = 2; i < x.len; i++) {
		int value;
		if(i % 2 == 0) {
			value = (x.ptr[i / 2] & 0xF0) >> 4;
		} else {
			value = (x.ptr[i / 2] & 0xF);
		}

		if(value == 0xF) {
			int loc = i - n - ((n >= 0) ? 1 : 0);

			if(loc >= 2 && loc < x.len) {
				if(loc % 2 == 0) {
					return (x.ptr[loc / 2] & 0xF0) >> 4;
				} else {
					return x.ptr[loc / 2] & 0xF;
				}
			} else {
				return 0;
			}
		}
	}

	int loc = x.len - n - 1;
	if(loc >= 2 && loc < x.len) {
		if(loc % 2 == 0)
			return (x.ptr[loc / 2] & 0xF0) >> 4;
		else
			return x.ptr[loc / 2] & 0xF;
	}

	return 0;
}
/*




int decimal_compare ( Decimal x, Decimal y ) {

}
Decimal decimal_add ( Decimal x, Decimal y ) {

}
Decimal decimal_sub ( Decimal x, Decimal y ) {

}

*/
