/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 03:53:54 by eakman            #+#    #+#             */
/*   Updated: 2025/04/15 03:53:54 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"
#include "../Commands.hpp"

namespace Commands {

void userCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string username, mode, unused, realname;
    iss >> username >> mode >> unused;
    std::getline(iss, realname);

    if (!server.isAuthorized(client_fd)) {
        std::string msg = ":ft_irc 451 :You have not registered\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }

    if (realname.empty() || realname[0] != ':') {
        std::string error_msg = ":ft_irc 461 USER :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string response = ":ft_irc 002 " + username + " :User registered successfully\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

}
