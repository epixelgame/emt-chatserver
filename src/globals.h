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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <string>
#include <log4cpp/Category.hh>
#include <pthread.h>
#include <apr-1/apr_pools.h>

extern log4cpp::Category& logger;

extern apr_pool_t *g_apr_pool;

#endif
