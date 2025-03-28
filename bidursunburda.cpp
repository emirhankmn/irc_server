/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/16 22:39:13 by eakman            #+#    #+#             */
/*   Updated: 2025/03/16 22:39:13 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <map>
#include <sstream>
#include <vector>
#include <poll.h>

//Yapıcı fonksiyon (Constructor)
Server::Server(int port) : port(port), server_socket(-1) {}

// Yıkıcı fonksiyon (Destructor)
Server::~Server() {
    if (server_socket != -1) {
        close(server_socket); // Sunucu kapatılırken soketi serbest bırak
    }
}

// Sunucu başlatma fonksiyonu
void Server::init() {
    std::cout << "✅ Sunucu başlatılıyor..." << std::endl;

    // 1️⃣ TCP soketi oluştur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "❌ Hata: Soket oluşturulamadı!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2️⃣ Soket ayarlarını yap (SO_REUSEADDR)
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "❌ Hata: Soket ayarları yapılamadı!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 3️⃣ Sunucu adres bilgilerini ayarla
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Tüm IP'lerden bağlantı kabul et
    server_addr.sin_port = htons(port);       // Port numarasını belirle

    // 4️⃣ Soketi porta bağla
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "❌ Hata: Bağlama (bind) başarısız!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 5️⃣ Bağlantıları dinlemeye başla
    if (listen(server_socket, 10) == -1) { // 10, maksimum bağlantı kuyruğu uzunluğu
        std::cerr << "❌ Hata: Dinleme (listen) başarısız!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "✅ Sunucu başlatıldı! Port: " << port << std::endl;
}

// Sunucu çalıştırma fonksiyonu (poll() ile çoklu istemci desteği)
void Server::run() {
    std::cout << "🚀 Sunucu çalışıyor... Bağlantılar bekleniyor..." << std::endl;

    // Sunucu için pollfd oluştur
    struct pollfd server_pollfd;
    server_pollfd.fd = server_socket;
    server_pollfd.events = POLLIN;
    clients.push_back(server_pollfd);

    while (true) {
        if (poll(clients.data(), clients.size(), -1) == -1) {
            std::cerr << "❌ Hata: poll() başarısız!" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Yeni bağlantı var mı kontrol et
        if (clients[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
            
            if (client_socket == -1) {
                std::cerr << "❌ Hata: İstemci bağlantısı kabul edilemedi!" << std::endl;
            } else {
                std::cout << "✅ Yeni istemci bağlandı! (Socket FD: " << client_socket << ")" << std::endl;
                
                struct pollfd new_client;
                new_client.fd = client_socket;
                new_client.events = POLLIN;
                clients.push_back(new_client);

                std::string welcome_msg = ":ft_irc 001 Welcome to ft_irc Server!\r\n";
                send(client_socket, welcome_msg.c_str(), welcome_msg.size(), 0);
            }
        }

        // Mevcut istemcilerden gelen mesajları kontrol et
        for (size_t i = 1; i < clients.size(); i++) {
            if (clients[i].revents & POLLIN) {
                char buffer[512];
                memset(buffer, 0, sizeof(buffer));
                int bytes_received = recv(clients[i].fd, buffer, sizeof(buffer) - 1, 0);

                if (bytes_received <= 0) {
                    std::cout << "❌ İstemci bağlantıyı kapattı! (Socket FD: " << clients[i].fd << ")" << std::endl;
                    close(clients[i].fd);
                    clients.erase(clients.begin() + i);
                    i--;
                } else {
                    std::string message(buffer);
                    std::cout << "📩 İstemciden mesaj alındı: " << message << std::endl;
                    processMessage(clients[i].fd, message);
                }
            }
        }
    }
}

// Gelen mesajları işle
void Server::processMessage(int client_fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    if (command == "NICK") {
        std::string nickname;
        iss >> nickname;
        nicknames[client_fd] = nickname;
        std::string response = ":ft_irc 001 " + nickname + " :Nickname set successfully\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
    else if (command == "USER") {
        std::string username, mode, unused, realname;
        iss >> username >> mode >> unused;
        if (!std::getline(iss, realname) || realname.empty()) {
            std::string error_msg = ":ft_irc 400 USER :Missing parameters. Usage: USER <username> 0 * :<realname>\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        if (realname[0] == ':') {
            realname = realname.substr(1);
        }
        std::string response = ":ft_irc 002 " + username + " :User registered successfully\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
    else if (command == "JOIN") {
        std::string channel;
        iss >> channel;

        if (channel.empty() || channel[0] != '#') {
            std::string error_msg = ":ft_irc 400 JOIN :Invalid channel name. Usage: JOIN #channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        channels[channel].insert(client_fd); // Kullanıcıyı kanala ekle
        std::string response = ":" + nicknames[client_fd] + " JOIN " + channel + "\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
    else if (command == "PRIVMSG") {
        std::string channel, msg;
        iss >> channel;
        std::getline(iss, msg);
    
        if (msg.empty() || msg.find_first_not_of(" ") == std::string::npos) {
            std::string error_msg = ":ft_irc 400 PRIVMSG :Invalid message format. Usage: PRIVMSG #channel :message\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
    
        if (msg[0] != ':') {
            msg = ":" + msg;
        }
    
        msg = msg.substr(1);
    
        if (channels.find(channel) == channels.end()) {
            std::string error_msg = ":ft_irc 403 PRIVMSG :No such channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
    
        if (channels[channel].find(client_fd) == channels[channel].end()) {
            std::string error_msg = ":ft_irc 404 PRIVMSG :You are not in this channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
    
        sendToChannel(channel, nicknames[client_fd], msg, client_fd); // 🛠️ Yeni parametre eklendi
    }
    else if (command == "PART") {
        std::string channel;
        iss >> channel;
    
        if (channel.empty()) {
            std::string error_msg = ":ft_irc 461 PART :Not enough parameters\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
    
        partChannel(client_fd, channel);
    }
    else if (command == "QUIT") {
        std::string reason;
        std::getline(iss, reason);
    
        if (!reason.empty() && reason[0] == ':') {
            reason = reason.substr(1); // ':' karakterini kaldır
        } else {
            reason = "Client Quit";
        }
    
        quitClient(client_fd, reason);
    }
    
}    


void Server::sendToChannel(const std::string& channel, const std::string& sender, std::string message, int sender_fd) {
    // 🛠️ Eğer mesaj zaten ':' ile başlıyorsa, onu kaldır
    if (!message.empty() && message[0] == ':') {
        message = message.substr(1);
    }

    std::string full_message = ":" + sender + " PRIVMSG " + channel + " :" + message + "\r\n";

    for (std::set<int>::iterator it = channels[channel].begin(); it != channels[channel].end(); ++it) {
        if (*it != sender_fd) { // 🛠️ Mesajı gönderen kişiye tekrar gönderme
            send(*it, full_message.c_str(), full_message.size(), 0);
        }
    }
}

void Server::partChannel(int client_fd, const std::string& channel) {
    // 🛠️ Eğer kanal yoksa hata döndür
    if (channels.find(channel) == channels.end()) {
        std::string error_msg = ":ft_irc 403 PART :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 🛠️ Kullanıcı kanalda değilse hata döndür
    if (channels[channel].find(client_fd) == channels[channel].end()) {
        std::string error_msg = ":ft_irc 404 PART :You are not in this channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 🛠️ Kullanıcıyı kanaldan çıkar
    channels[channel].erase(client_fd);

    // 🛠️ Eğer kanal boş kaldıysa, tamamen sil
    if (channels[channel].empty()) {
        channels.erase(channel);
    }

    // 🛠️ Kanal üyelerine PART mesajı gönder
    std::string part_msg = ":" + nicknames[client_fd] + " PART " + channel + "\r\n";
    for (std::set<int>::iterator it = channels[channel].begin(); it != channels[channel].end(); ++it) {
        send(*it, part_msg.c_str(), part_msg.size(), 0);
    }

    // 🛠️ Kullanıcıya çıkış mesajı gönder
    send(client_fd, part_msg.c_str(), part_msg.size(), 0);
}

void Server::quitClient(int client_fd, const std::string& reason) {
    std::string nickname = nicknames[client_fd];

    // 🛠️ Kullanıcının tüm kanallardan çıkmasını sağla
    for (std::map<std::string, std::set<int> >::iterator it = channels.begin(); it != channels.end(); ) {
        std::string channel = it->first;
        std::set<int>& members = it->second;

        if (members.find(client_fd) != members.end()) {
            members.erase(client_fd);

            // 🛠️ Kanalda kimse kalmadıysa, kanalı tamamen sil
            if (members.empty()) {
                channels.erase(it++);
            } else {
                // 🛠️ Kanal üyelerine "X kullanıcı sunucudan ayrıldı" mesajını gönder
                std::string quit_msg = ":" + nickname + " QUIT :Quit (" + reason + ")\r\n";
                for (std::set<int>::iterator member = members.begin(); member != members.end(); ++member) {
                    send(*member, quit_msg.c_str(), quit_msg.size(), 0);
                }
                ++it;
            }
        } else {
            ++it;
        }
    }

    // 🛠️ Kullanıcının bağlantısını kapat
    close(client_fd);

    // 🛠️ Kullanıcıyı clients listesinden sil (C++98 uyumlu yöntem)
    for (std::vector<struct pollfd>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->fd == client_fd) {
            clients.erase(it);
            break;
        }
    }
    
    // 🛠️ Kullanıcıyı nick listesinden çıkar
    nicknames.erase(client_fd);

    std::cout << "🛑 Kullanıcı sunucudan ayrıldı: " << nickname << std::endl;
}
