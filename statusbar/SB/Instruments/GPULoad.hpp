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

#ifndef SB_GPU_LOAD_HPP
#define SB_GPU_LOAD_HPP

#include <memory>
#include <algorithm>
#include <string>

namespace SB
{
    class GPULoad
    {
        public:

            static void    startObserving();
            static GPULoad current();

            GPULoad( const GPULoad & o );
            GPULoad( GPULoad && o ) noexcept;
            ~GPULoad();

            GPULoad & operator =( GPULoad o );

            double percent() const;

            friend void swap( GPULoad & o1, GPULoad & o2 );

        private:

            GPULoad( double percent );

            class IMPL;
            std::unique_ptr< IMPL > impl;
    };
}

#endif /* SB_GPU_LOAD_HPP */
