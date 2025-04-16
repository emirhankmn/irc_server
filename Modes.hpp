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

#include "Modes/Invite.hpp"
#include "Modes/Key.hpp"
#include "Modes/Limit.hpp"
#include "Modes/Topic.hpp"
#include "Modes/Op.hpp"

namespace Modes {
    void processMode(Server& server, int client_fd, const std::string& channel, const std::string& modes, std::vector<std::string>& params);
    void getChannelModes(Server& server, int client_fd, const std::string& channel); // Kanal modlarını göstermek için
}

#endif

