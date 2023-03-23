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

#ifndef SB_CPU_LOAD_HPP
#define SB_CPU_LOAD_HPP

#include <memory>
#include <algorithm>
#include <vector>
#include <cstdint>

namespace SB
{
    class CPULoad
    {
        public:

            static void    startObserving();
            static CPULoad current();
            
            CPULoad( const CPULoad & o );
            CPULoad( CPULoad && o ) noexcept;
            ~CPULoad();

            CPULoad & operator =( CPULoad o );

            double user()   const;
            double system() const;
            double idle()   const;
            double total()  const;

            friend void swap( CPULoad & o1, CPULoad & o2 );

        private:

            CPULoad( double user, double system, double idle, double total );

            class IMPL;
            std::unique_ptr< IMPL > impl;
    };
}

#endif /* SB_CPU_LOAD_HPP */
