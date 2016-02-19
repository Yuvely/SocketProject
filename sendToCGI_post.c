// sendToCGI_post.c

#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv )
{
	char* data;
	
	printf("<title> result </title>\n");
	printf("<h3> Hello </h3>\n");

	data = getenv("QUERY_STRING");

	if( data == NULL )
	{
		printf("<p>I don't know your name. please try again!!");
	}

	else
	{
		printf("<p> Nice to meet you %s!!", data);
	}

	return 0;
}
