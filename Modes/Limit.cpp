/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Limit.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 04:25:44 by eakman            #+#    #+#             */
/*   Updated: 2025/04/15 04:25:44 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Limit.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>

namespace Modes {

void setLimit(Server& server, int client_fd, const std::string& channel, int limit) {
    if (limit <= 0) {
        std::string error_msg = ":ft_irc 461 " + channel + " :Limit parameter missing or invalid\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::cout << "🔍 DEBUG: setLimit çağrıldı! Kanal: " << channel << " Limit: " << limit << std::endl;
    server.getChannelLimits()[channel] = limit;
    server.updateChannelMode(channel, 'l', true);

    std::ostringstream oss;
    oss << limit;
    std::string response = ":ft_irc MODE " + channel + " +l " + oss.str() + "\r\n";
    server.sendToChannel(channel, "server", response, client_fd);
}

}
