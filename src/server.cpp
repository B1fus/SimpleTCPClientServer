#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <exception>
#include <thread>
#include <vector>
#include <list>
#include <chrono>
#include <ostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/synchroFile.h"

#define LOGQUEUE 10  // size of connection queue

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(sockaddr *sa)
{
    if (sa->sa_family == AF_INET){
        return &(((sockaddr_in *)sa)->sin_addr);
    }
    return &(((sockaddr_in6 *)sa)->sin6_addr);
}

//return current time as string YYYY-mm-dd HH:MM:SS
std::string get_date_time()
{
    using namespace std::chrono;
    auto now = system_clock::now();

    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    auto timer = system_clock::to_time_t(now);

    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S"); // YYYY-mm-dd HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

//struct for threads, need wrapper
struct ThreadCompleted{
    bool isCompleted = 0;
    std::thread* thread;
    ~ThreadCompleted(){
        thread->join();
        delete thread;
    }
};

namespace ClientHandler
{
    void doSomething(bool& isCompleted, int clientFD, SynchroFileWriter writer){
        //usleep(rand()%1000000); //do something long
        int bytesCount;
        char buff[1024];
        if((bytesCount = recv(clientFD, buff, 1024, 0)) == -1){
            perror("recv");
            isCompleted = 1;
            close(clientFD);
        }
        buff[1023] = '\0';
        printf("Server: received: [%s] %s\n", get_date_time().c_str(), buff);
        writer.write("[" + get_date_time() + "] " + buff + "\n");

        //printf("%ld\n", std::this_thread::get_id());
        close(clientFD);

        isCompleted = 1;
    }
} // namespace ClientHandler

class Server{
private:
    int m_socketFD;
    std::string m_port;
    std::shared_ptr<SynchroFile> m_logFile;

public:
    Server(std::string port) noexcept(false)
                            : m_port(port) {
        m_logFile = std::shared_ptr<SynchroFile>(new SynchroFile("log.txt"));

        addrinfo *servinfo, *p, hints{};
        int rv;

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int err;
        if ((err = getaddrinfo(NULL, m_port.c_str(), &hints, &servinfo)) != 0){
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            throw std::exception();
        }
        
        for (p = servinfo; p != NULL; p = p->ai_next){
            if ((m_socketFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
                perror("server: socket");
                continue;
            }
            int yes = 1;
            if (setsockopt(m_socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
                perror("setsockopt");
                throw std::exception();
            }
            if (bind(m_socketFD, p->ai_addr, p->ai_addrlen) == -1){
                close(m_socketFD);
                perror("server: bind");
                continue;
            }
            break;
        }

        if (p == NULL){
            fprintf(stderr, "server: failed to bind\n");
            throw std::exception();
        }

        freeaddrinfo(servinfo);
    }

    ~Server(){
        close(m_socketFD);
    }

    bool listenServer() const noexcept{
        if (listen(m_socketFD, LOGQUEUE) == -1){
            return 0;
        }
        return 1;
    }

    void run(){
        sockaddr_storage clientAddr; // client address
        socklen_t sin_size;
        int newFD;
        char s[INET6_ADDRSTRLEN];
        std::list<std::unique_ptr<ThreadCompleted>> threads; //for deleting threads when they done

        printf("server: waiting for connections…\n");

        while(1){
            sin_size = sizeof(clientAddr);
            newFD = accept(m_socketFD, (sockaddr *)&clientAddr, &sin_size);
            if (newFD == -1){
                perror("accept");
                continue;
            }
            inet_ntop(clientAddr.ss_family, get_in_addr((sockaddr *)&clientAddr), s, sizeof(s));
            //printf("server: got connection from %s\n", s);

            auto th =  std::unique_ptr<ThreadCompleted>(new ThreadCompleted());
            th->isCompleted = 0;
            th->thread = (new std::thread (ClientHandler::doSomething, std::ref(th->isCompleted),
                                          newFD, SynchroFileWriter(m_logFile)));
            threads.push_back(std::move(th));


            for(auto it = threads.begin(); it != threads.end(); it++){
                if((*it)->isCompleted){
                    it = threads.erase(it); //delete completed thread
                    it--;
                }
            }
            //printf("%d", threads.size());
        } 
    }
};
int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "usage: server [port]\n");
        return 1;
    }

    Server server(argv[1]);
    server.listenServer();
    server.run();



    return 0;
}