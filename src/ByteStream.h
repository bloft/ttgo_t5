/**
 ByteStream.h

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

#ifndef BYTESTREAM_H_
#define BYTESTREAM_H_

#include <Stream.h>

class ByteStream: public Stream {
public:
    uint8_t byteArray[60750] = {0};
    long size = 0;

    size_t write(const uint8_t *buffer, size_t size) override;
    size_t write(uint8_t data) override;

    int available() override;
    int read() override;
    int peek() override;
    void flush() override;

    void reset();
};


#endif /* BYTESTREAM_H_ */
