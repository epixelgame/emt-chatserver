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

#include <vector>
#include "packethandler.h"
#include "session.h"

namespace epixel
{

struct PacketHandlerEntry
{
	const char* name;
	SessionState state;
	bool (Session::*handler)(const NetworkMessage& msg);
};

static const PacketHandlerEntry nullPacketHandlerEntry = { "NULL", SESSION_NOT_AUTH, nullptr };
const std::vector<PacketHandlerEntry> packetHandlingTable = {
	nullPacketHandlerEntry,
	nullPacketHandlerEntry,
	nullPacketHandlerEntry,
	{ "AUTH", SESSION_NOT_AUTH, &Session::handle_Auth },
	{ "CHATMSG", SESSION_AUTH, &Session::handle_ChatMessage },
};

bool PacketHandler::isValidOpcode(const u16 opcode) const
{
	return (opcode < packetHandlingTable.size() && packetHandlingTable[opcode].handler);
}

bool PacketHandler::packetNeedsUnauth(const u16 opcode) const
{
	return (packetHandlingTable[opcode].state == SESSION_NOT_AUTH);
}

bool PacketHandler::handlePacket(Session *sess, const NetworkMessage &msg) const
{
	const PacketHandlerEntry& opHandle = packetHandlingTable[msg.opcode];
	return (sess->*opHandle.handler)(msg);
}
}
