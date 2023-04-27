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

#include "SB/UUID.hpp"
#include "SB/Helpers/String.hpp"
#include <uuid/uuid.h>
#include <mutex>

namespace SB
{
    class UUID::IMPL
    {
        public:

            IMPL( void );
            IMPL( const IMPL & o );
            ~IMPL();

            std::string _uuid;
    };

    UUID::UUID( void ):
        impl( std::make_unique< IMPL >() )
    {}

    UUID::UUID( const UUID & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    UUID::UUID( UUID && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    UUID::~UUID( void )
    {}

    UUID & UUID::operator =( UUID o )
    {
        swap( *( this ), o );

        return *( this );
    }

    bool UUID::operator ==( const UUID & o ) const
    {
        return this->impl->_uuid == o.impl->_uuid;
    }

    bool UUID::operator !=( const UUID & o ) const
    {
        return !operator ==( o );
    }

    void swap( UUID & o1, UUID & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    UUID::IMPL::IMPL( void )
    {
        uuid_t uuid;
        char   s[ 37 ];

        uuid_generate_random( uuid );
        memset( s, 0, sizeof( s ) );
        uuid_unparse( uuid, s );

        this->_uuid = String::toLower( std::string( s ) );
    }

    UUID::IMPL::IMPL( const IMPL & o ):
        _uuid( o._uuid )
    {}

    UUID::IMPL::~IMPL()
    {}
}
