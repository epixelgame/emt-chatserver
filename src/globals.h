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

#include <string>
#include <log4cpp/Category.hh>
#include <pthread.h>
#include <apr-1/apr_pools.h>
#include <stdint.h>

#define MAX_ALLOWED_PACKET_SIZE 1024

struct configReceptacle
{
	std::string auth_token = "";
	uint16_t bind_port = 30050;
	std::string bind_addr = "0.0.0.0";
	bool daemonize = false;
};

extern configReceptacle config;
extern log4cpp::Category& logger;

extern apr_pool_t *g_apr_pool;
