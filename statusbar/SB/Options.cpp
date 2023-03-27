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

            bool  _debug;
            bool  _help;
            bool  _cpu;
            bool  _gpu;
            bool  _memory;
            bool  _temperature;
            bool  _battery;
            bool  _network;
            bool  _date;
            bool  _time;
            Color _cpuColor;
            Color _gpuColor;
            Color _memoryColor;
            Color _temperatureColor;
            Color _batteryColor;
            Color _networkColor;
            Color _dateColor;
            Color _timeColor;
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

    bool Options::help() const
    {
        return this->impl->_help;
    }

    bool Options::cpu() const
    {
        return this->impl->_cpu;
    }

    bool Options::gpu() const
    {
        return this->impl->_gpu;
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

    bool Options::time() const
    {
        return this->impl->_time;
    }

    Color Options::cpuColor() const
    {
        return this->impl->_cpuColor;
    }

    Color Options::gpuColor() const
    {
        return this->impl->_gpuColor;
    }

    Color Options::memoryColor() const
    {
        return this->impl->_memoryColor;
    }

    Color Options::temperatureColor() const
    {
        return this->impl->_temperatureColor;
    }

    Color Options::batteryColor() const
    {
        return this->impl->_batteryColor;
    }

    Color Options::networkColor() const
    {
        return this->impl->_networkColor;
    }

    Color Options::dateColor() const
    {
        return this->impl->_dateColor;
    }

    Color Options::timeColor() const
    {
        return this->impl->_timeColor;
    }

    void swap( Options & o1, Options & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    Options::IMPL::IMPL( int argc, const char ** argv ):
        _debug(            false ),
        _help(             false ),
        _cpu(              false ),
        _gpu(              false ),
        _memory(           false ),
        _temperature(      false ),
        _battery(          false ),
        _network(          false ),
        _date(             false ),
        _time(             false ),
        _cpuColor(         Color::green() ),
        _gpuColor(         Color::magenta() ),
        _memoryColor(      Color::blue() ),
        _temperatureColor( Color::red() ),
        _batteryColor(     Color::yellow() ),
        _networkColor(     Color::cyan() ),
        _dateColor(        Color::cyan() ),
        _timeColor(        Color::magenta() )
    {
        for( int i = 0; i < argc; i++ )
        {
            std::string option = argv[ i ];

            if( option == "--debug"       ) { this->_debug       = true; }
            if( option == "--help"        ) { this->_help        = true; }
            if( option == "--cpu"         ) { this->_cpu         = true; }
            if( option == "--gpu"         ) { this->_gpu         = true; }
            if( option == "--memory"      ) { this->_memory      = true; }
            if( option == "--temperature" ) { this->_temperature = true; }
            if( option == "--battery"     ) { this->_battery     = true; }
            if( option == "--network"     ) { this->_network     = true; }
            if( option == "--date"        ) { this->_date        = true; }
            if( option == "--time"        ) { this->_time        = true; }

            if( i < argc - 1 && option == "--cpu-color"         ) { this->_cpuColor         = Color::color( argv[ ++i ] ); }
            if( i < argc - 1 && option == "--gpu-color"         ) { this->_gpuColor         = Color::color( argv[ ++i ] ); }
            if( i < argc - 1 && option == "--memory-color"      ) { this->_memoryColor      = Color::color( argv[ ++i ] ); }
            if( i < argc - 1 && option == "--temperature-color" ) { this->_temperatureColor = Color::color( argv[ ++i ] ); }
            if( i < argc - 1 && option == "--battery-color"     ) { this->_batteryColor     = Color::color( argv[ ++i ] ); }
            if( i < argc - 1 && option == "--network-color"     ) { this->_networkColor     = Color::color( argv[ ++i ] ); }
            if( i < argc - 1 && option == "--date-color"        ) { this->_dateColor        = Color::color( argv[ ++i ] ); }
            if( i < argc - 1 && option == "--time-color"        ) { this->_timeColor        = Color::color( argv[ ++i ] ); }
        }

        if
        (
               this->_cpu         == false
            && this->_gpu         == false
            && this->_memory      == false
            && this->_temperature == false
            && this->_battery     == false
            && this->_network     == false
            && this->_date        == false
            && this->_time        == false
        )
        {
            this->_cpu         = true;
            this->_gpu         = true;
            this->_memory      = true;
            this->_temperature = true;
            this->_battery     = true;
            this->_network     = true;
            this->_date        = true;
            this->_time        = true;
        }

        for( int i = 0; i < argc; i++ )
        {
            std::string option = argv[ i ];

            if( option == "--no-cpu"         ) { this->_cpu         = false; }
            if( option == "--no-gpu"         ) { this->_gpu         = false; }
            if( option == "--no-memory"      ) { this->_memory      = false; }
            if( option == "--no-temperature" ) { this->_temperature = false; }
            if( option == "--no-battery"     ) { this->_battery     = false; }
            if( option == "--no-network"     ) { this->_network     = false; }
            if( option == "--no-date"        ) { this->_date        = false; }
            if( option == "--no-time"        ) { this->_time        = false; }
        }
    }

    Options::IMPL::IMPL( const IMPL & o ):
        _debug(            o._debug ),
        _help(             o._help ),
        _cpu(              o._cpu ),
        _gpu(              o._gpu ),
        _memory(           o._memory ),
        _temperature(      o._temperature ),
        _battery(          o._battery ),
        _network(          o._network ),
        _date(             o._date ),
        _time(             o._time ),
        _cpuColor(         o._cpuColor ),
        _gpuColor(         o._gpuColor ),
        _memoryColor(      o._memoryColor ),
        _temperatureColor( o._temperatureColor ),
        _batteryColor(     o._batteryColor ),
        _networkColor(     o._networkColor ),
        _dateColor(        o._dateColor ),
        _timeColor(        o._timeColor )
    {}

    Options::IMPL::~IMPL()
    {}
}
