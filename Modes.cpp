#include "Commands.hpp"
#include "Modes.hpp"
#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <sstream>

namespace {

void Modes::processMode(Server& server, int client_fd, const std::string& channel,
                        const std::string& modes, std::vector<std::string>& params) 
    std::ostringstream joined;
    for (size_t i = 0; i < params.size(); ++i) {
        if (i != 0) joined << " ";
        joined << params[i];
    }

    std::cout << "🔍 DEBUG: processMode called. Channel: [" << channel
              << "], Modes: [" << modes << "], Parameters: [" << joined.str() << "]\n";

    if (channel.empty() || channel[0] != '#') {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel (must start with '#')\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string& currentModes = server.getChannelModes()[channel];
    bool enable = true;
    size_t paramIndex = 0;
    char* endptr;

    for (size_t i = 0; i < modes.size(); ++i) {
        char mode = modes[i];

        if (mode == '+') {
            enable = true;
            continue;
        } else if (mode == '-') {
            enable = false;
            continue;
        }

        std::string paramToken = "";
        if (c == 'k' || c == 'l' || c == 'o') {
            if (paramIndex >= params.size()) {
                std::string error_msg = ":ft_irc 461 MODE " + channel + " " + mode + " :Parameter required\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                continue;
            }
            paramToken = params[paramIndex++];
        }

        switch (mode) {
            case 'i':
                setInviteOnly(server, client_fd, channel, enable);
                break;
            case 'k':
                setKey(server, client_fd, channel, enable ? paramToken : "");
                break;
            case 'l':
                {
                    long limit = std::strtol(paramToken.c_str(), &endptr, 10);
                    if (*endptr != '\0' || limit <= 0) {
                        std::string error_msg = ":ft_irc 461 MODE " + channel + " +l :Invalid limit\r\n";
                        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                        continue;
                    }
                    setLimit(server, client_fd, channel, static_cast<int>(limit));
                }
                break;
            case 'o':
                setOperator(server, client_fd, channel, paramToken, enable);
                break;
            case 't':
                setTopicLock(server, client_fd, channel, enable);
                break;
            default:
                {
                    std::string error_msg = ":ft_irc 472 " + channel + " " + mode + " :Unknown mode\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                }
                break;
        }

        if (enable) {
            if (currentModes.find(mode) == std::string::npos)
                currentModes += mode;
        } else {
            size_t pos = currentModes.find(mode);
            if (pos != std::string::npos)
                currentModes.erase(pos, 1);
        }
    }

    std::cout << "🔍 DEBUG: Updated channel modes: [" << currentModes << "]\n";
    std::string updatedModes = currentModes.empty() ? "+No modes set" : "+" + currentModes;
    std::string notify = ":ft_irc 324 " + channel + " " + updatedModes + "\r\n";
    send(client_fd, notify.c_str(), notify.size(), 0);
}

void Modes::getChannelModes(Server& server, int client_fd, const std::string& channel) {
    std::cout << "🔍 DEBUG: getChannelModes called for channel: " << channel << std::endl;

    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string modes = server.getChannelModes()[channel];
    std::string response = ":ft_irc 324 " + channel + " +" + modes + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
    std::cout << "🔍 DEBUG: Channel modes sent: " << response << std::endl;
}
