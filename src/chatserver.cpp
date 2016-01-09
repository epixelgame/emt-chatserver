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

#include <json/json.h>
#include <unistd.h>
#include "chatserver.h"
#include "globals.h"
#include "packethandler.h"
#include "session.h"
#include "util.h"

namespace epixel
{

#define POOLSET_SIZE 32
#define SOCKET_BUFFER_SIZE 1536
#define HEADER_LEN 4

ChatServer::ChatServer()
{
	m_session_mgr = new SessionManager();
	m_packet_handler = new PacketHandler();
}

ChatServer::~ChatServer()
{
	delete m_packet_handler;
	delete m_session_mgr;
}

void ChatServer::start()
{
	logger.notice("Starting ChatServer");
	// This thread will use its own pool child pool
	apr_pool_create(&m_apr_pool, g_apr_pool);

	if (startListening() != APR_SUCCESS) {
		logger.fatal("Failed to listen to TCP socket %s:%d", config.bind_addr.c_str(), config.bind_port);
		return;
	}

	apr_status_t rv;
	apr_int32_t num;
	const apr_pollfd_t *ret_pfd;

	apr_pollset_create(&m_pollset, POOLSET_SIZE, m_apr_pool, APR_POLLSET_THREADSAFE | APR_POLLSET_WAKEABLE);

	{
		apr_pollfd_t pfd = { m_apr_pool, APR_POLL_SOCKET, APR_POLLIN, 0, { NULL }, NULL };
		pfd.desc.s = m_sock;
		apr_pollset_add(m_pollset, &pfd);
	}

	logger.notice("ChatServer now listens on %s:%d", config.bind_addr.c_str(), config.bind_port);

	while (!m_should_stop) {
		rv = apr_pollset_poll(m_pollset, APR_USEC_PER_SEC * 30, &num, &ret_pfd);
		if (rv == APR_SUCCESS) {
			for (int i = 0; i < num; i++) {
				// Handling connection accept
				if (ret_pfd[i].desc.s == m_sock) {
					handlePeerAccept();
				}
				// reading incoming packets
				else {
					logger.debug("Poller event ID %d triggered.", ret_pfd[i].reqevents);
					if (ret_pfd[i].reqevents == APR_POLLIN) {
						handleReceiveData(&ret_pfd[i]);
					}
					else if (ret_pfd[i].reqevents == APR_POLLOUT) {
						sendQueuedDatas(&ret_pfd[i]);
					}
				}
			}
		}

#ifdef _WIN32
		sleep_ms(1);
#else
		usleep(50);
#endif
	}

	logger.notice("Stopping ChatServer");

	apr_socket_close(m_sock);
}

bool ChatServer::isSocketValid(const apr_pollfd_t *pfd, Session* sess)
{
	EpixelServerSession* fddatas = (EpixelServerSession*)pfd->client_data;
	apr_socket_t* p_sock = pfd->desc.s;

	int sock_atroeaf = 0;
	// Check if socket is always valid
	apr_status_t rv = apr_socket_atreadeof(p_sock, &sock_atroeaf);
	if (sock_atroeaf != 0) {
		logger.notice("%s is disconnected. Closing socket.", getIPFromSock(p_sock));
		if (sess) {
			m_session_mgr->destroySession(fddatas->session_id);
		}
		apr_pollset_remove(m_pollset, pfd);
		apr_socket_close(p_sock);
		return false;
	}

	return true;
}

void ChatServer::handleReceiveData(const apr_pollfd_t *pfd)
{
	EpixelServerSession* fddatas = (EpixelServerSession*)pfd->client_data;
	apr_socket_t* p_sock = pfd->desc.s;
	Session* sess = m_session_mgr->getSession(fddatas->session_id);

	std::string recv_s = "";
	unsigned int packet_size_to_recv = 0;
	while (true) {
		// Read socket
		char buf[SOCKET_BUFFER_SIZE];
		apr_size_t len = sizeof(buf);
		// Init buffer
		memset(buf, 0, len);

		if (!isSocketValid(pfd, sess)) {
			return;
		}

		// If packet_size_to_recv == 0, we should read the packet size
		if (packet_size_to_recv == 0) {
			apr_size_t header_len = HEADER_LEN;
			// receive datas
			apr_status_t rv = apr_socket_recv(p_sock, buf, &header_len);
			if (rv == APR_EOF || header_len < HEADER_LEN) {
				logger.warn("Invalid packet header received from %s. Closing socket.", getIPFromSock(p_sock));
				if (sess) {
					m_session_mgr->destroySession(fddatas->session_id);
				}
				apr_pollset_remove(m_pollset, pfd);
				apr_socket_close(p_sock);
				return;
			}

			packet_size_to_recv = readUInt(buf);

			if (packet_size_to_recv > MAX_ALLOWED_PACKET_SIZE) {
				logger.warn("Server %s wants to send us a very large packet (size: %d). Closing socket.", getIPFromSock(p_sock), packet_size_to_recv);
				if (sess) {
					m_session_mgr->destroySession(fddatas->session_id);
				}
				apr_pollset_remove(m_pollset, pfd);
				apr_socket_close(p_sock);
				return;
			}

			// Reinit buf
			memset(buf, 0, len);
		}

		if (!isSocketValid(pfd, sess)) {
			return;
		}

		if (len > packet_size_to_recv) {
			len = packet_size_to_recv;
		}

		// receive datas
		apr_status_t rv = apr_socket_recv(p_sock, buf, &len);

		// Store them to outbuffer
		recv_s.append(buf, len);

		// And decrement size to read
		packet_size_to_recv -= len;
		if (rv == APR_EOF || len == 0 || packet_size_to_recv <= 0) {
			packet_size_to_recv = 0;
			// Nothing more to read on the socket, exiting the loop
			break;
		}
	}

	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(recv_s, root, false) || !root["o"].isInt() || !root["data"].isObject()) {
		logger.error("Invalid JSON received from %s, ignoring and closing connection.", getIPFromSock(p_sock));
		if (sess) {
			m_session_mgr->destroySession(fddatas->session_id);
		}
		apr_pollset_remove(m_pollset, pfd);
		apr_socket_close(p_sock);
		return;
	}

	uint16_t opcode = root["o"].asInt();

	if (!m_packet_handler->isValidOpcode(opcode)) {
		logger.error("Invalid opcode %d received from %s, ignoring and closing connection.",
				opcode, getIPFromSock(p_sock));
		if (sess) {
			m_session_mgr->destroySession(fddatas->session_id);
		}
		apr_pollset_remove(m_pollset, pfd);
		apr_socket_close(p_sock);
		return;
	}

	// Read message
	NetworkMessage* msg = new NetworkMessage(fddatas->session_id, opcode);
	msg->data = root["data"];

	if (!sess) {
		logger.warn("No session found for %s, ignoring and closing connection.", getIPFromSock(p_sock));
		apr_pollset_remove(m_pollset, pfd);
		apr_socket_close(p_sock);
		delete msg;
		return;
	}

	// Handle packet which needs to not be auth there
	if (m_packet_handler->packetNeedsUnauth(msg->opcode)) {
		logger.debug("Packet (unauth) with opcode %d sent to packet handler", msg->opcode);
		if (!m_packet_handler->handlePacket(sess, *msg)) {
			logger.info("Packet was handled but content is invalid, disconnecting peer %s", getIPFromSock(p_sock));
			m_session_mgr->destroySession(fddatas->session_id);
			apr_pollset_remove(m_pollset, pfd);
			apr_socket_close(p_sock);
		}
		delete msg;
		// Message was okay, process other messages
		return;
	}
	// else drop session & connection if auth is required and session isn't
	else if (sess->getState() == SESSION_NOT_AUTH) {
		logger.warn("Packet received without valid session opened, ignoring and closing connection.", getIPFromSock(p_sock));
		m_session_mgr->destroySession(fddatas->session_id);
		apr_pollset_remove(m_pollset, pfd);
		apr_socket_close(p_sock);
		delete msg;
		return;
	}

	logger.debug("Packet (auth) with opcode %d sent to packet handler", msg->opcode);
	if (!m_packet_handler->handlePacket(sess, *msg)) {
		logger.info("Packet was handled but content is invalid, disconnecting peer %s", getIPFromSock(p_sock));
		m_session_mgr->destroySession(fddatas->session_id);
		apr_pollset_remove(m_pollset, pfd);
		apr_socket_close(p_sock);
		return;
	}
}

void ChatServer::requestPoller(const apr_pollfd_t *pfd_out, const apr_pollfd_t *pfd_in)
{
	apr_pollset_remove(m_pollset, pfd_in);
	apr_pollset_add(m_pollset, pfd_out);
}

void ChatServer::sendQueuedDatas(const apr_pollfd_t *pfd_out)
{
	EpixelServerSession* fddatas = (EpixelServerSession*)pfd_out->client_data;
	apr_socket_t* p_sock = pfd_out->desc.s;
	Session* sess = m_session_mgr->getSession(fddatas->session_id);
	if (!sess) {
		return;
	}

	logger.debug("Sending datas in queue for session %d - %s", fddatas->session_id, getIPFromSock(p_sock));

	std::string s_message = "";
	apr_size_t s_message_s;
	while (sess->consumeQueuedMessage(s_message)) {
		s_message_s = s_message.size();
		apr_socket_send(p_sock, s_message.c_str(), &s_message_s);
	}

	// all datas are sent, close this sender
	apr_pollset_remove(m_pollset, pfd_out);
	apr_pollset_add(m_pollset, sess->getPfdIn());
}

void ChatServer::handlePeerAccept()
{
	apr_socket_t *p_sock;
	apr_status_t rv;

	rv = apr_socket_accept(&p_sock, m_sock, m_apr_pool);
	if (rv == APR_SUCCESS) {
		// Create a FD data structure to retrieve which peer is it
		EpixelServerSession* fddata = (EpixelServerSession*)apr_palloc(m_apr_pool, sizeof(EpixelServerSession));
		fddata->session_id = m_session_mgr->createSession(this);

		// Failed to find a session ID, stop it
		if (fddata->session_id == -1) {
			logger.error("Failed to find a session slot %s", getIPFromSock(p_sock));
		}

		// Create a FD and bind socket on it
		apr_pollfd_t pfd = { m_apr_pool, APR_POLL_SOCKET, APR_POLLIN, 0, { NULL }, fddata };
		pfd.desc.s = p_sock;

		// non-blocking socket. We can't expect that @p_sock inherits non-blocking mode from @m_sock
		apr_socket_opt_set(p_sock, APR_SO_NONBLOCK, 1);
		apr_socket_timeout_set(p_sock, 0);

		// Bind FD to pollset
		apr_pollset_add(m_pollset, &pfd);

		// Prepare pfd_out for triggering messages to send
		apr_pollfd_t pfd_out = { m_apr_pool, APR_POLL_SOCKET, APR_POLLOUT, 0, { NULL }, fddata };
		pfd_out.desc.s = p_sock;
		Session* sess = m_session_mgr->getSession(fddata->session_id);
		sess->setPfdIn(pfd);
		sess->setPfdOut(pfd_out);

		logger.notice("Accept incoming server %s", getIPFromSock(p_sock));
	}
}

const apr_status_t ChatServer::startListening()
{
	apr_status_t rv;
	apr_sockaddr_t *sa;
	rv = apr_sockaddr_info_get(&sa, NULL, APR_INET, config.bind_port, 0, m_apr_pool);
	if (rv != APR_SUCCESS) {
		return rv;
	}

	rv = apr_socket_create(&m_sock, sa->family, SOCK_STREAM, APR_PROTO_TCP, m_apr_pool);
	if (rv != APR_SUCCESS) {
		return rv;
	}

	// Set socket as non blocking
	apr_socket_opt_set(m_sock, APR_SO_NONBLOCK, 1);
	apr_socket_timeout_set(m_sock, 0);
	// And permit to reuse address, useful to linux on multiple threads
	apr_socket_opt_set(m_sock, APR_SO_REUSEADDR, 1);

	rv = apr_socket_bind(m_sock, sa);
	if (rv != APR_SUCCESS) {
		return rv;
	}

	rv = apr_socket_listen(m_sock, SOMAXCONN);
	if (rv != APR_SUCCESS) {
		return rv;
	}

	return rv;
}

void ChatServer::stop()
{
	apr_pollset_wakeup(m_pollset);
}

const char* ChatServer::getIPFromSock(apr_socket_t *p_sock)
{
	apr_sockaddr_t* sa = nullptr;
	char *remote_ipaddr = nullptr;
	apr_socket_addr_get(&sa, APR_REMOTE, p_sock);
	apr_sockaddr_ip_get(&remote_ipaddr, sa);
	return remote_ipaddr;
}

}
