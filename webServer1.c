// webServer1.c
// multi process ver.

#include <stdio.h>
#include <stdlib.h>			// setenv(), getenv()
#include <string.h>			// memset()
#include <sys/types.h>		// socket()
#include <sys/socket.h>		// socket(), sockaddr_in
#include <arpa/inet.h>		// sockaddr_in
#include <unistd.h>			// close(), dup()
#include <pthread.h>
#include <signal.h>

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

	int fd[ 2 ];

	str_len = recv( clnt_sock, buf, BUFSIZE, 0 );
	printf("%s", buf);

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
		str = strtok( str, "/" );
	}

	if( strstr( str, ".cgi" ) != NULL ) 
	{
		int status;
		char buf[ BUFSIZE ];

		str = strtok( str, "?" );
		printf("filename : %s\n", str);

		/*
		char* name;	
		char* value;
		
		while( str != NULL )
		{
			name = strtok( NULL, "=" );
			// printf("%s\n", name);

			value = strtok( NULL, "& ");
			// printf("%s\n", value);

			setenv( name, value, 0 );

			name = NULL;
			value = NULL;
		}
		*/

		str = strtok( NULL, " " );
		// printf("%s\n", str );

		setenv( "QUERY_STRING", str, 0 );
		
		pthread_t pid = fork();

		pipe( fd );

		switch( pid )
		{
			case -1 :
			{
				error_handling( "process FAIL!!" );
			}
			
			case 0 :
			{
				// printf("child process\n");
				close( 1 );
				close( fd[ 0 ] );

				// dup2( fd[ 0 ], 0 );
				dup2( fd[ 1 ], 1 );	// 모든 출력이 fd[ 1 ] 에 write 된다.
				dup2( fd[ 0 ], 0 );

				printf("<title>hello</title>");

				execlp( "sendToCGI.cgi", NULL );

				// printf( "execlp FAIL !!!\n" );
			}

			default :
			{
				close( 0 );
				close( fd[ 1 ] );

				// dup2( fd[ 0 ], 0 );
			
				// printf("parent process\n");	
				
				wait( &status );
				// printf( "wait end!\n" );
				
				char protocol[] = "HTTP/1.1 200 OK\r\n\r\n";
	
				send( clnt_sock, protocol, strlen( protocol ), 0 );

				read( fd[ 0 ], buf, BUFSIZE );

				printf("%s\n", buf);
	
				send( clnt_sock, buf, strlen( buf ), 0 );
				
				/*	
				int len;	
				while ( ( len = read( fd[ 0 ], buf, BUFSIZE ) ) > 0 )
				{
					send( clnt_sock, buf, strlen( buf ), 0 );
				}
				*/

				close( clnt_sock );
				
				// send()
			}
		}
	}

	else
	{
		printf("filename : %s\n\n", str);
		strcpy( filename, str );

		sendData( clnt_sock, ct, filename );
	}



/*
	str_len = recv( clnt_sock, buf, BUFSIZE, 0 );
	printf("%s\n", buf);

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
		str = strtok( str, "/" );
	}

	if( strrchr( str, '?' ) != NULL ) 
	{
		str = strtok( str, "?" );	
	}
	
	printf("filename : %s\n", str);
	strcpy( filename, str );
	
	sendData( clnt_sock, ct, filename );
*/

	return 0;
}

void childHandler()
{
	int status;
    int spid;
    spid = wait(&status);
}

int main( int argc, char** argv )
{
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	unsigned int clnt_addr_size;
	pthread_t pid;
	int thr_id;
	int bf = sizeof( int );
	// pid_t pid;

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

	signal( SIGCHLD, (void *)childHandler );

	while( 1 )
	{
		clnt_sock = accept( serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size );

		if ( clnt_sock == -1 )
		{
			error_handling("accept() error!!!");
		}
	
		pid = fork();
	
		switch( pid )
		{
			case -1 :
			{
				error_handling( "자식 프로세스 생성 실패" );
			}
			
			// 자식 프로세스
			case 0 :
			{
				clntConnect( (void*) &clnt_sock );
				return 0;
			}

			// 부모 프로세스
			default :
			{
				close( clnt_sock );
				// printf("부모프로세스시다.\n");
			}
		}
	}	// while( 1 )
	
	return 0;
}
