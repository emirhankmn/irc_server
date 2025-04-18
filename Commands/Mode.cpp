/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 04:19:17 by eakman            #+#    #+#             */
/*   Updated: 2025/04/15 04:19:17 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Mode.hpp"
#include "../Commands.hpp"
#include "../Modes.hpp"

namespace Commands {

void modeCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string channel, modeStr, param;
    iss >> channel >> modeStr;

    if (!server.isAuthorized(client_fd)) {
        std::string msg = ":ft_irc 451 : If you register, you are blessed.\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }

    if (channel.empty() || channel[0] != '#') {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

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

    std::getline(iss, param);
    size_t start = param.find_first_not_of(" \t\r\n");
    param = (start != std::string::npos) ? param.substr(start) : "";

    if (modeStr.empty()) {
        Modes::getChannelModes(server, client_fd, channel);
        return;
    }

    if (modeStr == "+o" || modeStr == "-o") {
        bool enable = (modeStr == "+o");
        if (server.getChannelOperators()[channel].find(client_fd) == server.getChannelOperators()[channel].end()) {
            std::string error_msg = ":ft_irc 482 " + channel + " :You're not channel operator\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        if (param.empty()) {
            std::string error_msg = ":ft_irc 461 " + channel + " " + modeStr + " :Not enough parameters\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        Modes::setOperator(server, client_fd, channel, param, enable);
        return;
    }

    std::vector<std::string> parameters;
    std::string word;
    while (iss >> word)
        parameters.push_back(word);
    Modes::processMode(server, client_fd, channel, modeStr, parameters);

    }
}
