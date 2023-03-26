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

#include "SB/SMC.hpp"
#include <cmath>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfour-char-constants"

const uint32_t kSMCKeyNKEY = '#KEY';
const uint32_t kSMCKeyACID = 'ACID';

#pragma clang diagnostic push

namespace SB
{
    namespace SMC
    {
        bool callSMCFunction( io_connect_t connection, uint32_t function, const SMCParamStruct & input, SMCParamStruct & output )
        {
            if( connection == IO_OBJECT_NULL )
            {
                return false;
            }

            size_t        inputSize  = sizeof( SMCParamStruct );
            size_t        outputSize = sizeof( SMCParamStruct );
            kern_return_t result     = IOConnectCallMethod( connection, kSMCUserClientOpen, nullptr, 0, nullptr, 0, nullptr, nullptr, nullptr, nullptr );

            if( result != kIOReturnSuccess )
            {
                return false;
            }

            result = IOConnectCallStructMethod( connection, function, &input, inputSize, &output, &outputSize );

            IOConnectCallMethod( connection, kSMCUserClientClose, nullptr, 0, nullptr, 0, nullptr, nullptr, nullptr, nullptr );

            return result == kIOReturnSuccess;
        }

        bool readSMCKeyInfo( io_connect_t connection, SMCKeyInfoData & info, uint32_t key, std::map< uint32_t, SMCKeyInfoData > & cache )
        {
            if( connection == IO_OBJECT_NULL || key == 0 )
            {
                return false;
            }

            if( cache.find( key ) != cache.end() )
            {
                info = cache[ key ];

                return true;
            }

            SMCParamStruct input;
            SMCParamStruct output;

            bzero( &input,  sizeof( SMCParamStruct ) );
            bzero( &output, sizeof( SMCParamStruct ) );

            input.data8 = kSMCGetKeyInfo;
            input.key   = key;

            if( SMC::callSMCFunction( connection, kSMCHandleYPCEvent, input, output ) == false )
            {
                return false;
            }

            if( output.result != kSMCSuccess )
            {
                return false;
            }

            info         = output.keyInfo;
            cache[ key ] = output.keyInfo;

            return true;
        }

        bool readSMCKey( io_connect_t connection, uint32_t & key, uint32_t index )
        {
            if( connection == IO_OBJECT_NULL )
            {
                return false;
            }

            SMCParamStruct input;
            SMCParamStruct output;

            bzero( &input,  sizeof( SMCParamStruct ) );
            bzero( &output, sizeof( SMCParamStruct ) );

            input.data8  = kSMCGetKeyFromIndex;
            input.data32 = index;

            if( SMC::callSMCFunction( connection, kSMCHandleYPCEvent, input, output ) == false )
            {
                return false;
            }

            if( output.result != kSMCSuccess )
            {
                return false;
            }

            key = output.key;

            return true;
        }

        bool readSMCKey( io_connect_t connection, uint32_t key, uint8_t * buffer, uint32_t & maxSize, SMCKeyInfoData & keyInfo, std::map< uint32_t, SMCKeyInfoData > & cache )
        {
            if( connection == IO_OBJECT_NULL || key == 0 || buffer == nullptr )
            {
                return false;
            }

            SMCKeyInfoData info;

            if( SMC::readSMCKeyInfo( connection, info, key, cache ) == false )
            {
                return false;
            }

            SMCParamStruct input;
            SMCParamStruct output;

            bzero( &input,  sizeof( SMCParamStruct ) );
            bzero( &output, sizeof( SMCParamStruct ) );

            input.key              = key;
            input.data8            = kSMCReadKey;
            input.keyInfo.dataSize = info.dataSize;

            if( SMC::callSMCFunction( connection, kSMCHandleYPCEvent, input, output ) == false )
            {
                return false;
            }

            if( output.result != kSMCSuccess )
            {
                return false;
            }

            if( maxSize < info.dataSize )
            {
                return false;
            }

            keyInfo = info;
            maxSize = info.dataSize;

            bzero( buffer, maxSize );

            for( uint32_t i = 0; i < info.dataSize; i++ )
            {
                if( key == kSMCKeyACID )
                {
                    buffer[ i ] = output.bytes[ i ];
                }
                else
                {
                    buffer[ i ] = output.bytes[ info.dataSize - ( i + 1 ) ];
                }
            }

            return true;
        }

        uint32_t readSMCKeyCount( io_connect_t connection, std::map< uint32_t, SMCKeyInfoData > & cache )
        {
            uint8_t        data[ 8 ];
            uint32_t       size = sizeof( data );
            SMCKeyInfoData keyInfo;

            bzero( data, size );

            if( SMC::readSMCKey( connection, kSMCKeyNKEY, data, size, keyInfo, cache ) == false )
            {
                return 0;
            }

            return SMC::readInteger( data, size );
        }

        uint32_t readInteger( uint8_t * data, uint32_t size )
        {
            uint32_t n = 0;

            if( size > sizeof( uint32_t ) )
            {
                return 0;
            }

            for( uint32_t i = 0; i < size; i++ )
            {
                n |= static_cast< uint32_t >( data[ i ] ) << ( i * 8 );
            }

            return n;
        }

        uint32_t readUInt32( uint8_t * data, uint32_t size )
        {
            if( size != 4 )
            {
                return 0;
            }

            uint32_t u1 = static_cast< uint32_t >( data[ 0 ] ) << 24;
            uint32_t u2 = static_cast< uint32_t >( data[ 1 ] ) << 16;
            uint32_t u3 = static_cast< uint32_t >( data[ 2 ] ) <<  8;
            uint32_t u4 = static_cast< uint32_t >( data[ 3 ] ) <<  0;

            return u1 | u2 | u3 | u4;
        }

        uint64_t readUInt64( uint8_t * data, uint32_t size )
        {
            if( size != 8 )
            {
                return 0;
            }

            uint64_t u1 = static_cast< uint64_t >( data[ 0 ] ) << 56;
            uint64_t u2 = static_cast< uint64_t >( data[ 1 ] ) << 48;
            uint64_t u3 = static_cast< uint64_t >( data[ 2 ] ) << 40;
            uint64_t u4 = static_cast< uint64_t >( data[ 3 ] ) << 32;
            uint64_t u5 = static_cast< uint64_t >( data[ 4 ] ) << 24;
            uint64_t u6 = static_cast< uint64_t >( data[ 5 ] ) << 16;
            uint64_t u7 = static_cast< uint64_t >( data[ 6 ] ) <<  8;
            uint64_t u8 = static_cast< uint64_t >( data[ 7 ] ) <<  0;

            return u1 | u2 | u3 | u4 | u5 | u6 | u7 | u8;
        }

        double readFloat32( uint8_t * data, uint32_t size )
        {
            if( size != 4 )
            {
                return 0;
            }

            uint32_t u32 = SMC::readUInt32( data, size );

            if( u32 == 0 )
            {
                return 0;
            }

            return static_cast< double >( *( reinterpret_cast< float * >( &u32 ) ) );
        }

        double readIOFloat( uint8_t * data, uint32_t size )
        {
            if( size != 8 )
            {
                return 0;
            }

            uint64_t u64 = SMC::readUInt64( data, size );

            if( u64 == 0 )
            {
                return 0;
            }

            double integral   = static_cast< double >( u64 >> 16 );
            double mask       = pow( 2.0, 16.0 ) - 1;
            double fractional = static_cast< double >( u64 & static_cast< uint64_t >( mask ) ) / static_cast< double >( 1 << 16 );

            return integral + fractional;
        }
    }
}
