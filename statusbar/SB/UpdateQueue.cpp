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
#include <mutex>
#include <vector>
#include <thread>

namespace SB
{
    class UpdateQueue::IMPL
    {
        public:

            IMPL();
            ~IMPL();

            std::vector< std::function< void() > > _functions;

            static std::recursive_mutex * rmtx;
            static UpdateQueue          * queue;
    };

    UpdateQueue & UpdateQueue::shared()
    {
        static std::once_flag once;

        std::call_once
        (
            once,
            []
            {
                IMPL::rmtx  = new std::recursive_mutex();
                IMPL::queue = new UpdateQueue();

                IMPL::queue->run();
            }
        );

        return *( IMPL::queue );
    }

    UpdateQueue::UpdateQueue():
        impl( std::make_unique< IMPL >() )
    {}

    UpdateQueue::~UpdateQueue()
    {}

    void UpdateQueue::registerUpdate( const std::function< void() > & f )
    {
        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

        this->impl->_functions.push_back( f );
    }

    void UpdateQueue::run()
    {
        std::thread
        (
            [ this ]
            {
                while( true )
                {
                    std::vector< std::function< void() > > functions;

                    {
                        std::lock_guard< std::recursive_mutex > l( *( IMPL::rmtx ) );

                        functions = this->impl->_functions;
                    }

                    for( const auto & f: functions )
                    {
                        f();
                    }

                    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
                }
            }
        )
        .detach();
    }

    UpdateQueue::IMPL::IMPL()
    {}

    UpdateQueue::IMPL::~IMPL()
    {}

    std::recursive_mutex * UpdateQueue::IMPL::rmtx  = nullptr;
    UpdateQueue          * UpdateQueue::IMPL::queue = nullptr;
}
