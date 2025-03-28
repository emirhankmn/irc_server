/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Modes.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 00:15:14 by eakman            #+#    #+#             */
/*   Updated: 2025/03/25 00:15:14 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Modes.hpp"
#include "Server.hpp"
#include <iostream>
#include <cstdlib>  // atoi için
#include <sstream>

/**
 * @brief MODE komutunu işler.
 * Kanal adı, mod karakterleri ve opsiyonel parametre alınır.
 * Örnek kullanım: MODE #kanal +i, MODE #kanal +k sifre, MODE #kanal +l 5, MODE #kanal +o Emirhan
 */
void Modes::processMode(Server& server, int client_fd, const std::string& channel, std::string modes, const std::string& param) {
    std::cout << "🔍 DEBUG: processMode çağrıldı! Kanal: [" << channel << "], Modlar: [" << modes << "], Parametre: [" << param << "]\n";

    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string& currentModes = server.getChannelModes()[channel];
    bool enable = true;
    std::string response = ":ft_irc MODE " + channel + " ";

    std::istringstream paramStream(param);
    std::string paramToken;

    // ❗ DEĞİŞKENLERİ BAŞTA TANIMLIYORUZ!
    char *endptr;
    long limit = 0; // Varsayılan değer 0
    
    for (size_t i = 0; i < modes.size(); i++) {
        if (modes[i] == '+') { 
            enable = true;
            continue;  
        }
        if (modes[i] == '-') { 
            enable = false;
            continue;  
        }

        // Parametre gerektiren modlar için sıradaki parametreyi al
        if (modes[i] == 'k' || modes[i] == 'l') {
            if (!(paramStream >> paramToken)) {
                std::string error_msg = ":ft_irc 461 MODE " + channel + " " + modes[i] + " :Parameter required\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                continue;
            }
        }

        switch (modes[i]) {
            case 'i':  
                setInviteOnly(server, client_fd, channel, enable);
                break;

            case 'k':  
                setKey(server, client_fd, channel, enable ? paramToken : "");
                break;

            case 'l':  
                limit = std::strtol(paramToken.c_str(), &endptr, 10);
                if (*endptr != '\0' || limit <= 0) {
                    std::string error_msg = ":ft_irc 461 MODE " + channel + " +l :Invalid limit\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                    continue;
                }
                setLimit(server, client_fd, channel, limit);
                break;

            default:
                std::string error_msg = ":ft_irc 472 " + channel + " " + modes[i] + " :Unknown mode\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                continue;
        }

        if (enable) {
            if (currentModes.find(modes[i]) == std::string::npos)
                currentModes += modes[i];
        } else {
            size_t pos = currentModes.find(modes[i]);
            if (pos != std::string::npos)
                currentModes.erase(pos, 1);
        }
    }

    std::cout << "🔍 DEBUG: Güncellenmiş kanal modları: [" << currentModes << "]\n";

    std::string updatedModes = (currentModes.empty()) ? "+No modes set" : "+" + currentModes;
    std::string notify = ":ft_irc 324 " + channel + " " + updatedModes + "\r\n";
    send(client_fd, notify.c_str(), notify.size(), 0);
}

/**
 * @brief Kanal başlığını sadece operatörlerin değiştirebilmesini sağlar.
 */
void Modes::setTopicLock(Server& server, int client_fd, const std::string& channel, bool enable) {
    std::cout << "🔍 DEBUG: setTopicLock çağrıldı! Kanal: " << channel << " Enable: " << (enable ? "+t" : "-t") << std::endl;
    server.updateChannelMode(channel, 't', enable); // Kanal modlarını güncelle
    std::string response = ":ft_irc MODE " + channel + " " + (enable ? "+t" : "-t") + "\r\n";
    server.sendToChannel(channel, "server", response, client_fd);
}

/**
 * @brief Kanalı sadece davetle girişe açar veya kapatır.
 */
void Modes::setInviteOnly(Server& server, int client_fd, const std::string& channel, bool enable) {
    std::cout << "🔍 DEBUG: setInviteOnly çağrıldı! Kanal: " << channel 
              << " Enable: " << (enable ? "+i" : "-i") << "\n";
    
    std::string& currentModes = server.getChannelModes()[channel];
    
    if (enable) {
        if (currentModes.find('i') == std::string::npos)
            currentModes += 'i';
    } else {
        size_t pos = currentModes.find('i');
        if (pos != std::string::npos)
            currentModes.erase(pos, 1);
    }

    // Kanalın "invite-only" olup olmadığını tutan map güncellemesi
    if (enable)
        server.getInviteOnlyChannels().insert(channel);
    else
        server.getInviteOnlyChannels().erase(channel);

    // İstemciye MODE değişikliği bildirimi gönder
    std::string response = ":ft_irc MODE " + channel + (enable ? " +i" : " -i") + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

/**
 * @brief Kanal için şifre ayarlar veya kaldırır.
 */
void Modes::setKey(Server& server, int client_fd, const std::string& channel, const std::string& key) {
    std::cout << "🔍 DEBUG: setKey çağrıldı! Kanal: " << channel << " Şifre: " << key << "\n";
    
    if (!key.empty())
        server.getChannelKeys()[channel] = key; // Şifreyi ayarla
    else
        server.getChannelKeys().erase(channel); // Şifreyi kaldır

    // İstemciye MODE değişikliği bildirimi gönder
    std::string response;
    if (!key.empty())
        response = ":ft_irc MODE " + channel + " +k " + key + "\r\n";
    else
        response = ":ft_irc MODE " + channel + " -k\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}


/**
 * @brief Kanalın maksimum üye sınırını belirler.
 */
void Modes::setLimit(Server& server, int client_fd, const std::string& channel, int limit) {
    if (limit <= 0) {
        std::string error_msg = ":ft_irc 461 " + channel + " :Limit parameter missing or invalid\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::cout << "🔍 DEBUG: setLimit çağrıldı! Kanal: " << channel << " Limit: " << limit << std::endl;
    server.getChannelLimits()[channel] = limit; // Limiti ayarla
    server.updateChannelMode(channel, 'l', true);
    
    std::ostringstream oss;
    oss << limit;
    std::string response = ":ft_irc MODE " + channel + " +l " + oss.str() + "\r\n";
    server.sendToChannel(channel, "server", response, client_fd);
}

/**
 * @brief Kullanıcıya kanal operatörlüğü verir veya kaldırır.
 */
void Modes::setOperator(Server& server, int client_fd, const std::string& channel, const std::string& targetNick, bool enable) {
    std::cout << "🔍 DEBUG: setOperator çağrıldı! Kanal: " << channel << ", Kullanıcı: " << targetNick 
              << ", Enable: " << (enable ? "true" : "false") << std::endl;

    // Kanal var mı?
    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Komutu gönderen kullanıcı kanalın içinde mi?
    if (server.getChannels()[channel].find(client_fd) == server.getChannels()[channel].end()) {
        std::string error_msg = ":ft_irc 442 " + channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Komutu gönderen kullanıcı kanal operatörü mü?
    if (server.getChannelOperators()[channel].find(client_fd) == server.getChannelOperators()[channel].end()) {
        std::string error_msg = ":ft_irc 482 " + channel + " :You're not channel operator\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Hedef kullanıcı kanalın içinde mi?
    int target_fd = server.getUserFdByNick(targetNick);
    if (target_fd == -1 || server.getChannels()[channel].find(target_fd) == server.getChannels()[channel].end()) {
        std::string error_msg = ":ft_irc 441 " + targetNick + " " + channel + " :They aren't on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (enable) {
        // Kullanıcı zaten operatör mü?
        if (server.getChannelOperators()[channel].find(target_fd) != server.getChannelOperators()[channel].end()) {
            std::string error_msg = ":ft_irc 482 " + channel + " :User is already an operator\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        // Hedef kullanıcıyı operatör olarak ekle
        server.getChannelOperators()[channel].insert(target_fd);
        std::string success_msg = ":ft_irc MODE " + channel + " +o " + targetNick + "\r\n";
        send(client_fd, success_msg.c_str(), success_msg.size(), 0);
    } else {
        // Kullanıcı zaten operatör değilse
        if (server.getChannelOperators()[channel].find(target_fd) == server.getChannelOperators()[channel].end()) {
            std::string error_msg = ":ft_irc 482 " + channel + " :User is not an operator\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        // Hedef kullanıcıyı operatörlükten çıkar
        server.getChannelOperators()[channel].erase(target_fd);
        std::string success_msg = ":ft_irc MODE " + channel + " -o " + targetNick + "\r\n";
        send(client_fd, success_msg.c_str(), success_msg.size(), 0);
    }
}

/**
 * @brief Kanal modlarını gösterir.
 */
void Modes::getChannelModes(Server& server, int client_fd, const std::string& channel) {
    std::cout << "🔍 DEBUG: getChannelModes çağrıldı! Kanal: " << channel << std::endl;
    
    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string modes = server.getChannelModes()[channel];
    std::string response = ":ft_irc 324 " + channel + " +" + modes + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
    std::cout << "🔍 DEBUG: Kanal modları gönderildi: " << response << std::endl;
}

