#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <iostream>

#define PORT 8080

int main()
{
    int sock = 0; long valread;
    struct sockaddr_in serv_addr;
//    char *hello = "Hello from client";
    char buffer[1024] = {0};

    std::string hello = "POST /upload HTTP/1.1\r\n";
    hello += "Host: localhost:8080\r\n";
//                        "User-Agent: curl/7.54.0\n";
    hello += "Accept: */*\r\n";
    hello += "name: CHHHUUUUUCK\r\n";
    hello += "Transfer-Encoding: chunked\r\n";
    hello += "Content-Type: text/html\r\n\r\n";
    hello += "4\r\n";
    hello += "Wiki\r\n";
    hello += "6\r\n";
    hello += "pedia \r\n";
    hello += "E\r\n";
    hello += "in \r\n";
    hello += "\r\n";
    hello += "chunks.\r\n";
    hello += "0\r\n";
    hello += "\r\n";


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock , hello.c_str() , hello.size() , 0 );
    printf("Hello message sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    return 0;
}
