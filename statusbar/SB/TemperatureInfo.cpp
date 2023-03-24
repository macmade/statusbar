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

#include "SB/TemperatureInfo.hpp"
#include "SB/Vector.hpp"
#include "SB/IOHID-Internal.h"
#include <mutex>
#include <thread>
#include <vector>
#include <IOKit/hidsystem/IOHIDEventSystemClient.h>
#include <IOKit/hidsystem/IOHIDServiceClient.h>

namespace SB
{
    class TemperatureInfo::IMPL
    {
        public:

            IMPL( double temperature );
            IMPL( const IMPL & o );
            ~IMPL();

            double _temperature;

            static void                  init();
            static void                  observe() __attribute__( ( noreturn ) );
            static TemperatureInfo       getTemperatureInfo();
            static std::vector< double > readHIDSensors( double & tcal );
            static std::vector< double > readSMCSensors();

            static std::recursive_mutex    * rmtx;
            static TemperatureInfo         * info;
            static bool                      observing;
            static IOHIDEventSystemClientRef hidClient;
    };

    void TemperatureInfo::startObserving()
    {
        TemperatureInfo::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        if( IMPL::observing )
        {
            return;
        }

        IMPL::observing = true;

        std::thread( [] { TemperatureInfo::IMPL::observe(); } ).detach();
    }

    TemperatureInfo TemperatureInfo::current()
    {
        TemperatureInfo::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        return *( IMPL::info );
    }

    TemperatureInfo::TemperatureInfo( double temperature ):
        impl( std::make_unique< IMPL >( temperature ) )
    {}

    TemperatureInfo::TemperatureInfo( const TemperatureInfo & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    TemperatureInfo::TemperatureInfo( TemperatureInfo && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    TemperatureInfo::~TemperatureInfo()
    {}

    TemperatureInfo & TemperatureInfo::operator =( TemperatureInfo o )
    {
        swap( *( this ), o );

        return *( this );
    }

    double TemperatureInfo::temperature() const
    {
        return this->impl->_temperature;
    }

    void swap( TemperatureInfo & o1, TemperatureInfo & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    TemperatureInfo::IMPL::IMPL( double temperature ):
        _temperature( temperature )
    {}

    TemperatureInfo::IMPL::IMPL( const IMPL & o ):
        _temperature( o._temperature )
    {}

    TemperatureInfo::IMPL::~IMPL()
    {}

    void TemperatureInfo::IMPL::init()
    {
        static std::once_flag once;

        std::call_once
        (
            once,
            []
            {
                rmtx      = new std::recursive_mutex();
                info      = new TemperatureInfo( 0 );
                hidClient = IOHIDEventSystemClientCreate( kCFAllocatorDefault );
            }
        );
    }

    void TemperatureInfo::IMPL::observe()
    {
        while( true )
        {
            TemperatureInfo current = IMPL::getTemperatureInfo();

            {
                std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

                delete IMPL::info;

                IMPL::info = new TemperatureInfo( current );
            }

            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        }
    }

    TemperatureInfo TemperatureInfo::IMPL::getTemperatureInfo()
    {
        double tcal = 0;
        double hid  = Vector::reduce< double, double >( IMPL::readHIDSensors( tcal ), 0.0, []( double a, double b ) { return ( a > b ) ? a : b; } );
        double smc  = Vector::reduce< double, double >
        (
            IMPL::readSMCSensors(),
            0.0,
            []( double a, double b )
            {
                return ( a > b ) ? a : b;
            }
        );

        return ( hid > smc ) ? hid : smc;
    }

    std::vector< double > TemperatureInfo::IMPL::readHIDSensors( double & tcal )
    {
        tcal = 0;

        if( IMPL::hidClient == nullptr )
        {
            return {};
        }

        CFDictionaryRef filter = nullptr;

        {
            int64_t     page    = static_cast< int64_t >( IOHIDPage::IOHIDPageAppleVendor );
            int64_t     usage   = static_cast< int64_t >( IOHIDUsageAppleVendor::IOHIDUsageAppleVendorTemperatureSensor );
            CFNumberRef cfPage  = CFNumberCreate( nullptr, kCFNumberSInt64Type, &page );
            CFNumberRef cfUsage = CFNumberCreate( nullptr, kCFNumberSInt64Type, &usage );

            CFTypeRef keys[]   = { CFSTR( "PrimaryUsagePage" ), CFSTR( "PrimaryUsage" ) };
            CFTypeRef values[] = { cfPage, cfUsage };

            filter = CFDictionaryCreate( nullptr, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

            CFRelease( cfPage );
            CFRelease( cfUsage );
        }

        IOHIDEventSystemClientSetMatching( IMPL::hidClient, filter );
        CFRelease( filter );

        CFArrayRef services = IOHIDEventSystemClientCopyServices( IMPL::hidClient );

        if( services == nullptr )
        {
            return {};
        }

        std::vector< double > values;

        for( CFIndex i = 0; i < CFArrayGetCount( services ); i++ )
        {
            IOHIDServiceClientRef service = reinterpret_cast< IOHIDServiceClientRef >( const_cast< void * >( CFArrayGetValueAtIndex( services, i ) ) );
            CFStringRef           name    = reinterpret_cast< CFStringRef >( IOHIDServiceClientCopyProperty( service, CFSTR( "Product" ) ) );
            CFTypeRef             event   = IOHIDServiceClientCopyEvent( service, IOHIDEvent::IOHIDEventTypeTemperature, 0, 0 );

            if( name != nullptr && event != nullptr )
            {
                double value = IOHIDEventGetFloatValue( event, static_cast< int64_t >( IOHIDEventField::IOHIDEventFieldTemperatureLevel ) );

                if( CFStringHasSuffix( name, CFSTR( "tcal" ) ) )
                {
                    tcal = value;
                }
                else
                {
                    values.push_back( value );
                }
            }

            if( name  != nullptr ) { CFRelease( name ); }
            if( event != nullptr ) { CFRelease( event ); }
        }

        CFRelease( services );

        return values;
    }

    std::vector< double > TemperatureInfo::IMPL::readSMCSensors()
    {
        return {};
    }

    std::recursive_mutex    * TemperatureInfo::IMPL::rmtx      = nullptr;
    TemperatureInfo         * TemperatureInfo::IMPL::info      = nullptr;
    bool                      TemperatureInfo::IMPL::observing = false;
    IOHIDEventSystemClientRef TemperatureInfo::IMPL::hidClient = nullptr;
}
