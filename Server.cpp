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
#include "Commands.hpp"
#include <sstream>

Server::Server(int port, const std::string& password): port(port), server_socket(-1), password(password) {}

Server::~Server() {
    if (server_socket != -1) {
        close(server_socket);
    }
}

bool Server::isAuthorized(int fd) const { return authorizedClients.find(fd) != authorizedClients.end(); }
void Server::authorizeClient(int fd) { authorizedClients.insert(fd); }
const std::string& Server::getPassword() const { return password; }

// Sunucuyu başlatan fonksiyon
void Server::init() {
    std::cout << "✅ Sunucu başlatılıyor..." << std::endl;

    // 1️⃣ TCP soketi oluştur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "❌ Hata: Soket oluşturulamadı!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2️⃣ Soketin yeniden kullanılmasını sağla (SO_REUSEADDR)
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "❌ Hata: Soket ayarları yapılamadı!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 3️⃣ Sunucu adres bilgilerini ayarla
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Herhangi bir IP adresinden bağlanılabilir
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

// Sunucunun ana döngüsü
void Server::run() {
    std::cout << "🚀 Sunucu çalışıyor... Bağlantılar bekleniyor..." << std::endl;

    struct pollfd server_pollfd;
    server_pollfd.fd = server_socket;
    server_pollfd.events = POLLIN;
    clients.push_back(server_pollfd);

    while (true) {
        if (poll(clients.data(), clients.size(), -1) == -1) {
            std::cerr << "❌ Hata: poll() başarısız!" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (clients[0].revents & POLLIN) {
            acceptClient();
        }

        for (size_t i = 1; i < clients.size(); i++) {
            if (clients[i].revents & POLLIN) {
                receiveMessage(clients[i].fd);
            }
        }
    }
}

// Yeni istemci bağlantısını kabul et
void Server::acceptClient() {
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

        std::string welcome_msg = ":ft_irc 001 NewUser :Welcome to ft_irc Server!\r\n";
        send(client_socket, welcome_msg.c_str(), welcome_msg.size(), 0);
    }
}

// İstemciden gelen mesajı al ve işle
void Server::receiveMessage(int client_fd) {
    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        removeClient(client_fd);
    } else {
        std::string message(buffer);
        std::cout << "📩 İstemciden mesaj alındı: " << message << std::endl;
        Commands::processMessage(*this, client_fd, message);
    }
}

// İstemci bağlantısını kapat ve temizle
void Server::removeClient(int client_fd) {
    std::cout << "❌ İstemci bağlantıyı kapattı! (Socket FD: " << client_fd << ")" << std::endl;
    close(client_fd);

    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].fd == client_fd) {
            clients.erase(clients.begin() + i);
            break;
        }
    }

    nicknames.erase(client_fd);
}

// Kanal içindeki tüm istemcilere mesaj gönderen fonksiyon
void Server::sendToChannel(const std::string& channel, const std::string& sender, const std::string& message, int sender_fd, bool isCommand) {
    
    std::string full_message = ":" + sender + " PRIVMSG " + channel + " :" + message + "\r\n";

    if (channels.find(channel) == channels.end()) {
        std::cerr << "❌ Hata: Kanal bulunamadı!" << std::endl;
        return;
    }


    for (std::set<int>::iterator it = channels[channel].begin(); it != channels[channel].end(); ++it) {
        if (*it != sender_fd) { // Mesajı gönderen istemciye tekrar göndermiyoruz
            send(*it, full_message.c_str(), full_message.size(), 0);
            std::cout << "📨 Mesaj gönderildi: " << full_message << std::endl;
        }
        else if (isCommand && sender_fd == *it) {
            std::cout << "📨 Komut gönderildi: " << full_message << std::endl;
            send(*it, full_message.c_str(), full_message.size(), 0);
        }
        else {
            std::cout << "📨 Mesaj gönderilmedi: " << full_message << std::endl;
        }
    }
}

void Server::updateChannelMode(const std::string& channel, char mode, bool enable) {
    if (enable) {
        if (channelModes[channel].find(mode) == std::string::npos) {
            channelModes[channel] += mode; // Yeni modu ekle
        }
    } else {
        size_t pos = channelModes[channel].find(mode);
        if (pos != std::string::npos) {
            channelModes[channel].erase(pos, 1); // Modu kaldır
        }
    }
}

void Server::createChannel(const std::string& channelName, int client_fd) {
    channels[channelName].insert(client_fd);
    channelFounders[channelName] = client_fd;
}

int Server::getUserFdByNick(const std::string& nickname) {
    for (size_t i = 0; i < clients.size(); ++i) {
        int fd = clients[i].fd;
        if (nicknames.find(fd) != nicknames.end()) {
            if (strcasecmp(nicknames[fd].c_str(), nickname.c_str()) == 0)
                return fd;
        }
    }
    return -1;
}

std::vector<struct pollfd>& Server::getClients()                     { return clients; }
std::map<int, std::string>& Server::getNicknames()                   { return nicknames; }
std::map<std::string, int>& Server::getChannelLimits()               { return channelLimits; }
std::set<std::string>& Server::getInviteOnlyChannels()               { return inviteOnlyChannels; }
std::map<std::string, int>& Server::getChannelFounders()             { return channelFounders; }
std::map<std::string, std::set<int> >& Server::getChannels()         { return channels; }
std::map<std::string, std::string>& Server::getChannelKeys()         { return channelKeys; }
std::map<std::string, std::string>& Server::getChannelModes()        { return channelModes; }
std::map<std::string, std::string>& Server::getChannelTopics()       { return channelTopics; }
std::map<std::string, std::set<int> >& Server::getInvitedUsers()     { return invitedUsers; }
std::map<std::string, std::set<int> >& Server::getChannelOperators() { return channelOperators; }
