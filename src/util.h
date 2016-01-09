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

#include <stdint.h>

#if HAVE_ENDIAN_H
	#include <endian.h>

inline uint32_t readUInt(const char *data)
{
	uint32_t val;
	memcpy(&val, data, 4);
	return be32toh(val);
}

#else

inline uint32_t readUInt(const char *data)
{
	return
		((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
		((uint32_t)data[2] <<  8) | ((uint32_t)data[3] <<  0);
}

#endif

namespace filesystem
{

#ifdef _WIN32 // WINDOWS

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <shlwapi.h>

inline bool path_exists(const std::string &path)
{
	return (GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES);
}

#else

inline bool path_exists(const std::string &path)
{
	struct stat st;
	return (stat(path.c_str(),&st) == 0);
}

#endif

} // end namespace filesystem

