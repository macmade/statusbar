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

#include "SB/Instruments/NetworkInfo.hpp"
#include "SB/UpdateQueue.hpp"
#include "SB/SleepManager.hpp"
#include <mutex>
#include <chrono>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

namespace SB
{
    class NetworkInfo::IMPL
    {
        public:

            IMPL( const std::string & name, const std::string & address );
            IMPL( const IMPL & o );
            ~IMPL();

            friend void swap( IMPL & o1, IMPL & o2 ) noexcept
            {
                using std::swap;

                swap( o1._name,    o2._name );
                swap( o1._address, o2._address );
            }

            std::string _name;
            std::string _address;

            static void        init();
            static void        observe();
            static IMPL        getNetworkInfo();
            static void        dynamicStoreCallback( SCDynamicStoreRef store, CFArrayRef changedKeys, void * context );

            static std::recursive_mutex * rmtx;
            static NetworkInfo          * info;
            static bool                   observing;
    };

    void NetworkInfo::startObserving()
    {
        NetworkInfo::IMPL::init();

        {
            std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

            if( IMPL::observing )
            {
                return;
            }

            IMPL::observing = true;
        }

        /* The notification source only fires on change, so sample the current value once. */
        IMPL::observe();

        /* Fall back to polling if event-driven monitoring cannot be set up. */
        auto registerPoll = []
        {
            SB::UpdateQueue::shared().registerUpdate( [] { IMPL::observe(); }, std::chrono::seconds( 5 ) );
        };

        SCDynamicStoreRef store = SCDynamicStoreCreate( nullptr, CFSTR( "statusbar" ), IMPL::dynamicStoreCallback, nullptr );

        if( store == nullptr )
        {
            registerPoll();

            return;
        }

        CFStringRef key = SCDynamicStoreKeyCreateNetworkGlobalEntity( nullptr, kSCDynamicStoreDomainState, kSCEntNetIPv4 );

        if( key == nullptr )
        {
            CFRelease( store );
            registerPoll();

            return;
        }

        const void * keys[] = { key };
        CFArrayRef   watch  = CFArrayCreate( nullptr, keys, 1, &kCFTypeArrayCallBacks );
        Boolean      set    = SCDynamicStoreSetNotificationKeys( store, watch, nullptr );

        CFRelease( watch );
        CFRelease( key );

        if( set == false )
        {
            CFRelease( store );
            registerPoll();

            return;
        }

        CFRunLoopSourceRef source = SCDynamicStoreCreateRunLoopSource( nullptr, store, 0 );

        if( source == nullptr )
        {
            CFRelease( store );
            registerPoll();

            return;
        }

        /* Event-driven: host the source on SleepManager's shared run loop. The source
         * (which retains the store) lives for the process lifetime and is intentionally
         * never released (see the thread/singleton lifetime model documented in M18).
         */
        SB::SleepManager::shared().addRunLoopSource( source );
    }

    NetworkInfo NetworkInfo::current()
    {
        NetworkInfo::IMPL::init();
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        return *( IMPL::info );
    }

    NetworkInfo::NetworkInfo( const std::string & name, const std::string & address ):
        impl( std::make_unique< IMPL >( name, address ) )
    {}

    NetworkInfo::NetworkInfo( const NetworkInfo & o ):
        impl( std::make_unique< IMPL >( *( o.impl ) ) )
    {}

    NetworkInfo::NetworkInfo( NetworkInfo && o ) noexcept:
        impl( std::move( o.impl ) )
    {}

    NetworkInfo::~NetworkInfo()
    {}

    NetworkInfo & NetworkInfo::operator =( NetworkInfo o )
    {
        swap( *( this ), o );

        return *( this );
    }

    std::string NetworkInfo::name() const
    {
        return this->impl->_name;
    }

    std::string NetworkInfo::address() const
    {
        return this->impl->_address;
    }

    void swap( NetworkInfo & o1, NetworkInfo & o2 )
    {
        using std::swap;

        swap( o1.impl, o2.impl );
    }

    NetworkInfo::IMPL::IMPL( const std::string & name, const std::string & address ):
        _name(    name ),
        _address( address )
    {}

    NetworkInfo::IMPL::IMPL( const IMPL & o ):
        _name(    o._name ),
        _address( o._address )
    {}

    NetworkInfo::IMPL::~IMPL()
    {}

    void NetworkInfo::IMPL::init()
    {
        static std::once_flag once;

        std::call_once
        (
            once,
            []
            {
                rmtx = new std::recursive_mutex();
                info = new NetworkInfo( "", "" );
            }
        );
    }

    void NetworkInfo::IMPL::observe()
    {
        IMPL current = IMPL::getNetworkInfo();

        {
            std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

            swap( *( IMPL::info->impl ), current );
        }
    }

    void NetworkInfo::IMPL::dynamicStoreCallback( SCDynamicStoreRef store, CFArrayRef changedKeys, void * context )
    {
        ( void )store;
        ( void )changedKeys;
        ( void )context;

        IMPL::observe();
    }

    NetworkInfo::IMPL NetworkInfo::IMPL::getNetworkInfo()
    {
        struct ifaddrs * interfaces = nullptr;

        if( getifaddrs( &interfaces ) != 0 )
        {
            return { "", "" };
        }

        struct ifaddrs * interface = interfaces;

        while( interface != nullptr )
        {
            if( interface->ifa_addr != nullptr && interface->ifa_addr->sa_family == AF_INET )
            {
                std::string          name    = interface->ifa_name;
                struct sockaddr_in * in      = reinterpret_cast< struct sockaddr_in * >( interface->ifa_addr );
                std::string          address = inet_ntoa( in->sin_addr );

                if( name != "lo0" )
                {
                    freeifaddrs( interfaces );

                    return { name, address };
                }
            }

            interface = interface->ifa_next;
        }

        freeifaddrs( interfaces );

        return { "", "" };
    }

    std::recursive_mutex * NetworkInfo::IMPL::rmtx      = nullptr;
    NetworkInfo          * NetworkInfo::IMPL::info      = nullptr;
    bool                   NetworkInfo::IMPL::observing = false;
}
