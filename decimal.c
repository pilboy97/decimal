#include "decimal.h"

Decimal __decimal_new(int len) {
	Decimal ret;

	if((len + 1) / 2 < 4)
		len = 8;
	
	char * ptr = (char*) malloc(((len + 1) / 2 + 1) * sizeof(char));
	
	if(ptr == NULL) {
		panic("failed to malloc");
	}

	ret.len = len;
	ret.ptr = ptr;

	for(int i = 0; i < (len + 1) / 2 + 1; i++) {
		ptr[i] = 0;
	}

	return ret;
}
Decimal decimal_from_cstring(const char* str) {
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

	ret = __decimal_new(bits);


	if(isNeg) {
		decimal_write(ret, 0, 0xF);
	}

	int loc = 1;
	int fpoint = -1;
	for(int i = start; i < len; i++) {
		if(str[i] != '.') {
			int value = (str[i] - '0') & 0xF;
			decimal_write(ret, loc++, value);
		} else {
			fpoint = loc;
			decimal_write(ret, loc++, 0xF);
		}
	}

	if(fpoint == -1) {
		bits++;
		fpoint = bits - 1;

		decimal_write(ret, bits - 1, 0xF);
	}

	ret.len = bits;
	ret.ptr = ret.ptr;
	ret.fpoint = fpoint;

	Decimal ret_old = ret;
	ret = decimal_trim(ret);
	decimal_free(ret_old);

	return ret;
}

Decimal decimal_from_int(int x) {
	return decimal_from_longlong(x);
}

Decimal decimal_from_longlong(long long x) {
	Decimal ret;
	bool isNeg = x < 0;
	int len = 0;

	if(isNeg) {
		x = -x;
	}
	long long p = x;
	while(p > 0) {
		len++;
		p /= 10;
	}

	len += 2;

	ret = __decimal_new(len);

	if(isNeg) {
		decimal_write(ret, 0, 0xF);
	}

	decimal_write(ret, (len - 1), 0xF);
	for(int i = len - 2; i >= 1; i--) {
		decimal_write(ret, i, x % 10);
		x /= 10;
	}

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
	int header = 0;

	ret = __decimal_new(len);

	if(decimal_is_negative(x)) {
		decimal_write(ret, header++, 0xF);
	} else {
		decimal_write(ret, header++, 0x0);
	}


	for(int i = integer - 1; i >= 0; i--) {
		decimal_write(ret, header++, decimal_digit(x, i));
	}
	ret.fpoint = header;
	decimal_write(ret, header++, 0xF);
	for(int i = 1; i <= fraction; i++) {
		decimal_write(ret, header++, decimal_digit(x, -i));
	}

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
	int fpoint = x.fpoint;

	int loc = fpoint - n - ((n >= 0) ? 1 : 0);

	if(loc >= 1 && loc < x.len) {
		decimal_write(x, loc, v);
		return;
	} else {
		return;
	}

	loc = x.len - n - 1;
	if(loc >= 1 && loc < x.len) {
		decimal_write(x, loc, v);
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

	ret = __decimal_new(x.len);

	if(__decimal_read(x, 0) == 0xF) {
		decimal_write(ret, 0, 0x0);
	} else {
		decimal_write(ret, 0, 0xF);
	}

	for(int i = 1; i < x.len; i++) {
		int value = __decimal_read(x, i);
		decimal_write(ret, i, value);
	}

	ret.fpoint = x.fpoint;

	return ret;
}

void decimal_free(Decimal x) {
	free(x.ptr);
}

int __decimal_len(Decimal_format fmt) {
	return (fmt.integer + fmt.fraction + 2) / 2;
}

void decimal_write(Decimal x, int loc, int v) {
	if(0 <= loc && loc < x.len)
		__decimal_write(x.ptr, loc, v);
	else
		puts("!!");
}

Decimal decimal_trim(Decimal x) {
	Decimal ret;
	Decimal_format fmt = decimal_format(x);
	bool neg = decimal_is_negative(x);

	int start = -1;
	int end = 0;
	for(int i = fmt.integer - 1; i >= 0; i--) {
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
	
	if(end == 0 && start == -1)
		return decimal_from_int(0);

	if(start == -1) start = 0;

	ret = __decimal_new(end + start + 3);
	int header = 1;

	if(neg) {
		decimal_write(ret, 0, 0xF);
	}

	for(int i = start; i >= 0; i--) {
		decimal_write(ret, header++, decimal_digit(x, i));
	}

	ret.fpoint = header;
	decimal_write(ret, header++, 0xF);

	for(int i = 1; i <= end; i++) {
		decimal_write(ret, header++, decimal_digit(x, -i));
	}

	ret.len = header;

	return ret;
}


Decimal decimal_shift(Decimal x, int n) {
	Decimal_format fmt = decimal_format(x);
	Decimal_format neo = fmt;

	neo.integer += n;
	neo.fraction -= n;

	if(neo.integer < 0) neo.integer = 0;
	if(neo.fraction < 0) neo.fraction = 0;

	Decimal ret = __decimal_new(neo.fraction + neo.integer + 2);
	decimal_write(ret, neo.integer + 1, 0xF);
	ret.fpoint = neo.integer + 1;

	for(int i = -fmt.fraction; i <= fmt.integer; i++) {
		int value = decimal_digit(x, i);
		decimal_set_digit(ret, i + n, value);
	}

	Decimal ret_old = ret;
	ret = decimal_trim(ret);
	decimal_free(ret_old);

	return ret;
}

Decimal decimal_inc(Decimal x) {
	Decimal one = decimal_from_int(1);
	Decimal ret = decimal_add(x, one);
	
	decimal_free(one);
	return ret;
}
Decimal decimal_dec(Decimal x) {
	Decimal one = decimal_from_int(1);
	Decimal ret = decimal_sub(x, one);

	decimal_free(one);
	return ret;
}

Decimal decimal_max(Decimal x, Decimal y) {
	if(decimal_compare(x, y) > 0) {
		return decimal_copy(x);
	}

	return y;
}
Decimal decimal_min(Decimal x, Decimal y) {
	if(decimal_compare(x, y) < 0) {
		return decimal_copy(x);
	}

	return y;
}
Decimal decimal_abs(Decimal x) {
	if(!decimal_is_negative(x)) {
		return decimal_copy(x);
	}

	return decimal_negative(x);
}

Decimal decimal_pow(Decimal x,int n) {
	if(n == 0) return decimal_from_int(1);
	if(n == 1) return decimal_copy(x);


	Decimal X = decimal_pow(x, n / 2);
	Decimal ret = decimal_mul(X, X);

	if(n % 2 == 1) {
		Decimal ret_old = ret;
		decimal_free(ret_old);

		ret = decimal_mul(ret, x);
	}

	decimal_free(X);
	return ret;
}

Decimal decimal_factorial(Decimal x) {
	if(decimal_compare(x, decimal_from_int(0)) == 0) {
		return decimal_from_int(1);
	}
	if(decimal_compare(x, decimal_from_int(1)) == 0) {
		return decimal_from_int(1);
	}

	Decimal i = decimal_from_int(1);
	Decimal ret = decimal_from_int(1);

	while(decimal_compare(i, x) <= 0) {
		Decimal ret_old = ret;
		Decimal i_old = i;

		ret = decimal_mul(ret, i);
		i = decimal_inc(i);

		decimal_free(ret_old);
		decimal_free(i_old);
	}

	decimal_free(i);

	return ret;
}

Decimal decimal_pi() {
	Decimal ret = decimal_from_int(0);

	for(int i = 0; i < 5; i++) {
		Decimal m1 = decimal_from_int(-1);
		Decimal sign = decimal_pow(m1, i);
		Decimal Q = decimal_from_int(2 * i + 1);

		Decimal ret_old = ret;

		Decimal_div_result res = decimal_div(sign, Q, 30);
		
		printf("%s %s %s\n", decimal_to_cstring(sign), decimal_to_cstring(Q) , decimal_to_cstring(res.Q));

		ret = decimal_add(ret, res.Q);

		decimal_free(m1);
		decimal_free(sign);
		decimal_free(Q);
		decimal_free(ret_old);
		decimal_free(res.Q);
		decimal_free(res.R);
	}

	Decimal ret_old = ret;
	Decimal four = decimal_from_int(4);
	ret = decimal_mul(ret, four);
	return ret;
}

Decimal decimal_sin(Decimal x) {
	Decimal ret = decimal_from_int(0);
	
	for(int i = 0; i < 100; i++) {
		Decimal m1 = decimal_from_int(-1);
		Decimal sign = decimal_pow(m1, i);
		Decimal X = decimal_pow(x, 2 * i + 1);
		Decimal q = decimal_from_int(2 * i + 1);
		Decimal F = decimal_factorial(q);

		Decimal X2 = decimal_mul(sign, X);
		Decimal_div_result res = decimal_div(X2, F,100);

		Decimal ret_old = ret;
		ret = decimal_add(ret, res.Q);

		decimal_free(m1);
		decimal_free(q);
		decimal_free(sign);
		decimal_free(X);
		decimal_free(F);
		decimal_free(X2);
		decimal_free(res.Q);
		decimal_free(res.R);
	}

	return ret;
}

Decimal decimal_cos(Decimal x) {

}

Decimal decimal_exp(Decimal x) {

}
Decimal decimal_ln(Decimal x) {

}

Decimal decimal_sqrt(Decimal x) {

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

	Decimal X = decimal_make_format(x, fmt.integer, fmt.fraction);
	Decimal Y = decimal_make_format(y, fmt.integer, fmt.fraction);

	int overflow = 0;
	int header = 2 + fmt.integer + fmt.fraction;
	
	ret = __decimal_new(fmt.integer + fmt.fraction + 3);

	for(int i = -fmt.fraction; i <= fmt.integer; i++) {
		if(i == 0) {
			ret.fpoint = header;
			decimal_write(ret, header--, 0xF);
		}

		int value1 = decimal_digit(X, i);
		int value2 = decimal_digit(Y, i);
		int value = (value1 + value2 + overflow);
		overflow = value / 10;

		decimal_write(ret, header--, value % 10);
	}

	ret.len = fmt.integer + fmt.fraction + 3 + overflow;

	char* old = ret.ptr;
	ret = decimal_trim(ret);

	free(old);
	decimal_free(X);
	decimal_free(Y);

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

	fmt.integer = (xfmt.integer > yfmt.integer) ? xfmt.integer : yfmt.integer;
	fmt.fraction = (xfmt.fraction > yfmt.fraction) ? xfmt.fraction : yfmt.fraction;

	Decimal X = decimal_make_format(x, fmt.integer, fmt.fraction);
	Decimal Y = decimal_make_format(y, fmt.integer, fmt.fraction);

	ret = __decimal_new(fmt.integer + fmt.fraction + 3);
	int header = 2 + fmt.integer + fmt.fraction;

	for(int i = -fmt.fraction; i <= fmt.integer; i++) {
		if(i == 0) {
			ret.fpoint = header;
			decimal_write(ret, header--, 0xF);
		}

		int value1 = decimal_digit(X, i);
		int value2 = decimal_digit(Y, i);
		int value;

		if(value1 >= value2) {
			value = value1 - value2;
		} else {
			for(int j = i + 1; j <= fmt.integer; j++) {
				if(decimal_digit(X, j) != 0) {
					decimal_set_digit(X, j, decimal_digit(X, j) - 1);
					for(int k = j - 1; k >= i + 1; k--) {
						decimal_set_digit(X, k, 9);
					}
					value = 10 + value1 - value2;

					break;
				}
			}
		}
		decimal_write(ret, header--, value);
	}

	char* old = ret.ptr;
	ret = decimal_trim(ret);

	free(old);
	decimal_free(X);
	decimal_free(Y);

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
		Decimal temp = __decimal_new(ylen + 3);
		decimal_write(temp, ylen + 2, 0xF);
		temp.fpoint = ylen + 2;

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
		Decimal yneg = decimal_negative ( y );
		Decimal_div_result div = decimal_div ( x, yneg, p );
		Decimal_div_result ret = { decimal_negative ( div.Q ), decimal_negative ( div.R ) };

		decimal_free ( yneg );
		decimal_free ( div.Q );
		decimal_free ( div.R );

		return ret;
	}


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

	Decimal_div_result ret = { decimal_trim(Q), decimal_trim(R) };

	decimal_free(Q);
	decimal_free(R);
	decimal_free(X);
	decimal_free(Y);
	decimal_free(q);
	decimal_free(N);
	decimal_free(PQ);

	return ret;
}
Decimal decimal_copy(Decimal x) {
	Decimal ret = __decimal_new(x.len);

	for(int i = 0; i < x.len; i++) {
		decimal_write(ret, i, __decimal_read(x, i));
	}

	return ret;
}
