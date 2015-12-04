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

#include "compress.h"
#include <lz4.h>

namespace emt
{

void lz4_compress(const std::string &source, std::string &dest)
{
        int str_s = source.size();
        char* buffer = new char[LZ4_compressBound(str_s)];
        int s = LZ4_compress(source.c_str(), buffer, str_s);
        dest = std::string(buffer,s);
        delete [] buffer;
}

void lz4_decompress(const std::string &source, std::string &dest)
{
        int maxLZ4bufferSize = source.size() * 255;
        char* buffer =  new char[maxLZ4bufferSize];
        int s = LZ4_decompress_safe(source.c_str(), buffer,
                source.size(), maxLZ4bufferSize);

        dest = std::string(buffer, s);
        delete [] buffer;
}

} // end namespace emt
