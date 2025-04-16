/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 13:51:10 by eakman            #+#    #+#             */
/*   Updated: 2025/04/11 13:51:10 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Modes.hpp"
#include <signal.h>
#include <iostream>
#include <sstream>
#include <cctype>

// Yardımcı: Komutları büyük harfe çeviren fonksiyon
static std::string toUpper(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i)
        result[i] = std::toupper(result[i]);
    return result;
}

// Gelen mesajı işleyerek ilgili komut fonksiyonuna yönlendirir
void Commands::processMessage(Server& server, int client_fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command;
    iss >> command;
    command = toUpper(command);

    std::cout << "🔍 DEBUG: processMessage çağrıldı! Komut: " << command << std::endl;
    if (command == "PASS") {
        passCommand(server, client_fd, iss);
        return;
    }
    else if (command == "NICK")   nickCommand(server, client_fd, iss);
    else if (command == "USER")   userCommand(server, client_fd, iss);
    else if (command == "JOIN")   joinCommand(server, client_fd, iss);
    else if (command == "PART")   partCommand(server, client_fd, iss);
    else if (command == "QUIT")   quitCommand(server, client_fd, iss);
    else if (command == "MODE")   modeCommand(server, client_fd, iss);
    else if (command == "KICK")   kickCommand(server, client_fd, iss);
    else if (command == "TOPIC")  topicCommand(server, client_fd, iss);
    else if (command == "INVITE") inviteCommand(server, client_fd, iss);
    else if (command == "PRIVMSG") privmsgCommand(server, client_fd, iss);
}
