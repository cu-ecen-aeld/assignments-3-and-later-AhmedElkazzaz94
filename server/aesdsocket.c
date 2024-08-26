#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>

#define PORT "9000"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
#define FILE_PATH "/var/tmp/aesdsocketdata"

int sockfd;
int file_fd;
int new_fd;  // Declare the new_fd variable outside the loop

void sigchld_handler(int s) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void sigint_handler(int sig) {
    syslog(LOG_INFO, "Caught signal, exiting");
    close(sockfd);
    close(file_fd);
    remove(FILE_PATH);
    exit(0);
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handle_client(int new_fd, struct sockaddr_storage their_addr) {
    char s[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    syslog(LOG_INFO, "Accepted connection from %s", s);

    // Receiving data
    char buf[1024];
    int num_bytes;
    while ((num_bytes = recv(new_fd, buf, sizeof buf - 1, 0)) > 0) {
        buf[num_bytes] = '\0';
        write(file_fd, buf, num_bytes);  // Append to the file

        if (strchr(buf, '\n')) {  // Check for newline character
            lseek(file_fd, 0, SEEK_SET);  // Go to the beginning of the file
            char file_buf[1024];
            int read_bytes;
            while ((read_bytes = read(file_fd, file_buf, sizeof file_buf)) > 0) {
                send(new_fd, file_buf, read_bytes, 0);  // Send file content
            }
        }
    }

    syslog(LOG_INFO, "Closed connection from %s", s);
    close(new_fd);
}

int main(int argc, char *argv[]) {
    int daemon_mode = 0;

    // Parse command line arguments
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }

    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    int rv;

    // Setup syslog
    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_DAEMON);

    // Setup signal handling for SIGINT and SIGTERM
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        syslog(LOG_ERR, "getaddrinfo: %s", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            syslog(LOG_ERR, "server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            syslog(LOG_ERR, "setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            syslog(LOG_ERR, "server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        syslog(LOG_ERR, "server: failed to bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        syslog(LOG_ERR, "listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "sigaction");
        exit(1);
    }

    if (daemon_mode) {
        pid_t pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "Failed to fork for daemon mode");
            exit(1);
        } else if (pid > 0) {
            exit(0);  // Parent exits
        }
        // Child continues
        if (setsid() < 0) {
            syslog(LOG_ERR, "Failed to setsid for daemon mode");
            exit(1);
        }
    }

    // Open or create the file for appending
    file_fd = open(FILE_PATH, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (file_fd == -1) {
        syslog(LOG_ERR, "Failed to open or create file");
        exit(1);
    }

    syslog(LOG_INFO, "server: waiting for connections...");

    while (1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            syslog(LOG_ERR, "accept");
            continue;
        }

        if (!fork()) {
            close(sockfd);
            handle_client(new_fd, their_addr);
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    close(sockfd);
    close(file_fd);
    closelog();

    return 0;
}
