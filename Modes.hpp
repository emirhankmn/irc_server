/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Modes.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eakman <arcemirhanakman@gmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 16:54:32 by eakman            #+#    #+#             */
/*   Updated: 2025/03/24 16:54:32 by eakman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MODES_HPP
#define MODES_HPP

#include "Server.hpp"
#include <string>
#include <sstream>

/**
 * @brief Modes sınıfı, MODE komutlarını işlemek için kullanılır.
 */
class Modes {
public:
    /**
     * @brief MODE komutunu işler.
     * 
     * @param server Sunucu nesnesi
     * @param client_fd İstemci soket dosya tanımlayıcısı
     * @param channel Kanal adı (örneğin, "#kanal")
     * @param modes Mod karakterleri (örneğin, "+i" veya "-t")
     * @param param Opsiyonel parametre (örneğin, şifre, limit, operatör kullanıcı adı)
     */
    static void processMode(Server& server, int client_fd, const std::string& channel, std::string modes, const std::string& param);

    // Aşağıdaki fonksiyonlar, ilgili kanal modlarını ayarlamak için kullanılır.
    /**
     * @brief Kanalı sadece davetle girişe açar veya kapatır.
     * 
     * @param server Sunucu nesnesi
     * @param client_fd İstemci soket dosya tanımlayıcısı
     * @param channel Kanal adı
     * @param enable Eğer true ise invite-only açılır, false ise kapatılır.
     */
    static void setInviteOnly(Server& server, int client_fd, const std::string& channel, bool enable);

    /**
     * @brief Kanal başlığının değiştirilmesini sadece operatörlere sınırlar.
     * 
     * @param server Sunucu nesnesi
     * @param client_fd İstemci soket dosya tanımlayıcısı
     * @param channel Kanal adı
     * @param enable Eğer true ise topic lock aktif, false ise kapatılır.
     */
    static void setTopicLock(Server& server, int client_fd, const std::string& channel, bool enable);

    /**
     * @brief Kanal için şifre ayarlar veya kaldırır.
     * 
     * @param server Sunucu nesnesi
     * @param client_fd İstemci soket dosya tanımlayıcısı
     * @param channel Kanal adı
     * @param key Belirlenen şifre; boş ise şifre kaldırılır.
     */
    static void setKey(Server& server, int client_fd, const std::string& channel, const std::string& key);

    /**
     * @brief Kanalda maksimum üye sınırını belirler.
     * 
     * @param server Sunucu nesnesi
     * @param client_fd İstemci soket dosya tanımlayıcısı
     * @param channel Kanal adı
     * @param limit Maksimum üye sayısı (pozitif bir sayı); 0 veya negatif ise sınır kaldırılır.
     */
    static void setLimit(Server& server, int client_fd, const std::string& channel, int limit);

    /**
     * @brief Kullanıcıya kanal operatörlüğü verir veya alır.
     * 
     * @param server Sunucu nesnesi
     * @param client_fd İstemci soket dosya tanımlayıcısı
     * @param channel Kanal adı
     * @param user Hedef kullanıcı adı
     * @param enable Eğer true ise operatör yetkisi verilir, false ise alınır.
     */
    static void setOperator(Server& server, int client_fd, const std::string& channel, const std::string& user, bool enable);
    static void getChannelModes(Server& server, int client_fd, const std::string& channel); // Kanal modlarını gösterir
};

#endif

