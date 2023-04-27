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

#include "SleepManager.hpp"
#include <mutex>
#include <utility>
#include <vector>
#include <thread>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <mach/mach_port.h>
#include <mach/mach_interface.h>
#include <mach/mach_init.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

namespace SB
{
    class SleepManager::IMPL
    {
        public:

            IMPL();
            IMPL( const IMPL & o );
            ~IMPL();

            void run();

            static void sleepCallback( void * connection, io_service_t service, natural_t messageType, void * context );
                   void sleepCallback( Event e );

            std::vector< std::pair< UUID, std::function< void( Event ) > > > _callbacks;
            CFRunLoopRef                                                     _rl;
            std::recursive_mutex                                             _rmtx;
            io_connect_t                                                     _rootPort;
    };

    SleepManager & SleepManager::shared()
    {
        static SleepManager * instance = nullptr;
        static std::once_flag once;

        std::call_once
        (
            once,
            [ & ]
            {
                instance = new SleepManager();
            }
        );

        return *( instance );
    }

    SleepManager::SleepManager():
        impl( std::make_unique< IMPL >() )
    {}

    SleepManager::~SleepManager()
    {}

    UUID SleepManager::subscribe( const std::function< void( Event ) > & callback )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );
        UUID                                    uuid;

        this->impl->_callbacks.push_back( { uuid, callback } );

        return uuid;
    }

    void SleepManager::unsubscribe( const UUID & uuid )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );

        this->impl->_callbacks.erase
        (
            std::remove_if
            (
                this->impl->_callbacks.begin(),
                this->impl->_callbacks.end(),
                [ & ]( const auto & p )
                {
                    return p.first == uuid;
                }
            ),
            this->impl->_callbacks.end()
        );
    }

    SleepManager::IMPL::IMPL():
        _rl( nullptr )
    {
        this->run();
    }

    SleepManager::IMPL::IMPL( const IMPL & o ):
        _callbacks( o._callbacks ),
        _rl( nullptr )
    {
        this->run();
    }

    SleepManager::IMPL::~IMPL()
    {
        if( this->_rl != nullptr )
        {
            CFRunLoopStop( this->_rl );
        }
    }

    void SleepManager::IMPL::run()
    {
        std::thread
        (
            [ & ]
            {
                IONotificationPortRef notifyPort;
                io_object_t           notifier;
                io_connect_t          rootPort = IORegisterForSystemPower( this, &notifyPort, IMPL::sleepCallback, &notifier );

                if( rootPort == MACH_PORT_NULL )
                {
                    return;
                }

                CFRunLoopRef rl = CFRunLoopGetCurrent();

                {
                    std::lock_guard< std::recursive_mutex > l( this->_rmtx );

                    this->_rl       = rl;
                    this->_rootPort = rootPort;
                }

                CFRunLoopAddSource( rl, IONotificationPortGetRunLoopSource( notifyPort ), kCFRunLoopCommonModes );
                CFRunLoopRun();
                CFRunLoopRemoveSource( rl, IONotificationPortGetRunLoopSource( notifyPort ), kCFRunLoopCommonModes );
                IODeregisterForSystemPower( &notifier );
                IOServiceClose( rootPort );
                IONotificationPortDestroy( notifyPort );
            }
        )
        .detach();
    }

    void SleepManager::IMPL::sleepCallback( void * context, io_service_t service, natural_t messageType, void * messageArgument )
    {
        ( void )service;

        IMPL * o = static_cast< IMPL * >( context );

        if( messageType == kIOMessageCanSystemSleep || messageType == kIOMessageSystemWillSleep )
        {
            IOAllowPowerChange( o->_rootPort, reinterpret_cast< long >( messageArgument ) );
        }

        switch( messageType )
        {
            case kIOMessageCanSystemSleep:     o->sleepCallback( Event::CanSleep );    break;
            case kIOMessageSystemWillSleep:    o->sleepCallback( Event::WillSleep );   break;
            case kIOMessageSystemWillPowerOn:  o->sleepCallback( Event::WillPowerOn ); break;
            case kIOMessageSystemHasPoweredOn: o->sleepCallback( Event::DidPowerOn );  break;
            default:                                                                   break;
        }
    }

    void SleepManager::IMPL::sleepCallback( Event e )
    {
        std::lock_guard< std::recursive_mutex > l( this->_rmtx );

        for( const auto & p: this->_callbacks )
        {
            p.second( e );
        }
    }
}
