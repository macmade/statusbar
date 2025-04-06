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

#include "SB/UI/Screen.hpp"
#include "SB/UI/ColorPair.hpp"
#include "SB/SleepManager.hpp"
#include <algorithm>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <vector>
#include <poll.h>
#include <condition_variable>
#include <mutex>

namespace SB
{
    class Screen::IMPL
    {
        public:
            
            IMPL();
            ~IMPL();
            
            std::vector< std::function< void() > > _onResize;
            std::vector< std::function< void( int ) > >  _onKeyPress;
            std::vector< std::function< void() > > _onUpdate;
            
            std::size_t          _width;
            std::size_t          _height;
            bool                 _colors;
            bool                 _running;
            bool                 _sleeping;
            UUID                 _sleepRegistration;
            std::recursive_mutex _rmtx;
    };
    
    Screen & Screen::shared()
    {
        static Screen       * screen( nullptr );
        static std::once_flag once;
        
        std::call_once( once, [ & ]{ screen = new Screen(); } );
        
        return *( screen );
    }
    
    Screen::Screen():
        impl( std::make_unique< IMPL >() )
    {
        struct winsize s;
        
        ::initscr();
        
        if( ::has_colors() )
        {
            this->impl->_colors = true;
            
            ::start_color();
            ::use_default_colors();
        }
        
        this->clear();
        ::noecho();
        ::cbreak();
        ::keypad( stdscr, true );
        this->refresh();
        
        ::ioctl( STDOUT_FILENO, TIOCGWINSZ, &s );
        
        this->impl->_width  = s.ws_col;
        this->impl->_height = s.ws_row;
    }
    
    std::size_t Screen::width() const
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        return this->impl->_width;
    }
    
    std::size_t Screen::height() const
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        return this->impl->_height;
    }
    
    bool Screen::supportsColors() const
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        return this->impl->_colors;
    }
    
    void Screen::disableColors() const
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        this->impl->_colors = false;
    }
    
    bool Screen::isRunning() const
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        return this->impl->_running;
    }
    
    void Screen::clear() const
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        ::clear();
    }
    
    void Screen::refresh() const
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        ::refresh();
    }

    void Screen::print( const std::string & s )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );

        ::printw( s.c_str() );
    }

    void Screen::print( const Color & foreground, const std::string & s )
    {
        this->print( foreground, Color::clear(), s );
    }

    void Screen::print( const Color & foreground, const Color & background, const std::string & s )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );

        if( this->supportsColors() )
        {
            ::attrset( static_cast< int >( COLOR_PAIR( ColorPair::pairForColors( foreground, background ) ) ) );
        }

        ::printw( s.c_str() );

        if( this->supportsColors() )
        {
            ::attrset( static_cast< int >( COLOR_PAIR( ColorPair::pairForColors( Color::clear(), Color::clear() ) ) ) );
        }
    }
    
    void Screen::start( unsigned int refreshInterval )
    {
        {
            std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
            
            if( this->impl->_running )
            {
                return;
            }
            
            this->impl->_running = true;
        }

        ::curs_set( 0 );
        
        while( this->impl->_running )
        {
            struct winsize                               s;
            std::vector< std::function< void() > >       onResize;
            std::vector< std::function< void( int ) > >  onKeyPress;
            std::vector< std::function< void() > >       onUpdate;
            int                                          key( 0 );
            
            ::ioctl( STDOUT_FILENO, TIOCGWINSZ, &s );
            
            {
                std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
                
                onKeyPress = this->impl->_onKeyPress;
                onUpdate   = this->impl->_onUpdate;
                
                if( s.ws_col != this->impl->_width || s.ws_row != this->impl->_height )
                {
                    this->impl->_width  = s.ws_col;
                    this->impl->_height = s.ws_row;
                    
                    onResize = this->impl->_onResize;
                }
                
                {
                    static struct pollfd p;
                    
                    memset( &p, 0, sizeof( p ) );
                    
                    p.fd      = 0;
                    p.events  = POLLIN;
                    p.revents = 0;
                    
                    if( poll( &p, 1, 0 ) > 0 )
                    {
                        key        = getc( stdin );
                        onKeyPress = this->impl->_onKeyPress;
                    }
                }
            }
            
            for( const auto & f: onResize )
            {
                f();
            }
            
            for( const auto & f: onKeyPress )
            {
                f( key );
            }
            
            for( const auto & f: onUpdate )
            {
                f();
            }
            
            this->refresh();

            if( this->impl->_sleeping )
            {
                std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
            }
            else
            {
                std::this_thread::sleep_for( std::chrono::milliseconds( refreshInterval ) );
            }
        }
        
        this->clear();
        this->refresh();

        ::curs_set( 1 );
    }
    
    void Screen::stop()
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        this->impl->_running = false;
    }
    
    void Screen::onResize( const std::function< void() > & f )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        this->impl->_onResize.push_back( f );
    }
    
    void Screen::onKeyPress( const std::function< void( int key ) > & f )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        this->impl->_onKeyPress.push_back( f );
    }
    
    void Screen::onUpdate( const std::function< void() > & f )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        
        this->impl->_onUpdate.push_back( f );
    }
    
    Screen::IMPL::IMPL():
        _width( 0 ),
        _height( 0 ),
        _colors( false ),
        _running( false ),
        _sleeping( false )
    {
        this->_sleepRegistration = SleepManager::shared().subscribe
        (
            [ & ]( SleepManager::Event e )
            {
                std::lock_guard< std::recursive_mutex > l( this->_rmtx );

                if( e == SleepManager::Event::WillSleep )
                {
                    this->_sleeping = true;
                }
                else if( e == SleepManager::Event::DidPowerOn )
                {
                    this->_sleeping = false;
                }
            }
        );
    }
    
    Screen::IMPL::~IMPL()
    {
        SleepManager::shared().unsubscribe( this->_sleepRegistration );

        ::clrtoeol();
        ::refresh();
        ::endwin();
    }
}
