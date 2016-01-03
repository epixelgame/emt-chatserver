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

#include <apr-1/apr_poll.h>
#include <json/json.h>
#include <queue>
#include <unordered_map>

namespace epixel
{

struct NetworkMessage;
class ChatServer;
class SessionManager;

enum SessionState {
	SESSION_NOT_AUTH,
	SESSION_AUTH,
};

class Session
{
public:
	Session(const int id, SessionManager* mgr, ChatServer* cs): m_id(id), m_mgr(mgr), m_cs(cs) {}
	~Session() {}

	// Receivers
	bool handle_Auth(const NetworkMessage &msg);
	bool handle_ChatMessage(const NetworkMessage &msg);

	// Senders
	void sendMessage(const Json::Value &what);
	bool consumeQueuedMessage(std::string &what);

	// getters
	inline const SessionState getState() const { return m_state; }
	inline void setPfdIn(const apr_pollfd_t pfd_out) { m_pfd_in = pfd_out; }
	inline void setPfdOut(const apr_pollfd_t pfd_out) { m_pfd_out = pfd_out; }
	inline apr_pollfd_t* getPfdIn() { return &m_pfd_in; }
private:
	int m_id = 0;
	std::string m_servername = "";
	SessionState m_state = SESSION_NOT_AUTH;
	SessionManager* m_mgr;

	// Outgoing related
	ChatServer* m_cs;
	apr_pollfd_t m_pfd_in;
	apr_pollfd_t m_pfd_out;
	std::queue<std::string> m_sending_queue;
};

class SessionManager
{
public:
	SessionManager();
	~SessionManager();

	// Session management
	int createSession(ChatServer* cs);
	void destroySession(const int id);
	Session* getSession(const int id);
	void broadcastToOthers(Session* source, const Json::Value& what) const;
private:
	std::unordered_map<int, Session*> m_sessions;

};

}
