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

#include "SB/Instruments/GPULoad.hpp"
#include "SB/UpdateQueue.hpp"
#include <mutex>
#include <thread>
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOAccelClientConnect.h>
#include <CoreFoundation/CoreFoundation.h>

namespace SB
{
    class GPULoad::IMPL
    {
        public:

            IMPL( double percent );
            IMPL( const IMPL & o );
            ~IMPL();

            double _percent;

            static void    init();
            static void    observe();
            static GPULoad getGPULoad();

            static std::recursive_mutex * rmtx;
            static GPULoad              * info;
            static bool                   observing;
    };

    void GPULoad::startObserving()
    {
        GPULoad::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        if( IMPL::observing )
        {
            return;
        }

        IMPL::observing = true;

        SB::UpdateQueue::shared().registerUpdate( [] { IMPL::observe(); } );
    }

    GPULoad GPULoad::current()
    {
        GPULoad::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        return *( IMPL::info );
    }

    GPULoad::GPULoad( double percent ):
        impl( std::make_unique< IMPL >( percent ) )
    {}

    GPULoad::GPULoad( const GPULoad & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    GPULoad::GPULoad( GPULoad && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    GPULoad::~GPULoad()
    {}

    GPULoad & GPULoad::operator =( GPULoad o )
    {
        swap( *( this ), o );

        return *( this );
    }

    double GPULoad::percent() const
    {
        return this->impl->_percent;
    }

    void swap( GPULoad & o1, GPULoad & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    GPULoad::IMPL::IMPL( double percent ):
        _percent( percent )
    {}

    GPULoad::IMPL::IMPL( const IMPL & o ):
        _percent( o._percent )
    {}

    GPULoad::IMPL::~IMPL()
    {}

    void GPULoad::IMPL::init()
    {
        static std::once_flag once;

        std::call_once
        (
            once,
            []
            {
                rmtx = new std::recursive_mutex();
                info = new GPULoad( 0 );
            }
        );
    }

    void GPULoad::IMPL::observe()
    {
        GPULoad current = IMPL::getGPULoad();

        {
            std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

            delete IMPL::info;

            IMPL::info = new GPULoad( current );
        }
    }

    GPULoad GPULoad::IMPL::getGPULoad()
    {
        CFMutableDictionaryRef matching = IOServiceMatching( kIOAcceleratorClassName );
        io_iterator_t          iterator;

        if( IOServiceGetMatchingServices( kIOMasterPortDefault, matching, &iterator ) != kIOReturnSuccess )
        {
            return { -1 };
        }

        io_registry_entry_t entry;
        double              device   = -1;
        double              renderer = -1;
        double              tiler    = -1;

        while( ( entry = IOIteratorNext( iterator ) ) )
        {
            CFMutableDictionaryRef properties;

            if( IORegistryEntryCreateCFProperties( entry, &properties, kCFAllocatorDefault, kNilOptions ) != kIOReturnSuccess )
            {
                IOObjectRelease( entry );

                continue;
            }

            CFDictionaryRef stats = static_cast< CFDictionaryRef >( CFDictionaryGetValue( properties, CFSTR( "PerformanceStatistics" ) ) );

            if( stats != nullptr && CFGetTypeID( stats ) == CFDictionaryGetTypeID() )
            {
                CFNumberRef cfDevice   = static_cast< CFNumberRef >( CFDictionaryGetValue( stats, CFSTR( "Device Utilization %" ) ) );
                CFNumberRef cfRenderer = static_cast< CFNumberRef >( CFDictionaryGetValue( stats, CFSTR( "Renderer Utilization %" ) ) );
                CFNumberRef cfTiler    = static_cast< CFNumberRef >( CFDictionaryGetValue( stats, CFSTR( "Tiler Utilization %" ) ) );

                if( cfDevice != nullptr && CFGetTypeID( cfDevice ) == CFNumberGetTypeID() )
                {
                    CFNumberGetValue( cfDevice, kCFNumberDoubleType, &device );
                }

                if( cfRenderer != nullptr && CFGetTypeID( cfRenderer ) == CFNumberGetTypeID() )
                {
                    CFNumberGetValue( cfRenderer, kCFNumberDoubleType, &renderer );
                }

                if( cfTiler != nullptr && CFGetTypeID( cfTiler ) == CFNumberGetTypeID() )
                {
                    CFNumberGetValue( cfTiler, kCFNumberDoubleType, &tiler );
                }
            }

            CFRelease( properties );
            IOObjectRelease( entry );
        }

        IOObjectRelease( iterator );
        
        return { std::max( std::max( device, renderer ), tiler ) };
    }

    std::recursive_mutex * GPULoad::IMPL::rmtx      = nullptr;
    GPULoad              * GPULoad::IMPL::info      = nullptr;
    bool                   GPULoad::IMPL::observing = false;
}
