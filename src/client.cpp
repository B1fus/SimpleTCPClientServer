#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <exception>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(sockaddr *sa)
{
    if (sa->sa_family == AF_INET){
        return &(((sockaddr_in *)sa)->sin_addr);
    }
    return &(((sockaddr_in6 *)sa)->sin6_addr);
}

class Client{
private:
    int m_socketFD;
    std::string m_name;
    std::string m_port;

public:
    Client(std::string name, std::string port)
          :m_name(name + '\0'), m_port(port) {
        addrinfo *servinfo, *p, hints{};
        int rv;
        char s[INET6_ADDRSTRLEN];
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if ((rv = getaddrinfo(NULL, m_port.c_str(), &hints, &servinfo)) != 0){
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            throw std::exception();
        }
        
        for (p = servinfo; p != NULL; p = p->ai_next){
            if ((m_socketFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
                perror("client: socket");
                continue;
            }
            if (connect(m_socketFD, p->ai_addr, p->ai_addrlen) == -1){
                close(m_socketFD);
                perror("client: connect");
                continue;
            }
            break;
        }
        if (p == NULL){
            fprintf(stderr, "server: failed to bind\n");
            throw std::exception();
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
        printf("client: %s connecting to %s\n", m_name.c_str(), s);
        
        freeaddrinfo(servinfo);
    }

    ~Client(){
        close(m_socketFD);
    }

    void run(){
        if (send(m_socketFD, m_name.c_str(), m_name.size(), 0) == -1) 
            perror("send");
    }
};


int main(int argc, char *argv[]){
    if (argc != 4)
    {
        fprintf(stderr, "usage: client [client name] [port] [seconds of period]\n");
        return 1;
    }

    int period = atoi(argv[3]);
    while(1){
        Client client(argv[1], argv[2]);
        client.run();
        sleep(period);
    }

    return 0;
}