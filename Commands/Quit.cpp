/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quite.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 00:14:58 by eakman            #+#    #+#             */
/*   Updated: 2025/04/15 00:14:58 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Quit.hpp"
#include "../Commands.hpp"

namespace Commands {

void quitCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string target, reason;
    iss >> target;
    std::getline(iss, reason);
    
    if (!reason.empty()) {
        reason = reason.substr(reason.find_first_not_of(" "));
    } else {
        reason = "Client Quit";
    }

    for (std::map<std::string, std::set<int> >::iterator it = server.getChannels().begin(); it != server.getChannels().end(); ) {
        std::string channel = it->first;
        std::set<int>& members = it->second;
        
        if (members.find(client_fd) != members.end()) {
            members.erase(client_fd);
            std::string quitMsg = ":" + server.getNicknames()[client_fd] + " QUIT :" + reason + "\r\n";
            server.sendToChannel(target, server.getNicknames()[client_fd], quitMsg, client_fd);
            std::string partMsg = ":" + server.getNicknames()[client_fd] + " PART " + channel + " :" + reason + "\r\n";
            send(client_fd, partMsg.c_str(), partMsg.size(), 0);
            //members.erase(client_fd);
        }

        if (members.empty()) {
            server.getChannels().erase(it++);
        } else {
            ++it;
        }
    }
    close(client_fd);
    server.removeClient(client_fd);
    std::cout << "❌ Client disconnected from server. FD: " << client_fd << std::endl;
}

}
 