#include "decimal.h"

Decimal __decimal_new(const char* str) {
	Decimal ret;
	int len = strlen(str);
	int bits;
	int start = 0;

	bool isNeg = false;

	if(str[0] == '-') {
		bits = 1 + (len - 1);
		isNeg = true;
		start = 1;
	} else {
		bits = 1 + (len);
	}

	char* ptr = (char*) malloc((bits + 1) / 2 + 1);

	for(int i = 0; i < (bits + 1) / 2 + 1; i++) {
		ptr[i] = 0;
	}

	if(isNeg) {
		__decimal_write(ptr, 0, 0xF);
	}

	int loc = 1;
	int fpoint = -1;
	for(int i = start; i < len; i++) {
		if(str[i] != '.') {
			int value = (str[i] - '0') & 0xF;
			__decimal_write(ptr, loc++, value);
		} else {
			fpoint = loc;
			__decimal_write(ptr, loc++, 0xF);
		}
	}

	if(fpoint == -1) {
		bits++;
		fpoint = bits - 1;

		__decimal_write(ptr, bits - 1, 0xF);
	}

	ret.len = bits;
	ret.ptr = ptr;
	ret.fpoint = fpoint;

	ret = decimal_trim(ret);
	free(ptr);

	return ret;
}

Decimal decimal_from_int(int x) {
	return decimal_from_longlong(x);
}

Decimal decimal_from_longlong(long long x) {
	Decimal ret;
	bool isNeg = x < 0;
	int len = 0;
	char* str;

	if(isNeg) {
		x = -x;
	}
	long long p = x;
	while(p > 0) {
		len++;
		p /= 10;
	}

	len += 2;

	str = (char*) malloc((len + 1) / 2);
	for(int i = 0; i < (len + 1) / 2; i++) {
		str[i] = 0;
	}

	if(isNeg) {
		__decimal_write(str, 0, 0xF);
	}

	__decimal_write(str, (len - 1), 0xF);
	for(int i = len - 2; i >= 1; i--) {
		__decimal_write(str, i, x % 10);
		x /= 10;
	}

	ret.ptr = str;
	ret.len = len;
	ret.fpoint = ret.len - 1;
	return ret;
}

int __decimal_read(Decimal x, int n) {
	int loc = n / 2;

	if(n % 2 == 0) {
		return ((x.ptr[loc] & 0xF0) >> 4) & 0xF;
	} else {
		return (x.ptr[loc] & 0xF);
	}
}
void __decimal_write(char* ptr, int loc, int value) {
	value &= 0xF;
	if(loc % 2 == 0) {
		ptr[loc / 2] &= 0x0F;
		ptr[loc / 2] |= value << 4;
	} else {
		ptr[loc / 2] &= 0xF0;
		ptr[loc / 2] |= value;
	}
}

const char* decimal_to_cstring(Decimal x) {
	char* str = malloc(x.len + 2);
	int header = 0;

	for(int i = 0; i < x.len + 2; i++) {
		str[i] = 0;
	}

	if(__decimal_read(x, 0) != 0) {
		str[header++] = '-';
	}

	for(int i = 1; i < x.len; i++) {
		int value = __decimal_read(x, i);
		if(value == 0xF) {
			str[header++] = '.';
		} else {
			str[header++] = __decimal_read(x, i) + '0';
		}
	}

	return str;
}

Decimal decimal_make_format(Decimal x, int integer, int fraction) {
	Decimal ret;

	int len = 1 + integer + 1 + fraction;
	char* str;
	int header = 0;

	if(decimal_is_negative(x)) {
		str = (char*) malloc((len + 1) / 2 + 1);
		for(int i = 0; i < (len + 1) / 2 + 1; i++)
			str[i] = 0;

		__decimal_write(str, header++, 0xF);
	} else {
		str = (char*) malloc((len + 1) / 2 + 1);
		for(int i = 0; i < (len + 1) / 2 + 1; i++)
			str[i] = 0;

		__decimal_write(str, header++, 0x0);
	}


	for(int i = integer - 1; i >= 0; i--) {
		__decimal_write(str, header++, decimal_digit(x, i));
	}
	ret.fpoint = header;
	__decimal_write(str, header++, 0xF);
	for(int i = 1; i <= fraction; i++) {
		__decimal_write(str, header++, decimal_digit(x, -i));
	}

	ret.len = len;
	ret.ptr = str;
	return ret;
}

bool decimal_is_negative(Decimal x) {
	return __decimal_read(x, 0) != 0;
}

int decimal_digit(Decimal x, int n) {
	int fpoint = -1;

	int loc = x.fpoint - n - ((n >= 0) ? 1 : 0);

	if(loc >= 1 && loc < x.len) {
		return __decimal_read(x, loc);
	} else {
		return 0;
	}

	return 0;
}
void decimal_set_digit(Decimal x, int n, int v) {

	int fpoint = -1;

	for(int i = 1; i < x.len; i++) {
		int value;
		value = __decimal_read(x, i);

		if(value == 0xF) {
			int loc = i - n - ((n >= 0) ? 1 : 0);

			if(loc >= 1 && loc < x.len) {
				__decimal_write(x.ptr, loc, v);
				return;
			} else {
				return;
			}
		}
	}

	int loc = x.len - n - 1;
	if(loc >= 1 && loc < x.len) {
		__decimal_write(x.ptr, loc, v);
	}

	return;
}

Decimal_format decimal_format(Decimal x) {
	Decimal_format ret = { 0,0 };
	int fpoint = -1;

	for(int i = 1; i < x.len; i++) {
		if(__decimal_read(x, i) == 0xF) {
			fpoint = i;
			continue;
		}

		if(fpoint == -1) {
			ret.integer++;
		} else {
			ret.fraction++;
		}
	}

	return ret;
}
int decimal_compare(Decimal x, Decimal y) {
	if(!decimal_is_negative(x) && decimal_is_negative(y)) {
		return 1;
	} else if(decimal_is_negative(x) && !decimal_is_negative(y)) {
		return -1;
	} else if(decimal_is_negative(x) && decimal_is_negative(y)) {
		return decimal_compare(decimal_negative(y), decimal_negative(x));
	}

	Decimal_format xfmt = decimal_format(x);
	Decimal_format yfmt = decimal_format(y);
	Decimal_format fmt;

	fmt.integer = (xfmt.integer > yfmt.integer) ? xfmt.integer : yfmt.integer;
	fmt.fraction = (xfmt.fraction > yfmt.fraction) ? xfmt.fraction : yfmt.fraction;

	x = decimal_make_format(x, fmt.integer, fmt.fraction);
	y = decimal_make_format(y, fmt.integer, fmt.fraction);

	for(int i = fmt.integer; i >= -fmt.fraction; i--) {
		if(decimal_digit(x, i) == decimal_digit(y, i)) {
			continue;
		}

		if(decimal_digit(x, i) > decimal_digit(y, i)) {
			return 1;
		} else {
			return -1;
		}
	}

	decimal_free(x);
	decimal_free(y);

	return 0;
}
Decimal decimal_negative(Decimal x) {
	Decimal ret;
	char* str = (char*) malloc((x.len + 1) / 2);
	memcpy(str, x.ptr, (x.len + 1) / 2);

	if(decimal_is_negative(x))
		__decimal_write(str, 0, 0x0);
	else
		__decimal_write(str, 0, 0xF);

	ret.ptr = str;
	ret.len = x.len;
	ret.fpoint = x.fpoint;

	return ret;
}

void decimal_free(Decimal x) {
	free(x.ptr);
}

int __decimal_len(Decimal_format fmt) {
	return (fmt.integer + fmt.fraction + 2) / 2;
}

Decimal decimal_trim(Decimal x) {
	Decimal ret;
	Decimal_format fmt = decimal_format(x);
	bool neg = decimal_is_negative(x);

	int start = 0;
	int end = 0;
	for(int i = fmt.integer - 1; i >= 1; i--) {
		if(decimal_digit(x, i) != 0) {
			start = i;
			break;
		}
	}
	for(int i = -fmt.fraction; i <= -1; i++) {
		if(decimal_digit(x, i) != 0) {
			end = -i;
			break;
		}
	}

	char* str = (char*) malloc((end + start + 1 + 2) / 2 + 1);
	int header = 1;

	for(int i = 0; i < (end + start + 3) / 2 + 1; i++) {
		str[i] = 0;
	}

	if(neg) {
		__decimal_write(str, 0, 0xF);
	}

	for(int i = start; i >= 0; i--) {
		__decimal_write(str, header++, decimal_digit(x, i));
	}

	ret.fpoint = header;
	__decimal_write(str, header++, 0xF);

	for(int i = 1; i <= end; i++) {
		__decimal_write(str, header++, decimal_digit(x, -i));
	}

	ret.ptr = str;
	ret.len = header;

	return ret;
}


Decimal decimal_shift(Decimal x, int n) {
	Decimal_format fmt = decimal_format(x);
	Decimal ret;

	char* str;

	if(n >= -fmt.integer && n <= fmt.fraction) {
		ret.len = x.len;
	} else if(n < -fmt.integer) {
		ret.len = x.len - fmt.integer + n;
	} else {
		ret.len = x.len - fmt.fraction + n;
	}

	str = (char*) malloc((ret.len + 1) / 2);
	for(int i = 0; i < (ret.len + 1) / 2; i++) {
		str[i] = 0;
	}
	ret.ptr = str;

	for(int i = 1; i < x.len; i++) {
		if(__decimal_read(x, i) == 0xF) {
			__decimal_write(str, i + n, 0xF);
			ret.fpoint = i + n;
			break;
		}
	}

	for(int i = -fmt.fraction; i <= fmt.integer; i++) {
		decimal_set_digit(ret, i + n, decimal_digit(x, i));
	}

	ret = decimal_trim(ret);
	free(str);

	return ret;
}

Decimal decimal_add(Decimal x, Decimal y) {
	if(decimal_is_negative(x) && decimal_is_negative(y)) {
		Decimal xneg = decimal_negative(x);
		Decimal yneg = decimal_negative(y);
		Decimal add = decimal_add(xneg, yneg);
		Decimal ret = decimal_negative(add);

		decimal_free(xneg);
		decimal_free(yneg);
		decimal_free(add);

		return ret;
	} else if(!decimal_is_negative(x) && decimal_is_negative(y)) {
		Decimal ret = decimal_sub(x, y);

		return ret;
	} else if(decimal_is_negative(x) && !decimal_is_negative(y)) {
		Decimal ret = decimal_sub(y, x);

		return ret;
	}
	Decimal ret;

	Decimal_format xfmt = decimal_format(x);
	Decimal_format yfmt = decimal_format(y);
	Decimal_format fmt;

	char* ptr, * ptr2;
	ptr = x.ptr;
	ptr2 = y.ptr;

	fmt.integer = (xfmt.integer > yfmt.integer) ? xfmt.integer : yfmt.integer;
	fmt.fraction = (xfmt.fraction > yfmt.fraction) ? xfmt.fraction : yfmt.fraction;

	x = decimal_make_format(x, fmt.integer, fmt.fraction);
	y = decimal_make_format(y, fmt.integer, fmt.fraction);

	char* str = (char*) malloc((fmt.integer + fmt.fraction + 2) / 2 + 1);
	int overflow = 0;
	int header = 2 + fmt.integer + fmt.fraction;

	for(int i = 0; i < (fmt.integer + fmt.fraction + 2) / 2 + 1; i++) {
		str[i] = 0;
	}

	for(int i = -fmt.fraction; i <= fmt.integer; i++) {
		if(i == 0) {
			ret.fpoint = header;
			__decimal_write(str, header--, 0xF);
		}

		int value1 = decimal_digit(x, i);
		int value2 = decimal_digit(y, i);
		int value = (value1 + value2 + overflow);
		overflow = value / 10;

		__decimal_write(str, header--, value % 10);
	}

	ret.ptr = str;
	ret.len = fmt.integer + fmt.fraction + 3 + overflow;

	char* old = ret.ptr;
	ret = decimal_trim(ret);

	free(old);
	decimal_free(x);
	decimal_free(y);

	return ret;
}

Decimal decimal_sub(Decimal x, Decimal y) {
	if(decimal_is_negative(x) && decimal_is_negative(y)) {
		Decimal xneg = decimal_negative(x);
		Decimal yneg = decimal_negative(y);
		Decimal ret = decimal_sub(yneg, xneg);
		return ret;
	} else if(decimal_is_negative(x) && !decimal_is_negative(y)) {
		Decimal xneg = decimal_negative(x);
		Decimal add = decimal_add(x, y);
		Decimal ret = decimal_negative(add);

		decimal_free(xneg);
		decimal_free(add);

		return ret;
	} else if(!decimal_is_negative(x) && decimal_is_negative(y)) {
		Decimal yneg = decimal_negative(y);
		Decimal ret = decimal_add(x, yneg);

		decimal_free(yneg);

		return ret;
	}

	Decimal ret;

	int cmp = decimal_compare(x, y);
	if(cmp < 0) {
		Decimal sub = decimal_sub(y, x);
		Decimal ret = decimal_negative(sub);

		decimal_free(sub);

		return ret;
	} else if(cmp == 0) {
		return decimal_new(0);
	}

	Decimal_format xfmt = decimal_format(x);
	Decimal_format yfmt = decimal_format(y);
	Decimal_format fmt;

	char* ptr, * ptr2;
	ptr = x.ptr;
	ptr2 = y.ptr;

	fmt.integer = (xfmt.integer > yfmt.integer) ? xfmt.integer : yfmt.integer;
	fmt.fraction = (xfmt.fraction > yfmt.fraction) ? xfmt.fraction : yfmt.fraction;

	x = decimal_make_format(x, fmt.integer, fmt.fraction);
	y = decimal_make_format(y, fmt.integer, fmt.fraction);

	char* str = (char*) malloc((fmt.integer + fmt.fraction + 2) / 2 + 1);
	int header = 2 + fmt.integer + fmt.fraction;

	for(int i = 0; i < (fmt.integer + fmt.fraction + 2) / 2 + 1; i++) {
		str[i] = 0;
	}

	for(int i = -fmt.fraction; i <= fmt.integer; i++) {
		if(i == 0) {
			ret.fpoint = header;
			__decimal_write(str, header--, 0xF);
		}

		int value1 = decimal_digit(x, i);
		int value2 = decimal_digit(y, i);
		int value;

		if(value1 >= value2) {
			value = value1 - value2;
		} else {
			for(int j = i + 1; j <= fmt.integer; j++) {
				if(decimal_digit(x, j) != 0) {
					decimal_set_digit(x, j, decimal_digit(x, j) - 1);
					for(int k = j - 1; k >= i + 1; k--) {
						decimal_set_digit(x, k, 9);
					}
					value = 10 + value1 - value2;

					break;
				}
			}
		}
		__decimal_write(str, header--, value);
	}

	ret.ptr = str;
	ret.len = fmt.integer + fmt.fraction + 3;

	char* old = ret.ptr;
	ret = decimal_trim(ret);

	free(old);
	decimal_free(x);
	decimal_free(y);

	return ret;
}
Decimal decimal_mul(Decimal x, Decimal y) {
	if(decimal_is_negative(x) && decimal_is_negative(y)) {
		Decimal xneg = decimal_negative(x);
		Decimal yneg = decimal_negative(y);
		Decimal ret = decimal_mul(xneg, yneg);

		decimal_free(xneg);
		decimal_free(yneg);
		return ret;
	} else if(decimal_is_negative(x) && !decimal_is_negative(y)) {
		Decimal xneg = decimal_negative(x);
		Decimal mul = decimal_mul(xneg, y);
		Decimal ret = decimal_negative(mul);

		decimal_free(xneg);
		decimal_free(mul);

		return ret;
	} else if(!decimal_is_negative(x) && decimal_is_negative(y)) {
		Decimal yneg = decimal_negative(y);
		Decimal mul = decimal_mul(x, yneg);
		Decimal ret = decimal_negative(mul);

		decimal_free(yneg);
		decimal_free(mul);

		return ret;
	}
	Decimal ret;

	Decimal mul;
	Decimal shift;

	Decimal_format xfmt = decimal_format(x);
	Decimal_format yfmt = decimal_format(y);

	Decimal X = decimal_shift(x, xfmt.fraction);
	Decimal Y = decimal_shift(y, yfmt.fraction);

	int xlen = X.len - 1;
	int ylen = Y.len - 1;

	mul = decimal_from_int(0);

	for(int i = 0; i < xlen; i++) {
		int overflow = 0;
		char* str = (char*) malloc((ylen + 3) / 2);
		for(int i = 0; i < (ylen + 3) / 2; i++) {
			str[i] = 0;
		}
		__decimal_write(str, ylen + 2, 0xF);

		Decimal temp = { str, ylen + 3, ylen + 2 };

		for(int j = 0; j < ylen + 1; j++) {
			int value1 = decimal_digit(X, i);
			int value2 = decimal_digit(Y, j);
			int value = value1 * value2 + overflow;
			overflow = value / 10;

			decimal_set_digit(temp, j, value % 10);
		}

		Decimal old = mul;
		Decimal temp2 = decimal_shift(temp, i);
		mul = decimal_add(mul, temp2);

		decimal_free(temp);
		decimal_free(temp2);
		decimal_free(old);
	}


	shift = decimal_shift(mul, -xfmt.fraction - yfmt.fraction);
	ret = decimal_trim(shift);

	decimal_free(X);
	decimal_free(Y);
	decimal_free(mul);
	decimal_free(shift);

	return ret;
}

Decimal_div_result decimal_div ( Decimal x, Decimal y, int p ) {
	if( decimal_is_negative ( x ) && decimal_is_negative ( y ) ) {
		Decimal xneg = decimal_negative ( x );
		Decimal yneg = decimal_negative ( y );
		Decimal_div_result ret = decimal_div ( xneg, yneg, p );

		decimal_free ( xneg );
		decimal_free ( yneg );
		return ret;
	} else if( decimal_is_negative ( x ) && !decimal_is_negative ( y ) ) {
		Decimal xneg = decimal_negative ( x );
		Decimal_div_result div = decimal_div ( xneg, y, p );
		Decimal_div_result ret = { decimal_negative ( div.Q ), decimal_negative ( div.R ) };

		decimal_free ( xneg );
		decimal_free ( div.Q );
		decimal_free ( div.R );

		return ret;
	} else if( !decimal_is_negative ( x ) && decimal_is_negative ( y ) ) {
		Decimal yneg = decimal_negative ( x );
		Decimal_div_result div = decimal_div ( x, yneg, p );
		Decimal_div_result ret = { decimal_negative ( div.Q ), decimal_negative ( div.R ) };

		decimal_free ( yneg );
		decimal_free ( div.Q );
		decimal_free ( div.R );

		return ret;
	}

	Decimal_div_result ret;

	Decimal_format xfmt = decimal_format ( x );
	Decimal_format yfmt = decimal_format ( y );

	int shift = (xfmt.fraction > yfmt.fraction) ? xfmt.fraction : yfmt.fraction;

	Decimal X = decimal_shift ( x, shift );
	Decimal Y = decimal_shift ( y, shift );
	Decimal_format fmt = decimal_format(X);
	Decimal q = decimal_from_int(0);
	Decimal Q = decimal_make_format(q, fmt.integer, p);
	Decimal N = decimal_from_int(0);

	for(int i = fmt.integer - 1; i >= -p; i--) {
		Decimal shift = decimal_shift(N, 1);
		Decimal digit = decimal_from_int(decimal_digit(X, i));
		Decimal add = decimal_add(shift, digit);

		for(int j = 9; j >= 0; j--) {
			Decimal J = decimal_from_int(j);
			Decimal mul = decimal_mul(Y, J);

			if(decimal_compare(mul, add) <= 0) {
				decimal_set_digit(Q, i, j);
				
				Decimal N_old = N;
				N = decimal_sub(mul, add);

				decimal_free(N_old);

				break;
			}

			decimal_free(J);
			decimal_free(mul);
		}

		decimal_free(shift);
		decimal_free(digit);
		decimal_free(add);
	}

	Decimal PQ = decimal_mul(Q, y);
	Decimal R = decimal_sub(x, PQ);

	ret.Q = decimal_trim(Q);
	ret.R = decimal_trim(R);

	decimal_free(Q);
	decimal_free(R);
	decimal_free(X);
	decimal_free(Y);
	decimal_free(q);
	decimal_free(N);
	decimal_free(PQ);

	return ret;
}
