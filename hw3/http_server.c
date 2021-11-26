#define BUF_SIZE 1000

#define HEADER_FMT "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n"

#define NOT_FOUND_CONTENT       "<h1>404 Not Found</h1>\n"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>


void error_handling(char *message);
void response(char *header, int status, char *type, long len);
void file_type(char *ct_type, char *file_name);
void error_404(int clnt_sock);
void http_server(int clnt_sock);

int main(int argc, char **argv) {
    int port, pid;
    int serv_sock, clnt_sock;

    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_size;

    if (argc != 2) {
	    printf("Usage: %s <port>\n", argv[0]);
	    exit(1);
    }

    port = atoi(argv[1]);
    
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    
    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
	    error_handling("bind() error");
    if(listen(serv_sock, 20) == -1)
	    error_handling("listen() error");

    // to handle zombie process
    signal(SIGCHLD, SIG_IGN);

    while (1) {
	clnt_adr_size = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_size);
        if (clnt_sock < 0)
		error_handling("accept() error");

        pid = fork();
        if (pid == 0) { 
		close(serv_sock);
		http_server(clnt_sock);
		close(clnt_sock);
		exit(0);
        }

        if (pid != 0)
		close(clnt_sock);
        if (pid < 0)
		error_handling("fork() error");
    }
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void response(char *header, int status, char *type, long len) {
    char status_text[40];
    if(status == 200)
            strcpy(status_text, "OK");
    else if(status == 404)
            strcpy(status_text, "Not Found");
    sprintf(header, HEADER_FMT, status, status_text, type, len);
    //haeder에 내용을 저장함
}

//파일 확장자 판단
void file_type(char *ct_type, char *file_name) {
    char *ext = strrchr(file_name, '.');
    //.의 마지막 위치 
    if (!strcmp(ext, ".html")) //원하는 것과같으면 0반환
            strcpy(ct_type, "text/html");
    else if (!strcmp(ext, ".jpg"))
           strcpy(ct_type, "imapge/jpg");
    else if (!strcmp(ext, ".png"))
            strcpy(ct_type, "image/png");
    else strcpy(ct_type, "text/plain");
}

void error_404(int clnt_sock) {
    char header[BUF_SIZE];
    response(header, 404, "text/html", sizeof(NOT_FOUND_CONTENT));

    write(clnt_sock, header, strlen(header));
    write(clnt_sock, NOT_FOUND_CONTENT, sizeof(NOT_FOUND_CONTENT));
    printf("%s",header);
    //404error 출력
}

void http_server(int clnt_sock) {
    char header[BUF_SIZE];
    char buf[BUF_SIZE];
    read(clnt_sock, buf, BUF_SIZE);
    //clnt가 요청한 request message 읽기

    char *method = strtok(buf, " ");
    //요청 방법 출력, 공백 전까지 읽어오기
    char *file_name = strtok(NULL, " ");
    //요청 파일이 무엇인지 확인함. ex) index.html -> /index.html

    char file_name2[BUF_SIZE];
    char *local_name;
    struct stat st;

    strcpy(file_name2, file_name);
    //내용 복사
    if (!strcmp(file_name2, "/")) strcpy(file_name2, "/index.html");
    //요청 파일이 index.html인지 확인함
    local_name = file_name + 1;
    
    if(stat(local_name, &st) < 0){
	    error_404(clnt_sock);
	    return;
    }
    //stat 파일 정보를 잘 읽어오면 0 반환
    int fd = open(local_name, O_RDONLY);
    //파일 읽어오기
    if(fd < 0){
            error_404(clnt_sock);
            return;
    }
    //파일이 없으면 404 출력
    
    int ct_len = st.st_size;
    char ct_type[40];
    file_type(ct_type, local_name);
    response(header, 200, ct_type, ct_len);
    write(clnt_sock, header, strlen(header));
    printf("%s", header);//200 OK 출력
    int cnt;
    while ((cnt = read(fd, buf, BUF_SIZE)) > 0)
            write(clnt_sock, buf, cnt);
    //읽어온 파일 쓰기
}
