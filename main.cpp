#include <stdio.h>       // perror(), printf(),
#include <string.h>      // memset(), strlen(), strcmp(), bzero(), bcopy()
#include <unistd.h>      // chdir(), gethostname(), access(), close()
#include <stdlib.h>      // exit()
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h> // socket(), connect(), htons(), recv(), send()
#include <sstream>
#include <fstream>


// send client response with ERROR bc page dosn't exists
// @param: fd - descriptor of connection
// @return: -
void response_error(int fd){
    std::stringstream response;

    response << "HTTP/1.1 400 ERROR\r\n"
        << "Version: HTTP/1.1\r\n"
        << "Content-Type: text/html; charset=utf-8\r\n"
        << "Content-Length: 0\r\n";

    send(fd, response.str().c_str(), response.str().length(), 0);
}

// send client response with OK and page data
// @param: fd           - descriptor of connection
//         file_name    - name of pages' file
// @return: -
void response_ok(int fd, std::string file_name){

    std::stringstream response;
    std::stringstream response_body;
    // change link
    std::string link;
    std::ifstream in(link + file_name);
    if(!in) printf("No file\n");
    std::string s;
    getline(in, s, '\0');

    response << "HTTP/1.1 200 OK\r\n"
        << "Version: HTTP/1.1\r\n"
        << "Content-Type: text/html; charset=utf-8\r\n"
        << "Content-Length: " << s.length()
        << "\r\n\r\n"
        << s;

    send(fd, response.str().c_str(), response.str().length(), 0);
}

// check if file with name file_name exists
// @param: file_name - name of file
// @return: 1 - if file exists
//          0 - else
char page_exists(std::string file_name){
    std::string line = "";
    std::ifstream in("pages.txt");

    if (in.is_open())
        while (getline(in, line) && file_name.compare(line) != 0);
    in.close();
    if(file_name.compare(line) == 0) return 1;
    else return 0;

}

int main()
{
    struct sockaddr_in addr;
    struct hostent *hp;
    char buffer[1024] = {0}, temp_ch;
    int sid, sid_fd, result;
    size_t len = 0;
    std::stringstream message;
    std::string word;

    // get socket
    sid = socket(AF_INET, SOCK_STREAM, 0);
    if (sid == -1) exit(1);
    printf("Сервер\n--------------------\nСокет создан\n");

    bzero((void *)&addr, sizeof(addr));

    gethostname(buffer, 256);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port = htons(8081);
    addr.sin_family = AF_INET;

    if(bind(sid, (struct sockaddr *)&addr,sizeof(addr)) != 0){
        perror("Bind\n");
        close(sid);
        exit(1);
       }

    printf("Подключение сокета успешно\n");

    if(listen(sid,0) != 0){
        perror("Listen\n");
        close(sid);

        exit(1);
    }
    printf("Жду клиента\n");

    for(;;){
        sid_fd = accept(sid,0,0);
        printf("Клиент подключен\n");


        result = recv(sid_fd, buffer, sizeof(buffer), 0);

        if (result == -1) {
            // ошибка получения данных
            perror("Recv\n");
            close(sid);
            close(sid_fd);
            exit(1);
        } else if (result == 0) {
            // соединение закрыто клиентом
            perror("Connection closed\n");
            close(sid_fd);
        } else if (result > 0) {
            buffer[result] = '\0';
            message.str("");
            word.clear();
            message << buffer;
            message >> word;

            if(word != "GET") response_error(sid_fd);
            else {
                message >> word;
                if(word.compare("/") == 0) response_ok(sid_fd, "/root.html");
                else if(page_exists(word) == 1) response_ok(sid_fd, word);
                else response_error(sid_fd);
            }
        }

        close(sid_fd);
    }
    close(sid);
}
