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

#ifndef SB_BATTERY_INFO_HPP
#define SB_BATTERY_INFO_HPP

#include <memory>
#include <algorithm>
#include <vector>
#include <cstdint>

namespace SB
{
    class BatteryInfo
    {
        public:

            static void        startObserving();
            static BatteryInfo current();

            BatteryInfo( const BatteryInfo & o );
            BatteryInfo( BatteryInfo && o ) noexcept;
            ~BatteryInfo();

            BatteryInfo & operator =( BatteryInfo o );

            int64_t capacity()    const;
            bool    isCharging()  const;
            bool    isAvailable() const;

            friend void swap( BatteryInfo & o1, BatteryInfo & o2 );

        private:

            BatteryInfo( int64_t capacity, bool isCharging, bool isAvailable );

            class IMPL;
            std::unique_ptr< IMPL > impl;
    };
}

#endif /* SB_BATTERY_INFO_HPP */
