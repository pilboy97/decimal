#include "panic.h"

void panic ( const char* str ) {
	fprintf ( stderr, str );
	exit ( 0 );
}