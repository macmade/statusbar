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

#include "SB/Color.hpp"
#include <ncurses.h>
#include <mutex>

namespace SB
{
    class Color::IMPL
    {
        public:
            
            IMPL( short value );
            IMPL( const IMPL & o );
            ~IMPL();
            
            short _value;
    };

    Color Color::clear()
    {
        return Color( -1 );
    }
    
    Color Color::black()
    {
        return Color( COLOR_BLACK );
    }
    
    Color Color::red()
    {
        return Color( COLOR_RED );
    }
    
    Color Color::green()
    {
        return Color( COLOR_GREEN );
    }
    
    Color Color::yellow()
    {
        return Color( COLOR_YELLOW );
    }
    
    Color Color::blue()
    {
        return Color( COLOR_BLUE );
    }
    
    Color Color::magenta()
    {
        return Color( COLOR_MAGENTA );
    }
    
    Color Color::cyan()
    {
        return Color( COLOR_CYAN );
    }
    
    Color Color::white()
    {
        return Color( COLOR_WHITE );
    }

    Color::Color( short value ):
        impl( std::make_unique< IMPL >( value ) )
    {}

    Color::Color( const Color & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    Color::Color( Color && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    Color::~Color()
    {}

    Color & Color::operator =( Color o )
    {
        swap( *( this ), o );
        
        return *( this );
    }

    bool Color::operator ==( const Color & o ) const
    {
        return this->value() == o.value();
    }

    bool Color::operator !=( const Color & o ) const
    {
        return !operator ==( o );
    }
    
    short Color::value() const
    {
        return this->impl->_value;
    }

    void swap( Color & o1, Color & o2 )
    {
        using std::swap;
        
        swap( o1.impl, o2.impl );
    }
    
    Color::IMPL::IMPL( short value ):
        _value( value )
    {}

    Color::IMPL::IMPL( const IMPL & o ):
        _value( o._value )
    {}

    Color::IMPL::~IMPL()
    {}
}
