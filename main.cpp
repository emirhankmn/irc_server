/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/16 22:31:51 by eakman            #+#    #+#             */
/*   Updated: 2025/03/16 22:31:51 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cerrno>  
#include <climits> 

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "❌ Usage: ./ircserv <port> <password>" << std::endl;
        return EXIT_FAILURE;
    }

    char* endptr;
    errno = 0;
    long port = strtol(argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || port <= 0 || port > 65535) {
        std::cerr << "❌ Error: Invalid port number. Port must be between 1 and 65535." << std::endl;
        return EXIT_FAILURE;
    }

    std::string password = argv[2];
    Server server(port, password);  
    server.init();
    server.run();
    return 0;
}
