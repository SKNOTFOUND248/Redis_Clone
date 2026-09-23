#include <cerrno>
#include <cstring>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int PORT = 6379;
constexpr int BUFFER_SIZE = 1024;

int main() {
    // ------------------------------------------------------------
    // 1. Create a TCP socket
    // ------------------------------------------------------------
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        std::cerr << "socket: "
                  << std::strerror(errno)
                  << '\n';

        return 1;
    }

    std::cout << "Socket created. fd = "
              << server_fd
              << '\n';


    // ------------------------------------------------------------
    // 2. Configure server address
    // ------------------------------------------------------------
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);


    // ------------------------------------------------------------
    // 3. Bind socket to 0.0.0.0:6379
    // ------------------------------------------------------------
    if (bind(
            server_fd,
            reinterpret_cast<sockaddr*>(&server_address),
            sizeof(server_address)
        ) == -1) {

        std::cerr << "bind: "
                  << std::strerror(errno)
                  << '\n';

        close(server_fd);
        return 1;
    }

    std::cout << "Socket bound to port "
              << PORT
              << '\n';


    // ------------------------------------------------------------
    // 4. Start listening for TCP connections
    // ------------------------------------------------------------
    if (listen(server_fd, SOMAXCONN) == -1) {

        std::cerr << "listen: "
                  << std::strerror(errno)
                  << '\n';

        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port "
              << PORT
              << '\n';


    // ------------------------------------------------------------
    // 5. Accept clients forever
    // ------------------------------------------------------------
    while (true) {

        sockaddr_in client_address{};
        socklen_t client_address_length =
            sizeof(client_address);

        int client_fd = accept(
            server_fd,
            reinterpret_cast<sockaddr*>(&client_address),
            &client_address_length
        );

        if (client_fd == -1) {

            std::cerr << "accept: "
                      << std::strerror(errno)
                      << '\n';

            continue;
        }

        std::cout << "Client connected. fd = "
                  << client_fd
                  << '\n';


        // --------------------------------------------------------
        // 6. Receive data from client
        // --------------------------------------------------------
        char buffer[BUFFER_SIZE]{};

        ssize_t bytes_received = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );


        // --------------------------------------------------------
        // 7. Handle recv() error
        // --------------------------------------------------------
        if (bytes_received == -1) {

            std::cerr << "recv: "
                      << std::strerror(errno)
                      << '\n';

            close(client_fd);
            continue;
        }


        // --------------------------------------------------------
        // 8. Client disconnected
        // --------------------------------------------------------
        if (bytes_received == 0) {

            std::cout << "Client disconnected\n";

            close(client_fd);
            continue;
        }


        // --------------------------------------------------------
        // 9. Convert received bytes into C-style string
        // --------------------------------------------------------
        buffer[bytes_received] = '\0';

        std::cout << "Received: "
                  << buffer
                  << '\n';


        // --------------------------------------------------------
        // 10. Handle PING command
        // --------------------------------------------------------
        if (std::strncmp(buffer, "PING", 4) == 0) {

            const char response[] = "PONG\r\n";

            ssize_t bytes_sent = send(
                client_fd,
                response,
                sizeof(response) - 1,
                0
            );

            if (bytes_sent == -1) {

                std::cerr << "send: "
                          << std::strerror(errno)
                          << '\n';
            }
        }
        else {

            const char response[] =
                "ERR unknown command\r\n";

            send(
                client_fd,
                response,
                sizeof(response) - 1,
                0
            );
        }


        // --------------------------------------------------------
        // 11. Close client connection
        // --------------------------------------------------------
        close(client_fd);

        std::cout << "Client disconnected. fd = "
                  << client_fd
                  << '\n';
    }


    // Normally unreachable because of while(true)
    close(server_fd);

    return 0;
}