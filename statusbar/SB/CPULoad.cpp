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

#include "SB/CPULoad.hpp"
#include "SB/Vector.hpp"
#include <mutex>
#include <thread>
#include <unistd.h>
#include <mach/mach.h>

namespace SB
{
    class CPULoad::IMPL
    {
        public:

            IMPL( double user, double system, double idle, double total );
            IMPL( const IMPL & o );
            ~IMPL();

            double _user;
            double _system;
            double _idle;
            double _total;

            typedef struct
            {
                uint64_t user;
                uint64_t system;
                uint64_t idle;
                uint64_t nice;
            }
            CPULoadInfo;

            static void                       init();
            static void                       observe() __attribute__( ( noreturn ) );
            static std::vector< CPULoadInfo > getCPULoadInfo();

            static std::recursive_mutex * rmtx;
            static CPULoad              * load;
            static bool                   observing;
    };

    void CPULoad::startObserving()
    {
        CPULoad::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        if( IMPL::observing )
        {
            return;
        }

        IMPL::observing = true;

        std::thread( [] { CPULoad::IMPL::observe(); } ).detach();
    }

    CPULoad CPULoad::current()
    {
        CPULoad::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        return *( IMPL::load );
    }

    CPULoad::CPULoad( double user, double system, double idle, double total ):
        impl( std::make_unique< IMPL >( user, system, idle, total ) )
    {}

    CPULoad::CPULoad( const CPULoad & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    CPULoad::CPULoad( CPULoad && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    CPULoad::~CPULoad()
    {}

    CPULoad & CPULoad::operator =( CPULoad o )
    {
        swap( *( this ), o );

        return *( this );
    }

    double CPULoad::user() const
    {
        return this->impl->_user;
    }

    double CPULoad::system() const
    {
        return this->impl->_system;
    }

    double CPULoad::idle() const
    {
        return this->impl->_idle;
    }

    double CPULoad::total() const
    {
        return this->impl->_total;
    }

    void swap( CPULoad & o1, CPULoad & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    CPULoad::IMPL::IMPL( double user, double system, double idle, double total ):
        _user(   user ),
        _system( system ),
        _idle(   idle ),
        _total(  total )
    {}

    CPULoad::IMPL::IMPL( const IMPL & o ):
        _user(   o._user ),
        _system( o._system ),
        _idle(   o._idle ),
        _total(  o._total )
    {}

    CPULoad::IMPL::~IMPL()
    {}

    void CPULoad::IMPL::init()
    {
        static std::once_flag once;

        std::call_once
        (
            once,
            []
            {
                rmtx = new std::recursive_mutex();
                load = new CPULoad( 0, 0, 0, 0 );
            }
        );
    }

    void CPULoad::IMPL::observe()
    {
        while( true )
        {
            std::vector< CPULoadInfo > info1 = IMPL::getCPULoadInfo();

            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

            std::vector< CPULoadInfo > info2 = IMPL::getCPULoadInfo();

            uint64_t user1   = Vector::reduce< uint64_t, CPULoadInfo >( info1, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.user; } );
            uint64_t user2   = Vector::reduce< uint64_t, CPULoadInfo >( info2, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.user; } );
            uint64_t system2 = Vector::reduce< uint64_t, CPULoadInfo >( info2, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.system; } );
            uint64_t system1 = Vector::reduce< uint64_t, CPULoadInfo >( info1, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.system; } );
            uint64_t idle1   = Vector::reduce< uint64_t, CPULoadInfo >( info1, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.idle; } );
            uint64_t idle2   = Vector::reduce< uint64_t, CPULoadInfo >( info2, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.idle; } );
            uint64_t nice1   = Vector::reduce< uint64_t, CPULoadInfo >( info1, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.nice; } );
            uint64_t nice2   = Vector::reduce< uint64_t, CPULoadInfo >( info2, 0, []( uint64_t r, CPULoadInfo v ) -> uint64_t { return r + v.nice; } );

            double user   = static_cast< double >( user2 - user1 );
            double system = static_cast< double >( system2 - system1 );
            double idle   = static_cast< double >( idle2 - idle1 );
            double used   = static_cast< double >( ( user2 - user1 ) + ( system2 - system1 ) + ( nice2 - nice1 ) );
            double total  = used + static_cast< double >( idle2 - idle1 );

            {
                std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

                delete IMPL::load;

                IMPL::load = new CPULoad
                (
                    ( user   / total ) * 100.0,
                    ( system / total ) * 100.0,
                    ( idle   / total ) * 100.0,
                    ( used   / total ) * 100.0
                );
            }
        }
    }

    std::vector< CPULoad::IMPL::CPULoadInfo > CPULoad::IMPL::getCPULoadInfo()
    {
        std::vector< CPULoad::IMPL::CPULoadInfo > info             = {};
        natural_t                                 cpuCount         = 0;
        processor_cpu_load_info_t                 cpuLoadInfo      = nullptr;
        mach_msg_type_number_t                    cpuLoadInfoCount = 0;

        if
        (
            host_processor_info
            (
                mach_host_self(),
                PROCESSOR_CPU_LOAD_INFO,
                &cpuCount,
                reinterpret_cast< processor_info_array_t * >( &cpuLoadInfo ),
                &cpuLoadInfoCount
            )
            == KERN_SUCCESS && cpuLoadInfo != nullptr
        )
        {
            for( natural_t i = 0; i < cpuCount; i++ )
            {
                info.push_back
                (
                    {
                        cpuLoadInfo[ i ].cpu_ticks[ CPU_STATE_USER ],
                        cpuLoadInfo[ i ].cpu_ticks[ CPU_STATE_SYSTEM ],
                        cpuLoadInfo[ i ].cpu_ticks[ CPU_STATE_IDLE ],
                        cpuLoadInfo[ i ].cpu_ticks[ CPU_STATE_NICE ]
                    }
                );
            }
        }

        return info;
    }

    std::recursive_mutex * CPULoad::IMPL::rmtx      = nullptr;
    CPULoad              * CPULoad::IMPL::load      = nullptr;
    bool                   CPULoad::IMPL::observing = false;
}
