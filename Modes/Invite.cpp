/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 04:20:44 by eakman            #+#    #+#             */
/*   Updated: 2025/04/15 04:20:44 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Invite.hpp"
#include <iostream>
#include <string>
#include <set>
#include <sys/socket.h>

namespace Modes {

void setInviteOnly(Server& server, int client_fd, const std::string& channel, bool enable) {
    std::cout << "🔍 DEBUG: setInviteOnly çağrıldı! Kanal: " << channel
              << " Enable: " << (enable ? "+i" : "-i") << "\n";

    std::string& currentModes = server.getChannelModes()[channel];

    if (enable) {
        if (currentModes.find('i') == std::string::npos)
            currentModes += 'i';
        server.getInviteOnlyChannels().insert(channel);
    } else {
        size_t pos = currentModes.find('i');
        if (pos != std::string::npos)
            currentModes.erase(pos, 1);
        server.getInviteOnlyChannels().erase(channel);
    }

    std::string response = ":ft_irc MODE " + channel + (enable ? " +i" : " -i") + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

}
