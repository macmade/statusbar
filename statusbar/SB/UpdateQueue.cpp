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

#include "SB/UpdateQueue.hpp"
#include "SB/SleepManager.hpp"
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

namespace SB
{
    class UpdateQueue::IMPL
    {
        public:

            IMPL();
            ~IMPL();

            struct Update
            {
                std::function< void() >               function;
                std::chrono::milliseconds             interval;
                std::chrono::steady_clock::time_point lastRun;
            };

            std::vector< Update > _updates;
            std::recursive_mutex  _rmtx;
            std::atomic< bool >   _sleeping;
            UUID                  _sleepRegistration;
    };

    UpdateQueue & UpdateQueue::shared()
    {
        /* Process-lifetime singleton: allocated once and never freed. The widget runs
         * until 'q'/kill, so there is deliberately no shutdown/teardown path.
         */
        static UpdateQueue  * queue;
        static std::once_flag once;

        std::call_once
        (
            once,
            [ & ]
            {
                queue = new UpdateQueue();
            }
        );

        return *( queue );
    }

    UpdateQueue::UpdateQueue():
        impl( std::make_unique< IMPL >() )
    {}

    UpdateQueue::~UpdateQueue()
    {}

    void UpdateQueue::registerUpdate( const std::function< void() > & f, std::chrono::milliseconds interval )
    {
        std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );

        this->impl->_updates.push_back( { f, interval, std::chrono::steady_clock::time_point() } );
    }

    UpdateQueue::IMPL::IMPL():
        _sleeping( false )
    {
        this->_sleepRegistration = SleepManager::shared().subscribe
        (
            [ & ]( SleepManager::Event e )
            {
                if( e == SleepManager::Event::WillSleep )
                {
                    this->_sleeping = true;
                }
                else if( e == SleepManager::Event::DidPowerOn )
                {
                    this->_sleeping = false;
                }
            }
        );
    }

    UpdateQueue::IMPL::~IMPL()
    {
        SleepManager::shared().unsubscribe( this->_sleepRegistration );
    }

    void UpdateQueue::runDue()
    {
        if( this->impl->_sleeping )
        {
            return;
        }

        std::vector< std::function< void() > > functions;
        std::chrono::steady_clock::time_point  now = std::chrono::steady_clock::now();

        {
            std::lock_guard< std::recursive_mutex > l( this->impl->_rmtx );

            for( auto & update: this->impl->_updates )
            {
                if( now - update.lastRun >= update.interval )
                {
                    update.lastRun = now;

                    functions.push_back( update.function );
                }
            }
        }

        for( const auto & f: functions )
        {
            f();
        }
    }
}
