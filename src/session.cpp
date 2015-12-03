/*
 * Epixel
 * Copyright (C) 2015 nerzhul, Loic Blot <loic.blot@unix-experience.fr>
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

#include "session.h"
#include "chatserver.h"
#include "globals.h"
#include "packethandler.h"

#define MIN_SESSION_ID 1
#define MAX_SESSION_ID 1000000

namespace epixel
{

bool Session::handle_Auth(const NetworkMessage &msg)
{
	if (!msg.data.isMember("server_name") ||
			!msg.data.isMember("auth_token")) {
		logger.error("Invalid auth message received from session %d", m_id);
		return false;
	}

	std::string server_name = msg.data["server_name"].asString(),
			auth_token = msg.data["auth_token"].asString();

	m_servername = server_name;

	logger.info("Servername: %s, auth_token: %s", server_name.c_str(), auth_token.c_str());
	m_state = SESSION_AUTH;
	return true;
}

bool Session::handle_ChatMessage(const NetworkMessage &msg)
{
	if (!msg.data.isMember("author") ||
			!msg.data.isMember("message")) {
		logger.error("Invalid chat message received from session %d", m_id);
		return false;
	}

	Json::Value response = msg.data;
	response["servername"] = m_servername;
	m_mgr->broadcastToOthers(this, response);

	return true;
}

void Session::sendMessage(const Json::Value &what)
{
	Json::FastWriter writer;
	std::string raw_what = writer.write(what);
	m_sending_queue.push(raw_what);
	m_cs->requestPoller(&m_pfd_out, &m_pfd_in);
}

bool Session::consumeQueuedMessage(std::string &what)
{
	if (m_sending_queue.empty()) {
		return false;
	}

	what = m_sending_queue.back();
	m_sending_queue.pop();
	return true;
}

SessionManager::SessionManager()
{
	m_sessions.clear();
}

SessionManager::~SessionManager()
{
	for (auto &s: m_sessions) {
		delete s.second;
	}
}

int SessionManager::createSession(ChatServer* cs)
{
	// Choose the mininum
	int choosen_id = MIN_SESSION_ID;

	// Verify if there is a greater session id and try to take upper
	for (const auto &s: m_sessions) {
		if (s.first >= choosen_id)
			choosen_id = s.first +1;
	}

	// If choosen session_id is greater than MAX, try to find a free slot
	// in the list
	if (choosen_id >= MAX_SESSION_ID) {
		for (int i = MIN_SESSION_ID; i < MAX_SESSION_ID; ++i) {
			if (m_sessions.find(i) == m_sessions.end()) {
				m_sessions[i] = new Session(i, this, cs);
				return i;
			}
		}

		// ID not found
		return -1;
	}

	m_sessions[choosen_id] = new Session(choosen_id, this, cs);
	return choosen_id;
}

void SessionManager::destroySession(const int id)
{
	if (m_sessions.find(id) != m_sessions.end()) {
		delete m_sessions[id];
		m_sessions.erase(id);
	}
}

Session* SessionManager::getSession(const int id)
{
	if (m_sessions.find(id) != m_sessions.end()) {
		return m_sessions[id];
	}

	return nullptr;
}

void SessionManager::broadcastToOthers(Session* source, const Json::Value& what) const
{
	for (const auto &sess: m_sessions) {
		// Ignore itself
		if (sess.second == source) {
			continue;
		}

		// Ignore unauth sessions
		if (sess.second->getState() == SESSION_NOT_AUTH) {
			continue;
		}

		sess.second->sendMessage(what);
	}
}

}
