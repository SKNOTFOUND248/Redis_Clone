#include <cerrno>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {

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

    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(6379);

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

    std::cout << "Socket successfully bound to port 6379\n";

    close(server_fd);

    return 0;
}