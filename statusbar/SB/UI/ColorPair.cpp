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

#include "SB/UI/ColorPair.hpp"
#include <ncurses.h>
#include <mutex>
#include <vector>

namespace SB
{
    class ColorPair::IMPL
    {
        public:

            IMPL();
            IMPL( const IMPL & o );
            ~IMPL();
    };

    /* ncurses color-pair number for a (foreground, background) combination.
     * clear() is -1 and the eight ncurses colors are 0..7, so value() + 1
     * gives 0..8; the 9x9 grid maps to pair numbers 1..81. The constructor
     * registers every pair with this same index, so the two never drift.
     */
    static short pairIndex( const Color & foreground, const Color & background )
    {
        short fg = static_cast< short >( foreground.value() + 1 );
        short bg = static_cast< short >( background.value() + 1 );

        return static_cast< short >( ( fg * 9 ) + bg + 1 );
    }

    short ColorPair::pairForColors( const Color & foreground, const Color & background )
    {
        /* Constructing the shared instance registers the pairs with ncurses. */
        ColorPair::shared();

        return pairIndex( foreground, background );
    }

    ColorPair & ColorPair::shared()
    {
        /* Process-lifetime singleton: allocated once and never freed (no shutdown path). */
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

        for( const auto & c1: colors )
        {
            for( const auto & c2: colors )
            {
                ::init_pair( pairIndex( c1, c2 ), c1.value(), c2.value() );
            }
        }
    }

    ColorPair::IMPL::IMPL( const IMPL & )
    {}

    ColorPair::IMPL::~IMPL()
    {}
}
