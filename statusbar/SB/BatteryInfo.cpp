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

#include "SB/BatteryInfo.hpp"
#include <mutex>
#include <thread>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>

namespace SB
{
    class BatteryInfo::IMPL
    {
        public:

            IMPL( int64_t capacity, bool charging, bool isAvailable );
            IMPL( const IMPL & o );
            ~IMPL();

            int64_t _capacity;
            bool    _isCharging;
            bool    _isAvailable;

            static void        init();
            static void        observe() __attribute__( ( noreturn ) );
            static BatteryInfo getBatteryInfo();

            static std::recursive_mutex * rmtx;
            static BatteryInfo          * info;
            static bool                   observing;
    };

    void BatteryInfo::startObserving()
    {
        BatteryInfo::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        if( IMPL::observing )
        {
            return;
        }

        IMPL::observing = true;

        std::thread( [] { BatteryInfo::IMPL::observe(); } ).detach();
    }

    BatteryInfo BatteryInfo::current()
    {
        BatteryInfo::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        return *( IMPL::info );
    }

    BatteryInfo::BatteryInfo( int64_t capacity, bool isCharging, bool isAvailable ):
        impl( std::make_unique< IMPL >( capacity, isCharging, isAvailable ) )
    {}

    BatteryInfo::BatteryInfo( const BatteryInfo & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    BatteryInfo::BatteryInfo( BatteryInfo && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    BatteryInfo::~BatteryInfo()
    {}

    BatteryInfo & BatteryInfo::operator =( BatteryInfo o )
    {
        swap( *( this ), o );

        return *( this );
    }

    int64_t BatteryInfo::capacity() const
    {
        return this->impl->_capacity;
    }

    bool BatteryInfo::isCharging() const
    {
        return this->impl->_isCharging;
    }

    bool BatteryInfo::isAvailable() const
    {
        return this->impl->_isAvailable;
    }

    void swap( BatteryInfo & o1, BatteryInfo & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    BatteryInfo::IMPL::IMPL( int64_t capacity, bool isCharging, bool isAvailable ):
        _capacity(    capacity ),
        _isCharging(  isCharging ),
        _isAvailable( isAvailable )
    {}

    BatteryInfo::IMPL::IMPL( const IMPL & o ):
        _capacity(    o._capacity ),
        _isCharging(  o._isCharging ),
        _isAvailable( o._isAvailable )
    {}

    BatteryInfo::IMPL::~IMPL()
    {}

    void BatteryInfo::IMPL::init()
    {
        static std::once_flag once;

        std::call_once
        (
            once,
            []
            {
                rmtx = new std::recursive_mutex();
                info = new BatteryInfo( 0, false, false );
            }
        );
    }

    void BatteryInfo::IMPL::observe()
    {
        while( true )
        {
            BatteryInfo current = IMPL::getBatteryInfo();

            {
                std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

                delete IMPL::info;

                IMPL::info = new BatteryInfo( current );
            }

            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        }
    }

    BatteryInfo BatteryInfo::IMPL::getBatteryInfo()
    {
        CFTypeRef blob = IOPSCopyPowerSourcesInfo();

        if( blob == nullptr )
        {
            return { 0, false, false };
        }

        CFArrayRef sources = IOPSCopyPowerSourcesList( blob );

        if( sources == nullptr )
        {
            CFRelease( blob );

            return { 0, false, false };
        }

        for( CFIndex i = 0; i < CFArrayGetCount( sources ); i++ )
        {
            CFDictionaryRef description = IOPSGetPowerSourceDescription( blob, CFArrayGetValueAtIndex( sources, i ) );

            if( description == nullptr )
            {
                continue;
            }

            CFStringRef  type     = reinterpret_cast< CFStringRef  >( CFDictionaryGetValue( description, CFSTR( "Type" ) ) );
            CFBooleanRef charging = reinterpret_cast< CFBooleanRef >( CFDictionaryGetValue( description, CFSTR( "Is Charging" ) ) );
            CFNumberRef  capacity = reinterpret_cast< CFNumberRef  >( CFDictionaryGetValue( description, CFSTR( "Current Capacity" ) ) );

            if( type == nullptr || CFGetTypeID( type ) != CFStringGetTypeID() || CFEqual( type, CFSTR( "InternalBattery" ) ) == false )
            {
                continue;
            }

            if( charging == nullptr || CFGetTypeID( charging ) != CFBooleanGetTypeID() )
            {
                continue;
            }

            if( capacity == nullptr || CFGetTypeID( capacity ) != CFNumberGetTypeID() )
            {
                continue;
            }

            int64_t currentCapacity = 0;

            CFNumberGetValue( capacity, kCFNumberSInt64Type, &currentCapacity );

            CFRelease( sources );
            CFRelease( blob );

            return { currentCapacity, CFBooleanGetValue( charging ) == 1, true };
        }

        CFRelease( sources );
        CFRelease( blob );

        return { 0, false, false };
    }

    std::recursive_mutex * BatteryInfo::IMPL::rmtx      = nullptr;
    BatteryInfo          * BatteryInfo::IMPL::info      = nullptr;
    bool                   BatteryInfo::IMPL::observing = false;
}
