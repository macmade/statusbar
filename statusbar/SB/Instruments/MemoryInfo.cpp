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

#include "SB/Instruments/MemoryInfo.hpp"
#include "SB/UpdateQueue.hpp"
#include <mutex>
#include <thread>
#include <sys/sysctl.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <mach/task_info.h>
#include <mach/task.h>

namespace SB
{
    class MemoryInfo::IMPL
    {
        public:

            IMPL( uint64_t total, uint64_t wired, uint64_t active, uint64_t inactive, uint64_t free, uint64_t purgeable, uint64_t speculative );
            IMPL( const IMPL & o );
            ~IMPL();

            uint64_t _total;
            uint64_t _wired;
            uint64_t _active;
            uint64_t _inactive;
            uint64_t _free;
            uint64_t _purgeable;
            uint64_t _speculative;

            static void       init();
            static void       observe();
            static MemoryInfo getMemoryInfo();

            static std::recursive_mutex * rmtx;
            static MemoryInfo           * info;
            static bool                   observing;
    };

    void MemoryInfo::startObserving()
    {
        MemoryInfo::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        if( IMPL::observing )
        {
            return;
        }

        IMPL::observing = true;

        SB::UpdateQueue::shared().registerUpdate( [] { IMPL::observe(); } );
    }

    MemoryInfo MemoryInfo::current()
    {
        MemoryInfo::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        return *( IMPL::info );
    }

    MemoryInfo::MemoryInfo( uint64_t total, uint64_t wired, uint64_t active, uint64_t inactive, uint64_t free, uint64_t purgeable, uint64_t speculative ):
        impl( std::make_unique< IMPL >( total, wired, active, inactive, free, purgeable, speculative ) )
    {}

    MemoryInfo::MemoryInfo( const MemoryInfo & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    MemoryInfo::MemoryInfo( MemoryInfo && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    MemoryInfo::~MemoryInfo()
    {}

    MemoryInfo & MemoryInfo::operator =( MemoryInfo o )
    {
        swap( *( this ), o );

        return *( this );
    }

    uint64_t MemoryInfo::total() const
    {
        return this->impl->_total;
    }

    uint64_t MemoryInfo::wired() const
    {
        return this->impl->_wired;
    }

    uint64_t MemoryInfo::active() const
    {
        return this->impl->_active;
    }

    uint64_t MemoryInfo::inactive() const
    {
        return this->impl->_inactive;
    }

    uint64_t MemoryInfo::free() const
    {
        return this->impl->_free;
    }

    uint64_t MemoryInfo::purgeable() const
    {
        return this->impl->_purgeable;
    }

    uint64_t MemoryInfo::speculative() const
    {
        return this->impl->_speculative;
    }

    uint64_t MemoryInfo::used() const
    {
        return ( ( this->wired() + this->active() + this->inactive() ) - this->speculative() ) - this->purgeable();
    }

    double MemoryInfo::percentUsed() const
    {
        uint64_t total = this->total();
        uint64_t used  = this->used();

        if( total == 0 )
        {
            return 0;
        }

        return 100.0 * ( static_cast< double >( used ) / static_cast< double >( total ) );
    }

    void swap( MemoryInfo & o1, MemoryInfo & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    MemoryInfo::IMPL::IMPL( uint64_t total, uint64_t wired, uint64_t active, uint64_t inactive, uint64_t free, uint64_t purgeable, uint64_t speculative ):
        _total(       total ),
        _wired(       wired ),
        _active(      active ),
        _inactive(    inactive ),
        _free(        free ),
        _purgeable(   purgeable ),
        _speculative( speculative )
    {}

    MemoryInfo::IMPL::IMPL( const IMPL & o ):
        _total(       o._total ),
        _wired(       o._wired ),
        _active(      o._active ),
        _inactive(    o._inactive ),
        _free(        o._free ),
        _purgeable(   o._purgeable ),
        _speculative( o._speculative )
    {}

    MemoryInfo::IMPL::~IMPL()
    {}

    void MemoryInfo::IMPL::init()
    {
        static std::once_flag once;

        std::call_once
        (
            once,
            []
            {
                rmtx = new std::recursive_mutex();
                info = new MemoryInfo( 0, 0, 0, 0, 0, 0, 0 );
            }
        );
    }

    void MemoryInfo::IMPL::observe()
    {
        MemoryInfo current = IMPL::getMemoryInfo();

        {
            std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

            delete IMPL::info;

            IMPL::info = new MemoryInfo( current );
        }
    }

    MemoryInfo MemoryInfo::IMPL::getMemoryInfo()
    {
        uint64_t total = 0;

        {
            int      name[ 2 ] = { CTL_HW, HW_MEMSIZE };
            size_t   length    = sizeof( total );

            if( sysctl( name, 2, &total, &length, nullptr, 0 ) != 0 || total == 0 )
            {
                return { 0, 0, 0, 0, 0, 0, 0 };
            }
        }

        int    name[ 2 ] = { CTL_HW, HW_PAGESIZE };
        int    pageSize  = 0;
        size_t length    = sizeof( pageSize );

        if( sysctl( name, 2, &pageSize, &length, nullptr, 0 ) != 0 )
        {
            return { 0, 0, 0, 0, 0, 0, 0 };
        }

        mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
        vm_statistics_data_t   vmStat;

        if( host_statistics( mach_host_self(), HOST_VM_INFO, reinterpret_cast< host_info_t >( &vmStat ), &count ) != KERN_SUCCESS )
        {
            return { 0, 0, 0, 0, 0, 0, 0 };
        }

        uint64_t wired       = static_cast< uint64_t >( pageSize ) * vmStat.wire_count;
        uint64_t active      = static_cast< uint64_t >( pageSize ) * vmStat.active_count;
        uint64_t inactive    = static_cast< uint64_t >( pageSize ) * vmStat.inactive_count;
        uint64_t free        = static_cast< uint64_t >( pageSize ) * vmStat.free_count;
        uint64_t purgeable   = static_cast< uint64_t >( pageSize ) * vmStat.purgeable_count;
        uint64_t speculative = static_cast< uint64_t >( pageSize ) * vmStat.speculative_count;

        return { total, wired, active, inactive, free, purgeable, speculative };
    }

    std::recursive_mutex * MemoryInfo::IMPL::rmtx      = nullptr;
    MemoryInfo           * MemoryInfo::IMPL::info      = nullptr;
    bool                   MemoryInfo::IMPL::observing = false;
}
