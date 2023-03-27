/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2023, Jean-David Gadina - www.xs-labs.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#ifndef SB_SMC_HPP
#define SB_SMC_HPP

#include "SB/Instruments/Internal/SMC-Internal.h"
#include <cstdint>
#include <map>

namespace SB
{
    namespace SMC
    {
        bool     callSMCFunction( io_connect_t connection, uint32_t function, const SMCParamStruct & input, SMCParamStruct & output );
        bool     readSMCKeyInfo( io_connect_t connection, SMCKeyInfoData & info, uint32_t key, std::map< uint32_t, SMCKeyInfoData > & cache );
        bool     readSMCKey( io_connect_t connection, uint32_t & key, uint32_t index );
        bool     readSMCKey( io_connect_t connection, uint32_t key, uint8_t * buffer, uint32_t & maxSize, SMCKeyInfoData & keyInfo, std::map< uint32_t, SMCKeyInfoData > & cache );
        uint32_t readSMCKeyCount( io_connect_t connection, std::map< uint32_t, SMCKeyInfoData > & cache );
        uint32_t readInteger( uint8_t * data, uint32_t size );
        uint32_t readUInt32( uint8_t * data, uint32_t size );
        uint64_t readUInt64( uint8_t * data, uint32_t size );
        double   readFloat32( uint8_t * data, uint32_t size );
        double   readFloat32( uint8_t * data, uint32_t size );
        double   readIOFloat( uint8_t * data, uint32_t size );
    }
}

#endif /* SB_SMC_HPP */
