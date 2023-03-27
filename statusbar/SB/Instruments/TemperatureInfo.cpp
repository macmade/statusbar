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

#include "SB/Instruments/TemperatureInfo.hpp"
#include "SB/Helpers/Vector.hpp"
#include "SB/Helpers/String.hpp"
#include "SB/UpdateQueue.hpp"
#include "SB/Instruments/Internal/IOHID.hpp"
#include "SB/Instruments/Internal/SMC.hpp"
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
            static void                  observe();
            static TemperatureInfo       getTemperatureInfo();
            static std::vector< double > readHIDSensors( double & tcal );
            static std::vector< double > readSMCSensors();

            static std::recursive_mutex                 * rmtx;
            static TemperatureInfo                      * info;
            static bool                                   observing;
            static IOHIDEventSystemClientRef              hidClient;
            static io_connect_t                           smcConnection;
            static std::vector< uint32_t >              * smcKeys;
            static std::map< uint32_t, SMCKeyInfoData > * smcCache;

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

        SB::UpdateQueue::shared().registerUpdate( [] { IMPL::observe(); } );
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
                IMPL::rmtx      = new std::recursive_mutex();
                IMPL::info      = new TemperatureInfo( 0 );
                IMPL::hidClient = IOHIDEventSystemClientCreate( kCFAllocatorDefault );
                IMPL::smcKeys   = new std::vector< uint32_t >();
                IMPL::smcCache  = new std::map< uint32_t, SMCKeyInfoData >();

                {
                    io_service_t smc = IOServiceGetMatchingService( kIOMasterPortDefault, IOServiceMatching( "AppleSMC" ) );

                    if( smc != IO_OBJECT_NULL )
                    {
                        io_connect_t  connection = IO_OBJECT_NULL;
                        kern_return_t result     = IOServiceOpen( smc, mach_task_self(), 0, &connection );

                        if( result == kIOReturnSuccess && connection != IO_OBJECT_NULL )
                        {
                            IMPL::smcConnection = connection;
                        }
                    }
                }
            }
        );
    }

    void TemperatureInfo::IMPL::observe()
    {
        TemperatureInfo current = IMPL::getTemperatureInfo();

        {
            std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

            delete IMPL::info;

            IMPL::info = new TemperatureInfo( current );
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
            [ & ]( double a, double b )
            {
                if( static_cast< int64_t >( tcal * 100 ) == static_cast< int64_t >( b * 100 ) )
                {
                    return a;
                }

                return ( a > b ) ? a : b;
            }
        );

        return ( hid > smc ) ? hid : smc;
    }

    std::vector< double > TemperatureInfo::IMPL::readHIDSensors( double & tcal )
    {
        std::vector< double >           sensors;
        std::map< std::string, double > values = IOHID::read
        (
            IMPL::hidClient,
            static_cast< int64_t >( IOHIDPage::IOHIDPageAppleVendor ),
            static_cast< int64_t >( IOHIDUsageAppleVendor::IOHIDUsageAppleVendorTemperatureSensor ),
            static_cast< int64_t >( IOHIDEvent::IOHIDEventTypeTemperature ),
            static_cast< int64_t >( IOHIDEventField::IOHIDEventFieldTemperatureLevel )
        );

        tcal = 0;

        for( const auto & p: values )
        {
            if( String::hasSuffix( p.first, "tcal" ) )
            {
                tcal = p.second;
            }
            else
            {
                sensors.push_back( p.second );
            }
        }

        return sensors;
    }

    std::vector< double > TemperatureInfo::IMPL::readSMCSensors()
    {
        if( IMPL::smcConnection == IO_OBJECT_NULL )
        {
            return {};
        }

        if( IMPL::smcKeys->size() == 0 )
        {
            uint32_t count = SMC::readSMCKeyCount( IMPL::smcConnection, *( IMPL::smcCache ) );

            for( uint32_t i = 0; i < count; i++ )
            {
                uint32_t key = 0;

                if( SMC::readSMCKey( IMPL::smcConnection, key, i ) == false )
                {
                    continue;
                }

                if( key != 0 )
                {
                    IMPL::smcKeys->push_back( key );
                }
            }
        }

        std::vector< double > sensors;

        for( auto key: *( IMPL::smcKeys ) )
        {
            SMCKeyInfoData keyInfo;
            uint8_t        data[ 32 ];
            uint32_t       size = sizeof( data );

            if( SMC::readSMCKey( IMPL::smcConnection, key, data, size, keyInfo, *( IMPL::smcCache ) ) == false )
            {
                continue;
            }

            std::string name = String::fourCC( key );
            std::string type = String::fourCC( keyInfo.dataType );

            if( name.length() == 0 || name[ 0 ] != 'T' )
            {
                continue;
            }

            double value = 0;

            if( type == "ioft" )
            {
                value = SMC::readIOFloat( data, size );
            }
            else if( type == "flt " )
            {
                value = SMC::readFloat32( data, size );
            }

            if( value > 0 && value < 120 )
            {
                sensors.push_back( value );
            }
            else
            {
                continue;
            }
        }

        return sensors;
    }

    std::recursive_mutex                 * TemperatureInfo::IMPL::rmtx          = nullptr;
    TemperatureInfo                      * TemperatureInfo::IMPL::info          = nullptr;
    bool                                   TemperatureInfo::IMPL::observing     = false;
    IOHIDEventSystemClientRef              TemperatureInfo::IMPL::hidClient     = nullptr;
    io_connect_t                           TemperatureInfo::IMPL::smcConnection = 0;
    std::vector< uint32_t >              * TemperatureInfo::IMPL::smcKeys       = nullptr;
    std::map< uint32_t, SMCKeyInfoData > * TemperatureInfo::IMPL::smcCache      = nullptr;
}
