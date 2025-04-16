/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Modes.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 13:51:40 by eakman            #+#    #+#             */
/*   Updated: 2025/04/11 13:51:40 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"

#include "Modes.hpp"
#include "Server.hpp"
#include <iostream>
#include <cstdlib>   // atoi için
#include <sstream>

/**
 * MODE komutunu işler.
 * Örnek: MODE #kanal +i, MODE #kanal +k sifre, MODE #kanal +l 5, MODE #kanal +o Emirhan
 */
void Modes::processMode(Server& server, int client_fd, const std::string& channel, const std::string& modes, std::vector<std::string>& params){
    std::ostringstream joined;
    for (size_t i = 0; i < params.size(); ++i) {
        if (i != 0) joined << " ";
        joined << params[i];
    }

    std::cout << "🔍 DEBUG: processMode çağrıldı! Kanal: [" << channel << "], Modlar: [" << modes << "], Parametreler: [" << joined.str() << "]\n";


    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string& currentModes = server.getChannelModes()[channel];
    bool enable = true;
    std::string paramToken;
    size_t paramIndex = 0;
    char* endptr;
    long limit = 0;

    for (size_t i = 0; i < modes.size(); i++) {
        if (modes[i] == '+') {
            enable = true;
            continue;
        }
        if (modes[i] == '-') {
            enable = false;
            continue;
        }

        // Parametre gerektiren modlar: k, l, o
        if (modes[i] == 'k' || modes[i] == 'l' || modes[i] == 'o') {
            if (paramIndex >= params.size()) {
                std::string error_msg = ":ft_irc 461 MODE " + channel + " " + modes[i] + " :Parameter required\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                continue;
            }
            paramToken = params[paramIndex++];
        }

        switch (modes[i]) {
            case 'i':
                setInviteOnly(server, client_fd, channel, enable);
                break;
            case 'k':
                if (enable)
                    setKey(server, client_fd, channel, paramToken);
                else
                    setKey(server, client_fd, channel, "");
                break;
            case 'l':
                limit = std::strtol(paramToken.c_str(), &endptr, 10);
                if (*endptr != '\0' || limit <= 0) {
                    std::string error_msg = ":ft_irc 461 MODE " + channel + " +l :Invalid limit\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                    continue;
                }
                setLimit(server, client_fd, channel, limit);
                break;
            case 'o':
                setOperator(server, client_fd, channel, paramToken, enable);
                break;
            case 't':
                setTopicLock(server, client_fd, channel, enable);
                break;
            default:
                {
                    std::string error_msg = ":ft_irc 472 " + channel + " " + modes[i] + " :Unknown mode\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                    continue;
                }
        }

        // Kanal mod listesine ekle/çıkar
        if (enable) {
            if (currentModes.find(modes[i]) == std::string::npos)
                currentModes += modes[i];
        } else {
            size_t pos = currentModes.find(modes[i]);
            if (pos != std::string::npos)
                currentModes.erase(pos, 1);
        }
    }

    std::cout << "🔍 DEBUG: Güncellenmiş kanal modları: [" << currentModes << "]\n";
    std::string updatedModes = (currentModes.empty()) ? "+No modes set" : "+" + currentModes;
    std::string notify = ":ft_irc 324 " + channel + " " + updatedModes + "\r\n";
    send(client_fd, notify.c_str(), notify.size(), 0);
}

void Modes::getChannelModes(Server& server, int client_fd, const std::string& channel) {
    std::cout << "🔍 DEBUG: getChannelModes çağrıldı! Kanal: " << channel << std::endl;
    
    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string modes = server.getChannelModes()[channel];
    std::string response = ":ft_irc 324 " + channel + " +" + modes + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
    std::cout << "🔍 DEBUG: Kanal modları gönderildi: " << response << std::endl;
}
