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

#include "SB/String.hpp"
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
    }
}
