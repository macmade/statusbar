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

#include "SB/Helpers/String.hpp"
#include <cstring>
#include <cstdarg>

namespace SB
{
    namespace String
    {
        std::string format( const char * format, ... )
        {
            va_list     ap;
            std::string s;

            va_start( ap, format );

            int size = vsnprintf( nullptr, 0, format, ap );

            if( size > 0 )
            {
                char * cp = static_cast< char * >( calloc( 1, static_cast< size_t >( size ) + 1 ) );

                if( cp != nullptr )
                {
                    vsnprintf( cp, static_cast< size_t >( size + 1 ), format, ap );

                    s = cp;
                }

                free( cp );
            }

            va_end( ap );
            
            return s;
        }

        std::string bytesToHumanReadable( uint64_t bytes )
        {
            if( bytes < 1024ULL )
            {
                return String::format( "%lluB", bytes );
            }
            else if( bytes < 1024ULL * 1024ULL )
            {
                return String::format( "%.02fKB", static_cast< double >( bytes ) / static_cast< double >( 1024ULL ) );
            }
            else if( bytes < 1024ULL * 1024ULL * 1024ULL )
            {
                return String::format( "%.02fMB", static_cast< double >( bytes ) / static_cast< double >( 1024ULL * 1024ULL ) );
            }
            else if( bytes < 1024ULL * 1024ULL * 1024ULL * 1024ULL )
            {
                return String::format( "%.02fGB", static_cast< double >( bytes ) / static_cast< double >( 1024ULL * 1024ULL * 1024ULL ) );
            }

            return String::format( "%.02fTB", static_cast< double >( bytes ) / static_cast< double >( 1024ULL * 1024ULL * 1024ULL * 1024ULL ) );
        }

        bool hasSuffix( const std::string & str, const std::string & suffix )
        {
            std::string::size_type pos = str.find( suffix );

            if( str.length() >= suffix.length() && pos == str.length() - suffix.length() )
            {
                return true;
            }

            return false;
        }

        std::string fourCC( uint32_t c )
        {
            uint8_t c1 = static_cast< uint8_t >( ( c >> 24 ) & 0xFF );
            uint8_t c2 = static_cast< uint8_t >( ( c >> 16 ) & 0xFF );
            uint8_t c3 = static_cast< uint8_t >( ( c >>  8 ) & 0xFF );
            uint8_t c4 = static_cast< uint8_t >( ( c >>  0 ) & 0xFF );

            return String::format( "%c%c%c%c", c1, c2, c3, c4 );
        }
    }
}
