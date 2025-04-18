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
    std::string reason;
    std::getline(iss, reason);

    if (!server.isAuthorized(client_fd)) {
        std::string msg = ":ft_irc 462 :You not register\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }
    
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
            std::string partMsg = ":" + server.getNicknames()[client_fd] + " PART " + channel + " :" + reason + "\r\n";
            send(client_fd, partMsg.c_str(), partMsg.size(), 0);
        }

        if (members.empty()) {
            server.getChannels().erase(it++);
        } else {
            ++it;
        }
    }

    close(client_fd);
    server.removeClient(client_fd);
    std::cout << "❌ Kullanıcı sunucudan çıktı: FD " << client_fd << std::endl;
}

}
 