#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#define BUF_SIZE 1000000
#define REQUEST "GET /webhp HTTP/1.1\r\nUser-Agent: Mozilla/4.0\r\ncontent-type:text/html\r\nConnection: close\r\n\r\n"
void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    char message[BUF_SIZE];
    int str_len, recv_len, recv_cnt;
    struct sockaddr_in serv_adr;
    char *fname = "hw1.html";
    FILE *fp;

    if(argc!=3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }   

    sock = socket(PF_INET, SOCK_STREAM,0);
    if(sock == -1) 
        error_handling("socket() error");
    //inet_aton(argv[1], &serv_adr.sin_addr);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));
    
    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
        error_handling("connect() error!");
    else
        puts("Connected………");
    
    send(sock, REQUEST, sizeof(REQUEST),0);
    recv(sock, &message, BUF_SIZE,0); 
    fp = fopen("hw1.html","w");
    if(fwrite(message, sizeof(message), 1, fp) != 1) {
	fprintf(stderr, "fwrite error\n");
	exit(1);
    }
    fclose(fp);
    //printf("%s\n", message);

    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
