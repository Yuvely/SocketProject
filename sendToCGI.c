// sendToCGI.c

#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv )
{
	char* data;
	long m, n;

	printf( "%s%c%c\n", "Content-type:text/html; charset=iso-8859-1", 13, 10 );
	printf("<title> result </title>\n");
	printf("<h3>result</h3>\n");

	data = getenv("QUERY_STRING");

	if( data == NULL )
	{
		printf("<p>Error! Error in passing data from form to script.");
	}

	else if( sscanf( data, "m=%ld&n=%ld", &m, &n ) != 2 )
	{
		printf("<p>Error! Invalid data. Data must be numeric.");
	}

	else
	{
		printf("<p>The product of %ld and %ld is %ld.", m, n, ( m * n ) );
	}
	
	return 0;
}
