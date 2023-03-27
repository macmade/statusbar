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

#include "SB/Instruments/Internal/IOHID.hpp"

namespace SB
{
    namespace IOHID
    {
        std::map< std::string, double > read( IOHIDEventSystemClientRef client, int64_t page, int64_t usage, int64_t eventType, int64_t eventField )
        {
            if( client == nullptr )
            {
                return {};
            }

            CFDictionaryRef filter = nullptr;

            {
                CFNumberRef cfPage  = CFNumberCreate( nullptr, kCFNumberSInt64Type, &page );
                CFNumberRef cfUsage = CFNumberCreate( nullptr, kCFNumberSInt64Type, &usage );

                CFTypeRef keys[]   = { CFSTR( "PrimaryUsagePage" ), CFSTR( "PrimaryUsage" ) };
                CFTypeRef values[] = { cfPage, cfUsage };

                filter = CFDictionaryCreate( nullptr, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

                CFRelease( cfPage );
                CFRelease( cfUsage );
            }

            IOHIDEventSystemClientSetMatching( client, filter );
            CFRelease( filter );

            CFArrayRef services = IOHIDEventSystemClientCopyServices( client );

            if( services == nullptr )
            {
                return {};
            }

            std::map< std::string, double > values;

            for( CFIndex i = 0; i < CFArrayGetCount( services ); i++ )
            {
                IOHIDServiceClientRef service = reinterpret_cast< IOHIDServiceClientRef >( const_cast< void * >( CFArrayGetValueAtIndex( services, i ) ) );
                CFStringRef           name    = reinterpret_cast< CFStringRef >( IOHIDServiceClientCopyProperty( service, CFSTR( "Product" ) ) );
                CFTypeRef             event   = IOHIDServiceClientCopyEvent( service, eventType, 0, 0 );

                if( name != nullptr && event != nullptr )
                {
                    double       value = IOHIDEventGetFloatValue( event, eventField );
                    const char * cName = CFStringGetCStringPtr( name, kCFStringEncodingUTF8 );

                    if( cName != nullptr )
                    {
                        values[ cName ] = value;
                    }
                }

                if( name  != nullptr ) { CFRelease( name ); }
                if( event != nullptr ) { CFRelease( event ); }
            }

            CFRelease( services );

            return values;
        }
    }
}
