// sendToCGI_post.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char** argv )
{
	char* data;
	
	printf("<title> result </title>\n");

	data = getenv("QUERY_STRING");
	data = strtok( data, "=" );

	if( strstr( data, "" ) != NULL )
	{
		printf("<h3> Hello </h3>\n");
		printf("<p> Nice to meet you %s!!", data);
	}

	else
	{
		printf("<h3> Sorry </h3>\n");
		printf("<p>I don't know your name. please try again!!");
	}

	return 0;
}
