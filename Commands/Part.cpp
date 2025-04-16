/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 00:20:57 by eakman            #+#    #+#             */
/*   Updated: 2025/04/15 00:20:57 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Part.hpp"
#include "../Commands.hpp"

namespace Commands {

void partCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string channel;
    iss >> channel;

    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (server.getChannels()[channel].find(client_fd) == server.getChannels()[channel].end()) {
        std::string error_msg = ":ft_irc 442 " + channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (server.getChannelFounders()[channel] == client_fd) {
        if (!server.getChannelOperators()[channel].empty()) {
            int new_founder = *server.getChannelOperators()[channel].begin();
            server.getChannelFounders()[channel] = new_founder;
            std::cout << "👑 Founder değişti: " << new_founder << " artık founder!" << std::endl;
        } else {
            server.getChannelFounders().erase(channel);
            std::cout << "❌ Founder kaldırıldı, kanal sahipsiz kaldı." << std::endl;
        }
    }

    server.getChannels()[channel].erase(client_fd);
    std::string partMsg = ":" + server.getNicknames()[client_fd] + " PART " + channel + "\r\n";
    send(client_fd, partMsg.c_str(), partMsg.size(), 0);

    if (server.getChannels()[channel].empty()) {
        server.getChannels().erase(channel);
        server.getChannelFounders().erase(channel);
        server.getChannelOperators().erase(channel);
    }

    std::cout << "👋 Kullanıcı kanaldan ayrıldı: " << channel << std::endl;
}

}
