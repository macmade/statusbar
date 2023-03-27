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

#ifndef SB_MEMORY_INFO_HPP
#define SB_MEMORY_INFO_HPP

#include <memory>
#include <algorithm>
#include <vector>
#include <cstdint>

namespace SB
{
    class MemoryInfo
    {
        public:

            static void       startObserving();
            static MemoryInfo current();

            MemoryInfo( const MemoryInfo & o );
            MemoryInfo( MemoryInfo && o ) noexcept;
            ~MemoryInfo();

            MemoryInfo & operator =( MemoryInfo o );

            uint64_t total()       const;
            uint64_t wired()       const;
            uint64_t active()      const;
            uint64_t inactive()    const;
            uint64_t free()        const;
            uint64_t purgeable()   const;
            uint64_t speculative() const;
            uint64_t used()        const;
            double   percentUsed() const;

            friend void swap( MemoryInfo & o1, MemoryInfo & o2 );

        private:

            MemoryInfo( uint64_t total, uint64_t wired, uint64_t active, uint64_t inactive, uint64_t free, uint64_t purgeable, uint64_t speculative );

            class IMPL;
            std::unique_ptr< IMPL > impl;
    };
}

#endif /* SB_MEMORY_INFO_HPP */
