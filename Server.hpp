/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/16 22:20:25 by eakman            #+#    #+#             */
/*   Updated: 2025/03/16 22:20:25 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <cstdlib>
# include <cstring>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <vector>
# include <poll.h>
# include <map>
# include <set>

class Server {
private:
    // 🔌 Temel Ağ Parametreleri
    int                         port;
    int                         server_socket;
    struct sockaddr_in          server_addr;
    std::string                 password;

    // 🧍 İstemciler
    std::set<int>               authorizedClients;
    std::vector<struct pollfd>  clients;
    std::map<int, std::string>  nicknames;

    // 📺 Kanal Yönetimi
    std::map<std::string, std::set<int> >    channels;
    std::map<std::string, std::string>       channelModes;
    std::map<std::string, std::string>       channelKeys;
    std::map<std::string, int>               channelLimits;
    std::map<std::string, std::set<int> >    channelOperators;
    std::map<std::string, int>               channelFounders;
    std::map<std::string, std::string>       channelTopics;

    // ➕ Kanal Modları
    std::set<std::string>                    inviteOnlyChannels;
    std::map<std::string, std::set<int> >    invitedUsers;

public:
    // 🚀 Kurucu / Yıkıcı
    Server(int port, const std::string& password);
    ~Server();

    // ⚙️ Başlatma ve Döngü
    void init();
    void run();
    void acceptClient();
    void receiveMessage(int client_fd);
    void removeClient(int client_fd);

    // 🔄 İstemci ve Kanal Etkileşimi
    void sendToChannel(const std::string& channel, const std::string& sender,
                       const std::string& message, int sender_fd);
    void partChannel(int client_fd, const std::string& channel);
    void quitClient(int client_fd, const std::string& reason);

    // ⚙️ Kanal Modları
    void updateChannelMode(const std::string& channel, char mode, bool enable);

    // ✅ Yetkilendirme
    bool                isAuthorized(int fd) const;
    void                authorizeClient(int fd);
    const std::string&  getPassword() const;

    // 🔎 Getters (Erişim için)
    std::vector<struct pollfd>&             getClients();
    std::map<int, std::string>&             getNicknames();
    std::map<std::string, std::set<int> >&  getChannels();
    std::map<std::string, std::string>&     getChannelModes();
    std::set<std::string>&                  getInviteOnlyChannels();
    std::map<std::string, std::set<int> >&  getInvitedUsers();
    std::map<std::string, std::string>&     getChannelKeys();
    std::map<std::string, int>&             getChannelLimits();
    std::map<std::string, std::set<int> >&  getChannelOperators();
    std::map<std::string, int>&             getChannelFounders();
    std::map<std::string, std::string>&     getChannelTopics();

    // 🧩 Yardımcılar
    int  getUserFdByNick(const std::string& nickname);
    void createChannel(const std::string& channelName, int client_fd);
};

#endif // SERVER_HPP
