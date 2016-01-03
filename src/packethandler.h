/*
 * Epixel
 * Copyright (C) 2015-2016  nerzhul, Loic Blot <loic.blot@unix-experience.fr>
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

#include <json/json.h>
#include <sstream>
#include "cs_types.h"

namespace epixel {

class Session;

struct NetworkMessage
{
	NetworkMessage(u16 _session, u16 _opcode): opcode(_opcode), session_id(_session) {}
	u16 opcode = 0;
	int session_id = 0;
	Json::Value data;
};

class PacketHandler
{
public:
	PacketHandler() {}
	~PacketHandler() {}

	bool isValidOpcode(const u16 opcode) const;
	bool packetNeedsUnauth(const u16 opcode) const;
	inline bool packetRequireAuth(const u16 opcode) const { return !packetNeedsUnauth(opcode); }
	bool handlePacket(epixel::Session* sess, const NetworkMessage &msg) const;
};

}
