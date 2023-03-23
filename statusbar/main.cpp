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

#include "SB/Screen.hpp"
#include "SB/CPULoad.hpp"
#include "SB/BatteryInfo.hpp"
#include "SB/String.hpp"
#include "SB/Window.hpp"
#include <locale.h>

void displayCPU(     SB::Window & window, const SB::CPULoad & cpu );
void displayBattery( SB::Window & window, const SB::BatteryInfo & battery );
void displayDate(    SB::Window & window, time_t time );
void displayHour(    SB::Window & window, time_t time );

int main( int argc, const char * argv[] )
{
    ( void )argc;
    ( void )argv;

    setlocale( LC_ALL, "" );

    {
        SB::Screen & screen = SB::Screen::shared();

        screen.onKeyPress
        (
            [ & ]( int key )
            {
                if( key == 'q' )
                {
                    screen.stop();
                }
            }
        );

        screen.onUpdate
        (
            [ & ]
            {
                SB::CPULoad cpu         = SB::CPULoad::current();
                SB::BatteryInfo battery = SB::BatteryInfo::current();

                SB::Window left( 0, 0, screen.width() - 30, screen.height() );
                SB::Window right( screen.width() - 30, 0, 30, screen.height() );

                displayCPU( left, SB::CPULoad::current() );

                if( battery.isAvailable() )
                {
                    left.print( "    " );
                    displayBattery( left, battery );
                }

                displayDate( right, time( nullptr ) );
                right.print( " " );
                displayHour( right, time( nullptr ) );

                left.refresh();
                right.refresh();
            }
        );

        SB::CPULoad::startObserving();
        SB::BatteryInfo::startObserving();
        screen.start();
    }

    return 0;
}

void displayCPU( SB::Window & window, const SB::CPULoad & cpu )
{
    window.print( SB::Color::red(), SB::Color::clear(), "\uf2db CPU: %.0f%% ", cpu.total() );

    for( int i = 0; i < 10; i++ )
    {
        if( cpu.total() / 10 < i )
        {
            window.print( SB::Color::red(), SB::Color::clear(), "\uf096 " );
        }
        else
        {
            window.print( SB::Color::red(), SB::Color::clear(), "\uf0c8 " );
        }
    }

    window.print( SB::Color::red(), SB::Color::clear(), " \uf007 %.0f%% \uf013 %.0f%%", cpu.user(), cpu.system() );
}

void displayBattery( SB::Window & window, const SB::BatteryInfo & battery )
{
    window.print( SB::Color::yellow(), SB::Color::clear(), "\uf241  Battery: %lli%%", battery.capacity() );

    if( battery.isCharging() )
    {
        window.print( SB::Color::yellow(), SB::Color::clear(), " \uf0e7 " );
    }
}

void displayDate( SB::Window & window, time_t time )
{
    struct tm * local = localtime( &time );

    char buf[ 1024 ];

    memset( buf, 0, sizeof( buf ) );
    strftime( buf, sizeof( buf ), "%d %B %Y", local );

    window.print( SB::Color::cyan(), SB::Color::clear(), "\uf073  %s ", buf );
}

void displayHour( SB::Window & window, time_t time )
{
    struct tm * local = localtime( &time );

    char buf[ 1024 ];

    memset( buf, 0, sizeof( buf ) );
    strftime( buf, sizeof( buf ), "%H:%M:%S", local );

    window.print( SB::Color::magenta(), SB::Color::clear(), "\uf017  %s ", buf );
}

