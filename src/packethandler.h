/*
 * Epixel
 * Copyright (C) 2015-2016 nerzhul, Loic Blot <loic.blot@unix-experience.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include <json/json.h>
#include <sstream>
#include <stdint.h>

namespace epixel {

class Session;

struct NetworkMessage
{
	NetworkMessage(uint16_t _session, uint16_t _opcode): opcode(_opcode), session_id(_session) {}
	uint16_t opcode = 0;
	int session_id = 0;
	Json::Value data;
};

class PacketHandler
{
public:
	PacketHandler() {}
	~PacketHandler() {}

	bool isValidOpcode(const uint16_t opcode) const;
	bool packetNeedsUnauth(const uint16_t opcode) const;
	inline bool packetRequireAuth(const uint16_t opcode) const { return !packetNeedsUnauth(opcode); }
	bool handlePacket(epixel::Session* sess, const NetworkMessage &msg) const;
};

}
