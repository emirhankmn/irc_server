/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Key.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 04:24:13 by eakman            #+#    #+#             */
/*   Updated: 2025/04/15 04:24:13 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Key.hpp"
#include <iostream>
#include <sys/socket.h>

namespace Modes {

void setKey(Server& server, int client_fd, const std::string& channel, const std::string& key) {
    std::cout << "🔍 DEBUG: setKey çağrıldı! Kanal: " << channel << " Şifre: " << key << "\n";

    if (!key.empty())
        server.getChannelKeys()[channel] = key;
    else
        server.getChannelKeys().erase(channel);

    std::string response = (!key.empty())
        ? ":ft_irc MODE " + channel + " +k " + key + "\r\n"
        : ":ft_irc MODE " + channel + " -k\r\n";

    server.sendToChannel(channel, "server", response, client_fd);
}

}
