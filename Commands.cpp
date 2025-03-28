/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/23 21:20:14 by eakman            #+#    #+#             */
/*   Updated: 2025/03/23 21:20:14 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Modes.hpp"
#include <signal.h>

// 📌 Gelen mesajı işleyerek ilgili komut fonksiyonuna yönlendirir
void Commands::processMessage(Server& server, int client_fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    std::cout << "🔍 DEBUG: processMessage çağrıldı! Komut: " << command << std::endl;

    if (command == "NICK")        nickCommand(server, client_fd, iss);
    else if (command == "USER")   userCommand(server, client_fd, iss);
    else if (command == "JOIN")   joinCommand(server, client_fd, iss);
    else if (command == "PRIVMSG") privmsgCommand(server, client_fd, iss);
    else if (command == "PART")   partCommand(server, client_fd, iss);
    else if (command == "QUIT")   quitCommand(server, client_fd, iss);
    else if (command == "MODE")   modeCommand(server, client_fd, iss);  // 📌 Burada `MODE` çağrısını kontrol et!
}

// 📌 `NICK` Komutu - Kullanıcının takma adını ayarlar
void Commands::nickCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string nickname;
    iss >> nickname;

    if (nickname.empty()) {
        std::string error_msg = ":ft_irc 431 * :No nickname given\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 📌 Kullanici bir baska kullanicinin takma adinimi kullanmaya çalışıyor?
    for (size_t i = 0; i < server.getClients().size(); ++i) { // Use size_t for the loop variable
        if (server.getNicknames()[server.getClients()[i].fd] == nickname) {
            std::string error_msg = ":ft_irc 433 " + nickname + " :Nickname is already in use\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
    }

    // 📌 Kullanıcıya takma adi verildi
    server.getNicknames()[client_fd] = nickname;
    std::string response = ":ft_irc 001 " + nickname + " :Nickname set successfully\r\n";
    send(client_fd, response.c_str(), response.size(), 0);

    std::cout << "✅ Kullanıcı takma adı ayarlandı: " << nickname << std::endl;
}

// 📌 `USER` Komutu - Kullanıcının bilgilerini kaydeder
void Commands::userCommand(Server& /*server*/, int client_fd, std::istringstream& iss) {
    std::string username, mode, unused, realname;
    iss >> username >> mode >> unused;
    std::getline(iss, realname);

    if (realname.empty() || realname[0] != ':') {
        std::string error_msg = ":ft_irc 461 USER :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    std::string response = ":ft_irc 002 " + username + " :User registered successfully\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

// 📌 `JOIN` Komutu - Kullanıcının bir kanala katılmasını sağlar
void Commands::joinCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string channel, key;
    iss >> channel >> key;  // Kanal ve opsiyonel şifreyi al

    if (channel.empty() || channel[0] != '#') {
        std::string error_msg = ":ft_irc 400 JOIN :Invalid channel name. Usage: JOIN #channel [key]\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 🔍 Kanal mevcut değilse yeni bir kanal oluştur
    bool isNewChannel = false;
    if (server.getChannels().find(channel) == server.getChannels().end()) {
        isNewChannel = true;
        server.getChannels()[channel].insert(client_fd);
        server.getChannelModes()[channel] = ""; // Varsayılan modları ekleyelim
        std::cout << "🆕 Yeni kanal oluşturuldu: " << channel << "\n";
    } else {
        // 📌 `+i` (invite-only) kontrolü
        if (server.getInviteOnlyChannels().find(channel) != server.getInviteOnlyChannels().end() &&
            server.getInvitedUsers()[channel].find(client_fd) == server.getInvitedUsers()[channel].end()) {
            std::string error_msg = ":ft_irc 473 " + channel + " :You must be invited to join this channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        // 📌 `+k` (kanal şifresi) kontrolü
        if (server.getChannelKeys().find(channel) != server.getChannelKeys().end()) {
            if (server.getChannelKeys()[channel] != key) {
                std::string error_msg = ":ft_irc 475 " + channel + " :Incorrect channel key\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                return;
            }
        }

        // 📌 `+l` (kanal limiti) kontrolü (Limit varsa ve aşılmışsa)
        if (server.getChannelLimits().find(channel) != server.getChannelLimits().end()) {
            int limit = server.getChannelLimits()[channel];
            if (server.getChannels()[channel].size() >= static_cast<size_t>(limit)) {
                std::string error_msg = ":ft_irc 471 " + channel + " :Cannot join channel (+l limit reached)\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                return;
            }
        }
        // 🚀 Kullanıcıyı kanala ekleyelim
        server.getChannels()[channel].insert(client_fd);
    }

    // ✅ Kullanıcıyı başarıyla eklediğimizde JOIN mesajını gönderelim
    std::string response = ":" + server.getNicknames()[client_fd] + " JOIN " + channel + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);

    // 📝 Eğer yeni bir kanal oluşturulmuşsa, kullanıcıyı operatör (`+o`) yapalım
    if (isNewChannel) {
        server.getChannelOperators()[channel].insert(client_fd);
        std::string opResponse = ":ft_irc MODE " + channel + " +o " + server.getNicknames()[client_fd] + "\r\n";
        send(client_fd, opResponse.c_str(), opResponse.size(), 0);
    }
}

// 📌 `PRIVMSG` Komutu - Mesajı kanaldaki tüm üyelere gönderir
void Commands::privmsgCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string channel, msg;
    iss >> channel;
    std::getline(iss, msg);

    if (msg.empty() || msg.find_first_not_of(" ") == std::string::npos) {
        std::string error_msg = ":ft_irc 400 PRIVMSG :Invalid message format. Usage: PRIVMSG #channel :message\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    msg = msg.substr(1);

    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 PRIVMSG :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (server.getChannels()[channel].find(client_fd) == server.getChannels()[channel].end()) {
        std::string error_msg = ":ft_irc 404 PRIVMSG :You are not in this channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    server.sendToChannel(channel, server.getNicknames()[client_fd], msg, client_fd);
}

// 📌 `PART` Komutu - Kullanıcının bir kanaldan ayrılmasını sağlar
void Commands::partCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string channel;
    iss >> channel;

    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (server.getChannels()[channel].find(client_fd) == server.getChannels()[channel].end()) {
        std::string error_msg = ":ft_irc 442 " + channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 📌 Founder çıkarsa yeni founder belirle
    if (server.getChannelFounders()[channel] == client_fd) {
        if (!server.getChannelOperators()[channel].empty()) {
            int new_founder = *server.getChannelOperators()[channel].begin(); // İlk operatörü founder yap
            server.getChannelFounders()[channel] = new_founder;
            std::cout << "👑 Founder değişti: " << new_founder << " artık founder!" << std::endl;
        } else {
            server.getChannelFounders().erase(channel); // Eğer hiç operatör yoksa founder sıfırlanır
            std::cout << "❌ Founder kaldırıldı, kanal sahipsiz kaldı." << std::endl;
        }
    }

    // 📌 Kullanıcıyı kanaldan çıkar
    server.getChannels()[channel].erase(client_fd);
    std::string partMessage = ":" + server.getNicknames()[client_fd] + " PART " + channel + "\r\n";
    send(client_fd, partMessage.c_str(), partMessage.size(), 0);

    // Eğer kanalda kimse kalmadıysa kanalı sil
    if (server.getChannels()[channel].empty()) {
        server.getChannels().erase(channel);
        server.getChannelFounders().erase(channel);
        server.getChannelOperators().erase(channel);
    }
    std::cout << "👋 Kullanıcı kanaldan ayrıldı: " << channel << std::endl;
}

// 📌 `QUIT` Komutu - Kullanıcının sunucudan çıkmasını sağlar
void Commands::quitCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string reason;
    std::getline(iss, reason);
    
    if (!reason.empty()) {
        reason = reason.substr(reason.find_first_not_of(" ")); // Boşlukları temizle
    } else {
        reason = "Client Quit"; // Eğer sebep verilmezse default olarak atanır
    }

    // 📌 Kullanıcının bulunduğu tüm kanallardan çıkmasını sağla
    for (std::map<std::string, std::set<int> >::iterator it = server.getChannels().begin(); it != server.getChannels().end(); ++it) {
        std::string channel = it->first;
        if (it->second.find(client_fd) != it->second.end()) {
            it->second.erase(client_fd);
            std::string partMessage = ":" + server.getNicknames()[client_fd] + " PART " + channel + " :" + reason + "\r\n";
            send(client_fd, partMessage.c_str(), partMessage.size(), 0);
            
            // Eğer kanal boş kaldıysa tamamen sil
            if (it->second.empty()) {
                server.getChannels().erase(channel);
                server.getChannelFounders().erase(channel);
                server.getChannelOperators().erase(channel);
            }
        }
    }

    // 📌 Kullanıcıyı sunucudan kaldır
    close(client_fd);
    server.removeClient(client_fd);
}


void Commands::modeCommand(Server& server, int client_fd, std::istringstream& iss) {
    std::string channel, modeStr, param;
    iss >> channel >> modeStr;
    
    // 🚨 Kanal ismi kontrolü
    if (channel.empty() || channel[0] != '#') {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 🚨 Kanal mevcut mu?
    if (server.getChannels().find(channel) == server.getChannels().end()) {
        std::string error_msg = ":ft_irc 403 " + channel + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 🚨 Kullanıcı kanalın içinde mi?
    if (server.getChannels()[channel].find(client_fd) == server.getChannels()[channel].end()) {
        std::string error_msg = ":ft_irc 442 " + channel + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // 🎯 Mod belirtilmemişse mevcut modları göster
    if (modeStr.empty()) {
        Modes::getChannelModes(server, client_fd, channel);
        return;
    }

    // 🎯 Parametreyi al (boşlukları temizle)
    std::getline(iss, param);
    if (!param.empty()) {
        param = param.substr(param.find_first_not_of(" "));
    }

    std::cout << "🔍 DEBUG: modeCommand çağrıldı! Kanal: [" << channel << "], Modlar: [" 
              << modeStr << "], Parametre: [" << param << "]" << std::endl;

    // 🚨 Operatör (`+o` veya `-o`) işlemleri
    if (modeStr == "+o" || modeStr == "-o") {
        // ✅ Komutu gönderen kişi operatör mü?
        if (server.getChannelOperators()[channel].find(client_fd) == server.getChannelOperators()[channel].end()) {
            std::string error_msg = ":ft_irc 482 " + channel + " :You're not channel operator\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        // ✅ Hedef kullanıcı ismi var mı?
        if (param.empty()) {
            std::string error_msg = ":ft_irc 461 " + channel + " " + modeStr + " :Not enough parameters\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        // ✅ Hedef kullanıcıyı bul
        int target_fd = server.getUserFdByNick(param);
        if (target_fd == -1 || server.getChannels()[channel].find(target_fd) == server.getChannels()[channel].end()) {
            std::string error_msg = ":ft_irc 441 " + param + " " + channel + " :They aren't on that channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        // ✅ Kullanıcıya operatör ekle veya çıkar
        if (modeStr == "+o") {
            server.getChannelOperators()[channel].insert(target_fd);
        } else {  // `-o`
            server.getChannelOperators()[channel].erase(target_fd);
        }

        std::string success_msg = ":ft_irc MODE " + channel + " " + modeStr + " " + param + "\r\n";
        send(client_fd, success_msg.c_str(), success_msg.size(), 0);
        return;
    }

    // 🚀 Genel mod işleme fonksiyonunu çağır
    Modes::processMode(server, client_fd, channel, modeStr, param);
}

void Commands::handleFounderExit(Server& server, const std::string& channel, int client_fd) {
    if (server.getChannelFounders()[channel] == client_fd) { // Eğer çıkan kişi founder ise
        if (!server.getChannelOperators()[channel].empty()) {
            int new_founder = *server.getChannelOperators()[channel].begin(); // İlk operatörü seç
            server.getChannelFounders()[channel] = new_founder;
            std::cout << "Founder değiştirildi: " << new_founder << std::endl;
        } else {
            server.getChannelFounders().erase(channel); // Operatör yoksa founder sıfırlanır
            std::cout << "Founder kaldırıldı, kanal sahipsiz kaldı." << std::endl;
        }
    }
}

