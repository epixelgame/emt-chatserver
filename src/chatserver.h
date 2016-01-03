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

#include <queue>
#include <apr-1/apr_network_io.h>
#include <apr-1/apr_poll.h>
#include "cs_types.h"

namespace epixel
{

struct NetworkMessage;
class PacketHandler;
class Session;
class SessionManager;

struct EpixelServerSession
{
	int session_id = 0;
};

struct ChatMessage
{
	std::string who = "";
	std::string message = "";
};

class ChatServer {
public:
	ChatServer();
	~ChatServer();

	void stop();
	void start();

	void requestPoller(const apr_pollfd_t *pfd_out, const apr_pollfd_t* pfd_in);
private:
	// Task
	bool m_should_stop = false;

	// Memory
	apr_pool_t* m_apr_pool;

	// Socket
	const apr_status_t startListening();
	bool isSocketValid(const apr_pollfd_t *pfd, epixel::Session* sess);
	void handlePeerAccept();
	void handleReceiveData(const apr_pollfd_t* pfd);
	void sendQueuedDatas(const apr_pollfd_t* pfd_out);

	const char* getIPFromSock(apr_socket_t *p_sock);

	apr_socket_t *m_sock;
	apr_pollset_t *m_pollset;

	SessionManager* m_session_mgr;
	PacketHandler* m_packet_handler;
};

}
