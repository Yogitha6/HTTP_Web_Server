#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<sys/stat.h>

#define	QLEN		  30
#define	BUFFSIZE	4096

void *handleRequest(void *);
char* getdefaultWebPage()
{
    FILE *fp;
    char webPage[255];
    char *confFile = "/home/user/ws.conf";
    char *finalPage = malloc(100*sizeof(char));
    fp = fopen(confFile,"r");
    if(fp == NULL)
    {
        printf("file opening failed");
        fprintf(stderr,"Can't open the input file \n");
        exit(-1);
    }
    else
    {
            char *line = malloc(300*sizeof(char));
            while(fgets(line,2550,(FILE*)fp)!=NULL)
            {
               if(strstr(line,"#default web page"))
                {
                    strcpy(finalPage,fgets(webPage, 255, (FILE *)fp));
                    break;
                }
            }
    }
    fclose(fp);
    return finalPage;
}

int get_file_size(int filePointer)
{
    struct stat file_struct;
    if(fstat(filePointer, &file_struct) == -1)
        return(1);
    return (int)file_struct.st_size;
}

int getdefaultPort()
{
    FILE *fp;
    char portnumber[255];
    int port;
    fp = fopen("/home/user/ws.conf","r");
    if(fp == NULL)
    {
        fprintf(stderr,"Can't open the input file \n");
        exit(-1);
    }
    else
    {
        char *line = malloc(300*sizeof(char));
        while(fgets(line,255,(FILE *)fp)!=NULL)
        {
            if(strstr(line,"#service port number"))
                {
                    port = atoi(fgets(portnumber, 255, (FILE *)fp));
                    break;
                }
        }
    }
    fclose(fp);
    return port;
}
char* getdocumentRoot()
{
    FILE *fp;
    char docRoot[255];
    char *confFile = "/home/user/ws.conf";
    char *finaldocRoot = malloc(100*sizeof(char));

    fp = fopen(confFile,"r");
    if(fp == NULL)
    {
        printf("file opening failed");
        fprintf(stderr,"Can't open the input file \n");
        exit(-1);
    }
    else
    {
            char *line = malloc(300*sizeof(char));
            while(fgets(line,2550,(FILE*)fp)!=NULL)
            {
               if(strstr(line,"#document root"))
                {
                    strcpy(finaldocRoot,fgets(docRoot, 255, (FILE *)fp));
                    break;
                }
            }
    }
    fclose(fp);
    return finaldocRoot;
}

main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int requestCode;
    char buffer[BUFFSIZE];
    int receivedMessage;
    pthread_t threadID;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(getdefaultPort());
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        perror("Socket creation failed :");
        exit(-1);
    }

    if((bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)))<0)
    {
        perror("Socket binding failed :");
        exit(-1);
    }

    if(listen(serverSocket, QLEN)<0)
    {
        perror("Listening on the socket failed :");
        exit(-1);
    }
    unsigned int length = sizeof(clientAddress);
    time_t start = 0;
    time_t stop = 0;
    double timeElapsed = 0.0;
    start = clock();
    while((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &length)))
        {
            stop = clock();
            timeElapsed = (double) (stop-start)/CLOCKS_PER_SEC;
            if(timeElapsed >= 0.015)
            {
                break;
            }
        if(pthread_create(&threadID, NULL, handleRequest, (void*) &clientSocket) < 0)
        {
            perror("Thread creation failed:");
            exit(-1);

        }
        pthread_join(threadID , NULL);
        if(clientSocket<0)
        {
            perror("Accept failed:");
            exit(-1);
        }
        }
}

void send_client(int clientSocket, char *message)
{
    int length = strlen(message);
    if(send(clientSocket,message,length,0) == -1)
    {
        printf("Error in sending the message to client \n");
    }
}
void *handleRequest(void *clientSock)
{
    int clientSocket = *(int*)clientSock;
    int sizeRead;
    char defaultPage[200],documentRoot[200];

    //reading the default values from ws.conf
    strcpy(defaultPage,getdefaultWebPage());
    strcpy(documentRoot,getdocumentRoot());

    char *page, *requestType;
    char client_message[2000];
    int temp=0;
    int clientRequestID = 0;

        time_t start = 0;
        time_t stop = 0;
        double timeElapsed = 0.0;

    while((sizeRead =  recv(clientSocket,client_message,2000,0))>0)
    {
         client_message[sizeRead] = '\0';

     while(client_message!=NULL && client_message!= " ")
     {

        if(strstr(client_message,"GET")!=0)
        {
            clientRequestID=clientRequestID+1;
        }

            //processing the client request
            char* message = (char *)malloc(strlen(client_message));
            char* secondmessage = (char *)malloc(strlen(client_message));
            char *client_message2 = (char *)malloc(100*sizeof(char));
            char *client_message3 = (char *)malloc(100*sizeof(char));
            strcpy(message,client_message);
            strcpy(secondmessage,message);
            requestType = strtok(message, " ");// Splits spaces between words in str
            page = strtok(NULL," ");
            printf("page is %s\n",page);
            if(requestType!=NULL)
            {
                if(strcmp(requestType,"POST")==0)
                {
                     printf("400 Bad Request: Invalid Request Method \n");
                     send_client(clientSocket,"HTTP/1.1 400 Bad Request\r\n\n");
                     send_client(clientSocket,"Server: Yogitha/Private\r\n\r\n");
                     send_client(clientSocket,"<html><head><title>400 Bad Request Error :-( !! </head></title>");
                     send_client(clientSocket,"<body><h1> Request METHOD is BAD </h1><br><p>The request type from client is not GET.</p><br><br></body></html>");
                     break;
                }
            }
            if(page!=NULL)
            {
                if(strcmp(page,"/")==0)
                {
                    page = defaultPage;
                    page[strlen(page)-1]='\0';
                }
                else if((strstr(page,"//")!=0) || strstr(page,"[")!=0 || strstr(page,"]")!=0)
                {
                     printf("400 Bad Request: Invalid URI \n");
                     send_client(clientSocket,"HTTP/1.1 400 Bad Request\r\n\n");
                     send_client(clientSocket,"Server: Yogitha/Private\r\n\r\n");
                     send_client(clientSocket,"<html><head><title>400 Bad Request Error :-( !! </head></title>");
                     send_client(clientSocket,"<body><h1> Request URL is BAD </h1><br><p>The request URL from the client is malformed.</p><br><br></body></html>");
                     break;
                }
            }
            else
            {
            stop = clock();
            timeElapsed = (double) (stop-start)/CLOCKS_PER_SEC;

            if(start==0)
            {
               printf("There is no connection alive in the request header\n");
               //close(clientSocket);
               break;
            }

            if(timeElapsed >= 0.010)
            {
                printf("connection closed as the time has been more than 10 seconds\n");
                close(clientSocket);
                break;
            }
                break;
            }

            //constructing the full length path to the page requested
            int newSize = strlen(documentRoot)+strlen(page)+1;
            char * fullpath = (char *)malloc(newSize);
            strcpy(fullpath,documentRoot);
            fullpath[strlen(fullpath)-1] = '\0';
            strtok(fullpath, " ");
            fullpath = strtok(NULL," ");
            strcat(fullpath,page);
            if(strstr(page,"index")!=0)
            {temp = 1;}


            //reading the file in the file path and writing it to the client
            char *buffer;
            buffer = strstr(client_message, "HTTP/");
            if(buffer == NULL)
            {
                printf("Invalid request, Not Http, can't be processed");
            }
            else
            {
                if(strstr(buffer,"GET")!=0)
                {
                    client_message2 = strtok(secondmessage,"G");
                    strcpy(client_message3,"G");
                    char * temp = (char *)malloc(100*sizeof(char));
                    strcat(client_message3,strtok(NULL,"G"));
                    while((temp = strtok(NULL,"G"))!=NULL)
                    {
                        strcat(client_message3,"G");
                        strcat(client_message3,temp);
                    }

                    clientRequestID = clientRequestID + 1;
                }
                if((strstr(buffer,"1.1")) ==0 && strstr(buffer,"1.0")==0)
                {
                    printf("400 Bad Request: Invalid HTTP-Version \n");
                     send_client(clientSocket,"HTTP/1.1 400 Bad Request\r\n\n");
                     send_client(clientSocket,"Server: Yogitha/Private\r\n\r\n");
                     send_client(clientSocket,"<html><head><title>400 Bad Request Error :-( !! </head></title>");
                     send_client(clientSocket,"<body><h1> Request HTML Version is bad </h1><br><p>The request type from client is not either HTTP/1.1 or HTTP/1.0.</p><br><br></body></html>");
                     break;
                }
                int fileResource = open(fullpath,O_RDONLY,0);
                printf("opening %s \n", fullpath);
                int fileLength;

                *buffer=0;
                 buffer = NULL;


                if(fileResource == -1)
                {
                 char *resourceExtension = (char *)malloc(100*sizeof(char));
                 strtok(fullpath,".");
                 strcpy(resourceExtension,strtok(NULL,"."));
                 if(strstr(resourceExtension,"html")==0)
                 {
                     printf("501 NOT Implemented Error \n");
                     send_client(clientSocket,"HTTP/1.1 NOT Implemented Error \r\n\n");
                     send_client(clientSocket,"Server: Yogitha/Private\r\n\r\n");
                     send_client(clientSocket,"<body><h1> The content type is not supported </h1><br><p>The .</p><br><br></body></html>");
                     send_client(clientSocket,"<html><head><title>NOT IMPLEMENTED :-( !! </head></title>");
                     break;
                 }

                 printf("404 File Not Found Error \n");
                 send_client(clientSocket,"HTTP/1.1 404 Not Found\r\n\n");
                 send_client(clientSocket,"Server: Yogitha/Private\r\n\r\n");
                 send_client(clientSocket,"<html><head><title>404 Not Found Error :-( !! </head></title>");
                 send_client(clientSocket,"<body><h1> URL NOT FOUND </h1><br><p>The page you are looking for was not found on the server.</p><br><br></body></html>");
                 break;
                }
                else
                {
                 printf("200 OK \n");
                 send_client(clientSocket,"\n\n\rHTTP/1.1 200 OK\r\n\n");
                    if(strstr(requestType,"GET")!=0)
                    {
                        if((fileLength = get_file_size(fileResource))==-1)
                            printf("Error getting size \n");
                        if((buffer = (char *)malloc(fileLength))==NULL)
                            printf("Error allocating memory \n");
                        int total = read(fileResource,buffer,fileLength);

                        if(total == 0)
                        {
                             printf("500 Internal Server Error : System Error \n");
                             send_client(clientSocket,"HTTP/1.1 500 Internal Server Error\r\n\n");
                             send_client(clientSocket,"Server: Yogitha/Private\r\n\r\n");
                             send_client(clientSocket,"<html><head><title>500 Internal Server Error :-( !! </head></title>");
                             send_client(clientSocket,"<body><h1> Internal Server Error </h1><br><p>Server is experiencing unexpected system errors.</p><br><br></body></html>");
                             break;
                        }
                        send(clientSocket,buffer,total,0);
                        printf("send COMPLETED\n");
                        close(fileResource);
                        free(buffer);
                    }
                }
            }
            clientRequestID=clientRequestID-1;
            if(strstr(client_message3,"/")!=NULL && client_message3!=" ")
            {
                strcpy(client_message,client_message3);
                continue;
            }

            stop = clock();
            timeElapsed = (double) (stop-start)/CLOCKS_PER_SEC;

            if(timeElapsed < 0.10)
            {
               break;
            }

            if(timeElapsed >= 10.0 && clientRequestID==0)
            {
                printf("connection closed as the time has been more than 10 seconds");
                close(clientSocket);
                break;
            }
        }

        stop = clock();
        timeElapsed = (double) (stop-start)/CLOCKS_PER_SEC;

            if(timeElapsed >= 0.10)
            {
                printf("connection closed as the time has been more than 10 seconds\n");
                close(clientSocket);
                break;
            }
        if(temp ==1)
        {
            sleep(10);
        }
                printf("connection closed by Yogitha server\n");
                close(clientSocket);
        break;
    }
}
