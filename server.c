#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
int server_fd;

// Signal handler to gracefully stop the server
void handle_sigint(int sig) {
    printf("\nServer shutting down...\n");
    close(server_fd);
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);

    struct sockaddr_in address;
    int new_socket;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    const char *body = "Hello BOSCH This is my first Docker programme\n";
    char response[512];

    // Prepare the HTTP response
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(body), body);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow reuse of the port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind to port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("🚀 Listening on port %d...\n", PORT);

    // Serve requests one by one
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        // Read the request
        read(new_socket, buffer, sizeof(buffer) - 1);
        printf("🔹 Received request:\n%s\n", buffer);

        // Send the prepared HTTP response
        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}

