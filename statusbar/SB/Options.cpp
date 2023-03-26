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

#include "SB/Options.hpp"

namespace SB
{
    class Options::IMPL
    {
        public:

            IMPL( int argc, const char ** argv );
            IMPL( const IMPL & o );
            ~IMPL();

            bool _debug;
            bool _cpu;
            bool _memory;
            bool _temperature;
            bool _battery;
            bool _network;
            bool _date;
            bool _hour;
    };

    Options::Options( int argc, const char ** argv ):
        impl( std::make_unique< IMPL >( argc, argv ) )
    {}

    Options::Options( const Options & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    Options::Options( Options && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    Options::~Options()
    {}

    Options & Options::operator =( Options o )
    {
        swap( *( this ), o );

        return *( this );
    }

    bool Options::debug() const
    {
        return this->impl->_debug;
    }

    bool Options::cpu() const
    {
        return this->impl->_cpu;
    }

    bool Options::memory() const
    {
        return this->impl->_memory;
    }

    bool Options::temperature() const
    {
        return this->impl->_temperature;
    }

    bool Options::battery() const
    {
        return this->impl->_battery;
    }

    bool Options::network() const
    {
        return this->impl->_network;
    }

    bool Options::date() const
    {
        return this->impl->_date;
    }

    bool Options::hour() const
    {
        return this->impl->_hour;
    }

    void swap( Options & o1, Options & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    Options::IMPL::IMPL( int argc, const char ** argv ):
        _debug(       false ),
        _cpu(         false ),
        _memory(      false ),
        _temperature( false ),
        _battery(     false ),
        _network(     false ),
        _date(        false ),
        _hour(        false )
    {
        for( int i = 0; i < argc; i++ )
        {
            std::string option = argv[ i ];

            if( option == "--debug"       ) { this->_debug       = true; }
            if( option == "--cpu"         ) { this->_cpu         = true; }
            if( option == "--memory"      ) { this->_memory      = true; }
            if( option == "--temperature" ) { this->_temperature = true; }
            if( option == "--battery"     ) { this->_battery     = true; }
            if( option == "--network"     ) { this->_network     = true; }
            if( option == "--date"        ) { this->_date        = true; }
            if( option == "--hour"        ) { this->_hour        = true; }
        }

        if
        (
               this->_cpu         == false
            && this->_memory      == false
            && this->_temperature == false
            && this->_battery     == false
            && this->_network     == false
            && this->_date        == false
            && this->_hour        == false
        )
        {
            this->_cpu         = true;
            this->_memory      = true;
            this->_temperature = true;
            this->_battery     = true;
            this->_network     = true;
            this->_date        = true;
            this->_hour        = true;
        }
    }

    Options::IMPL::IMPL( const IMPL & o ):
        _debug(       o._debug ),
        _cpu(         o._cpu ),
        _memory(      o._memory ),
        _temperature( o._temperature ),
        _battery(     o._battery ),
        _network(     o._network ),
        _date(        o._date ),
        _hour(        o._hour )
    {}

    Options::IMPL::~IMPL()
    {}
}
