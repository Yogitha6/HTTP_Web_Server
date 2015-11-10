#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <string.h>

main()
{

    int sockfd, portno, n;
    struct sockaddr_in server_addr;
    struct hostent *server;

    char buffer[256];

    portno = 23156;

    sockfd = socket(AF_INET,SOCK_STREAM, 0);

    if(sockfd < 0)
    {
        perror("Socket creation failed :");
        exit(-1);
    }

    server = gethostbyname("localhost");
    server_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(portno);

    if(connect(sockfd, (struct sockaddr *)&server_addr,sizeof(server_addr))<0)
    {
        perror("connection creation failed :");
        exit(-1);
    }
    printf("Please enter a message");
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if(n<0)
    {

        perror("write failed:");
        exit(-1);
    }
    n = read(sockfd, buffer, 255);
    if(n < 0)
    {
        perror("read failed:");
        exit(-1);
    }
    printf("The message i received, %s", buffer);

}
