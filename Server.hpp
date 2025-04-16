/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/16 22:20:25 by eakman            #+#    #+#             */
/*   Updated: 2025/03/16 22:20:25 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>      // Standart giriş/çıkış işlemleri
#include <cstdlib>       // exit() fonksiyonu için
#include <cstring>       // memset() kullanımı için
#include <sys/socket.h>  // Soket işlemleri için
#include <netinet/in.h>  // sockaddr_in yapısı için
#include <unistd.h>      // close() fonksiyonu için
#include <vector>        // poll() ile bağlantıları yönetmek için
#include <poll.h>        // Çoklu istemci desteği için
#include <map>          // Kullanıcı ve kanal listelerini saklamak için
#include <set>          // Kanallardaki kullanıcıları saklamak için

class Server {
private:
    int port;                           // Sunucunun çalıştığı port
    int server_socket;                 // Sunucu soket dosya tanımlayıcısı (FD)
    std::string password;              // Sunucu şifresi
    std::set<int> authorizedClients;    // Yetkilendirilmiş istemcilerin FD'leri
    struct sockaddr_in server_addr;     // Sunucu adres yapısı
    std::vector<struct pollfd> clients; // Bağlı istemcileri takip eden liste
    std::map<int, std::string> nicknames; // Kullanıcıların takma adları (socket_fd -> nickname)
    std::map<std::string, std::set<int> > channels; // Kanallar ve içindeki kullanıcılar (kanal adı -> kullanıcı listesi)
    std::map<std::string, std::string> channelModes; // Kanal modlarını saklayan harita (kanal adı -> modlar)
    std::set<std::string> inviteOnlyChannels;         // `+i` modunda olan kanallar
    std::map<std::string, std::set<int> > invitedUsers; // Kanal bazında davet edilen kullanıcılar
    std::map<std::string, std::string> channelKeys;   // `+k` modunda olan kanallar ve şifreleri
    std::map<std::string, int> channelLimits;         // `+l` modundaki kullanıcı limiti
    std::map<std::string, std::set<int> > channelOperators; // Kanal operatörlerini saklayan harita (kanal adı -> kullanıcılar)
    std::map<std::string, int> channelFounders;  // Kanal adını founder ile eşleştirir
    std::map<std::string, std::string> channelTopics; // Kanal başlıklarını saklar (kanal adı -> başlık)

public:
    Server(int port, const std::string& password); // Yapıcı fonksiyon
    ~Server();         // Yıkıcı fonksiyon
    
    void init();          // Sunucu başlatma işlemi
    void run();           // Sunucu döngüsü (bağlantıları ve mesajları dinler)
    void acceptClient();  // Yeni istemci bağlantısını kabul eder
    void receiveMessage(int client_fd); // İstemciden gelen mesajları işler
    void removeClient(int client_fd);   // Bağlantıyı kesen istemciyi temizler

    void sendToChannel(const std::string& channel, const std::string& sender, const std::string& message, int sender_fd); // Kanal üyelerine mesaj yollar
    void partChannel(int client_fd, const std::string& channel);
    void quitClient(int client_fd, const std::string& reason);

    void updateChannelMode(const std::string& channel, char mode, bool enable); // Kanal modlarını günceller

    bool isAuthorized(int fd) const;
    void authorizeClient(int fd);
    const std::string& getPassword() const;
    std::map<std::string, std::string>& getChannelModes(); // Kanal modlarını döndürür
    // Getter fonksiyonları (Commands.cpp erişimi için)
    std::map<int, std::string>& getNicknames();
    std::map<std::string, std::set<int> >& getChannels();
    std::vector<struct pollfd>& getClients();
    // Getters for channel modes
    std::set<std::string>& getInviteOnlyChannels();
    std::map<std::string, std::set<int> >& getInvitedUsers();
    std::map<std::string, std::string>& getChannelKeys();
    std::map<std::string, int>& getChannelLimits();
    std::map<std::string, std::set<int> >& getChannelOperators();
    void createChannel(const std::string& channelName, int client_fd);
    std::map<std::string, int>& getChannelFounders();
    int getUserFdByNick(const std::string& nickname); // Kullanıcıyı takma adıyla bulur
    std::map<std::string, std::string>& getChannelTopics(); // Kanal başlıklarını döndürür
};

#endif


