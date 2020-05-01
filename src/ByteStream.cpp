/**
 StreamString.cpp

 Copyright (c) 2015 Bjarne Loft. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 */

#include <Arduino.h>
#include "ByteStream.h"

size_t ByteStream::write(const uint8_t *data, size_t size) {
    if(size && data) {
        memcpy (byteArray + this->size, data, size);
        this->size += size;
        return size;
    }
    return 0;
}

size_t ByteStream::write(uint8_t data) {
    byteArray[++size] = data;
}

int ByteStream::available() {
    return size;
}

int ByteStream::read() {
    if(size) {
        char c = byteArray[0];
        // Remove first element
        return c;
    }
    return -1;
}

int ByteStream::peek() {
    if(size) {
        return byteArray[0];
    }
    return -1;
}

void ByteStream::flush() {
}

void ByteStream::reset() {
    size = 0;
}

