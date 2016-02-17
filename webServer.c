// webServer.c
// multi thread ver.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// memset()
#include <sys/types.h>		// socket()
#include <sys/socket.h>		// socket(), sockaddr_in
#include <arpa/inet.h>		// sockaddr_in
#include <unistd.h>			// close()
#include <pthread.h>

#define BUFSIZE 1024 
#define SMALLBUF 100


void error_handling( char* message )
{
	printf("%s\n", message);
	exit( 1 );
}

void sendWrongMessage( int data );

void sendData( int data, char* ct, char* filename )
{
	int sock = data;
	char protocol[] = "HTTP/1.1 200 OK\r\n\r\n";
	char buf[ BUFSIZE ];
	int len;

	FILE* file = fopen( filename, "r" );
	
	if( file == NULL )
	{
		error_handling( "FILE is NULL\n" );	
		exit( 1 );
	}
	
	send( sock, protocol, strlen( protocol ), 0 );

	while( len = fgets( buf, BUFSIZE, file ) != NULL )
	{
		send( sock, buf, strlen( buf ), 0 );
	}

	close( sock );
}

void sendWrongMessage( int data )
{
	int sock = data;
	char protocol[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
	char content[] = "<html><head><title>NETWORK</title></head>" "<body><font size=+5><br>ERROR!"  "</font></body></html>";
	
	send( sock, protocol, strlen( protocol ), 0 );
	send( sock, content, strlen( content ), 0 );

	close( sock );
}

void* clntConnect( void* data )
{
	int clnt_sock = *( (int*) data );
	int str_len = 0;
	char buf[ BUFSIZE ];
	char* str;
	char method[ SMALLBUF ];
	char ct[ SMALLBUF ];
	char filename[ SMALLBUF ];

	str_len = recv( clnt_sock, buf, BUFSIZE, 0 );
	printf( "%s\n", buf );

	if( str_len == 0 )
	{
		error_handling("recv() error!!!");
	}
	
 	str = strtok( buf, "\r\n" );
	
	if( strstr( str, "HTTP" ) == NULL)
	{
		printf("HTTP ERROR!!!\n");
		sendWrongMessage( clnt_sock );
		close( clnt_sock );
	}

	str = strtok( str, " " );

	if( strcmp( str, "GET" ) )
	{
		sendWrongMessage( clnt_sock );
		close( clnt_sock );
	}

	str = strtok( NULL, " " );
		
	if( ( strstr( str, "html" ) != NULL ) || ( strstr( str, "HTML" ) ) )
	{
		strcpy( ct, "text/html" );
	}

	else
	{
		strcpy( ct, "text/plain" );
	}

	if( strrchr( str, '/' ) != NULL )
	{
		str = strrchr( str, '/' );	
	}

	str = strtok( str, "/" );
	
	printf("filename : %s\n", str);
	strcpy( filename, str );
	
	sendData( clnt_sock, ct, filename );

	return 0;
}

int main( int argc, char** argv )
{
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	unsigned int clnt_addr_size;
	pthread_t p_id;
	int thr_id;
	int bf = sizeof( int );

	clnt_addr_size = sizeof( clnt_addr );

	serv_sock = socket( PF_INET, SOCK_STREAM, 0 );

	if( serv_sock == -1 )
	{
		error_handling("socket() error!!!");
	}

	/* 비정상종료된 상태로 아직 커널이 bind 정보를 유지하고 있을 때 발생하는 문제 해결 */	
	setsockopt( serv_sock, SOL_SOCKET, SO_REUSEADDR, (char*) &bf, ( (int)sizeof( bf ) ) );

	memset( &serv_addr, 0, sizeof( serv_addr ) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl( INADDR_ANY );
	serv_addr.sin_port = htons( atoi( argv[ 1 ] ) );

	if( bind( serv_sock, (struct sockaddr*) &serv_addr, sizeof( serv_addr ) ) == -1 )
	{
		error_handling("bind() error!!!");
	}

	if( listen( serv_sock, 5 ) == -1 )
	{
		error_handling("listen() error!!!");
	}

	while ( 1 )
	{
		clnt_sock = accept( serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size );

		if ( clnt_sock == -1 )
		{
			error_handling("accept() error!!!");
		}

		thr_id = pthread_create( &p_id, NULL, clntConnect, (void*) &clnt_sock );

		if ( thr_id != 0 )
		{
			error_handling("pthread_create() error!!!");
			exit( 1 );
		}

		pthread_detach( p_id );
	}
	
	return 0;
}
