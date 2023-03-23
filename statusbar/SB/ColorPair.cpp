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

#include "SB/ColorPair.hpp"
#include <ncurses.h>
#include <mutex>
#include <tuple>
#include <vector>

namespace SB
{
    class ColorPair::IMPL
    {
        public:

            IMPL();
            IMPL( const IMPL & o );
            ~IMPL();

            std::vector< std::tuple< short, Color, Color > > _pairs;
    };

    short ColorPair::pairForColors( const Color & foreground, const Color & background )
    {
        ColorPair & pairs = ColorPair::shared();

        for( const auto & p: pairs.impl->_pairs )
        {
            if( std::get< 1 >( p ) == foreground && std::get< 2 >( p ) == background )
            {
                return std::get< 0 >( p );
            }
        }

        return 0;
    }

    ColorPair & ColorPair::shared()
    {
        static ColorPair    * instance( nullptr );
        static std::once_flag once;

        std::call_once( once, [ & ]{ instance = new ColorPair(); } );

        return *( instance );
    }

    ColorPair::ColorPair():
        impl( std::make_unique< IMPL >() )
    {}

    ColorPair::ColorPair( const ColorPair & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    ColorPair::ColorPair( ColorPair && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    ColorPair::~ColorPair()
    {}

    ColorPair & ColorPair::operator =( ColorPair o )
    {
        swap( *( this ), o );

        return *( this );
    }

    void swap( ColorPair & o1, ColorPair & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    ColorPair::IMPL::IMPL()
    {
        short                i      = 1;
        std::vector< Color > colors =
        {
            Color::clear(),
            Color::black(),
            Color::red(),
            Color::green(),
            Color::yellow(),
            Color::blue(),
            Color::magenta(),
            Color::cyan(),
            Color::white()
        };

        for( const auto & c1: std::vector< Color >( colors ) )
        {
            for( const auto & c2: std::vector< Color >( colors ) )
            {
                ::init_pair( i, c1.value(), c2.value() );
                this->_pairs.push_back( { i, c1, c2 } );

                i++;
            }
        }
    }

    ColorPair::IMPL::IMPL( const IMPL & o ):
        _pairs( o._pairs )
    {}

    ColorPair::IMPL::~IMPL()
    {}
}
