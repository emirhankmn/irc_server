/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 03:12:44 by eakman            #+#    #+#             */
/*   Updated: 2025/03/24 03:12:44 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Server.hpp"
#include <string>
#include <sstream>

class Commands {
public:
    static void processMessage(Server& server, int client_fd, const std::string& message);
    
    // IRC command handlers
    static void nickCommand(Server& server, int client_fd, std::istringstream& iss);
    static void userCommand(Server& server, int client_fd, std::istringstream& iss);
    static void joinCommand(Server& server, int client_fd, std::istringstream& iss);
    static void privmsgCommand(Server& server, int client_fd, std::istringstream& iss);
    static void partCommand(Server& server, int client_fd, std::istringstream& iss);
    static void quitCommand(Server& server, int client_fd, std::istringstream& iss);
    static void modeCommand(Server& server, int client_fd, std::istringstream& iss);
    static void handleFounderExit(Server& server, const std::string& channel, int client_fd);
};

#endif
