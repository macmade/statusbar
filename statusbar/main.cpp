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
#include "SB/NetworkInfo.hpp"
#include "SB/Options.hpp"
#include "SB/MemoryInfo.hpp"
#include "SB/TemperatureInfo.hpp"
#include "SB/String.hpp"
#include "SB/Window.hpp"
#include <locale.h>
#include <thread>
#include <iostream>

void displayCPU(         const SB::Color & color, SB::Window & window, const SB::CPULoad & cpu );
void displayMemory(      const SB::Color & color, SB::Window & window, const SB::MemoryInfo & memory );
void displayBattery(     const SB::Color & color, SB::Window & window, const SB::BatteryInfo & battery );
void displayTemperature( const SB::Color & color, SB::Window & window, const SB::TemperatureInfo & temperature );
void displayNetwork(     const SB::Color & color, SB::Window & window, const SB::NetworkInfo & network );
void displayDate(        const SB::Color & color, SB::Window & window, time_t time );
void displayTime(        const SB::Color & color, SB::Window & window, time_t time );

void showHelp();

int main( int argc, const char * argv[] )
{
    SB::Options options = { argc, argv };

    if( options.help() )
    {
        showHelp();

        return 0;
    }

    if( options.cpu()         ) { SB::CPULoad::startObserving(); }
    if( options.memory()      ) { SB::MemoryInfo::startObserving(); }
    if( options.battery()     ) { SB::BatteryInfo::startObserving(); }
    if( options.network()     ) { SB::NetworkInfo::startObserving(); }
    if( options.temperature() ) { SB::TemperatureInfo::startObserving(); }

    setlocale( LC_ALL, "" );

    if( options.debug() )
    {
        while( true )
        {
            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        }
    }
    else
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
                SB::CPULoad         cpu         = SB::CPULoad::current();
                SB::MemoryInfo      memory      = SB::MemoryInfo::current();
                SB::BatteryInfo     battery     = SB::BatteryInfo::current();
                SB::NetworkInfo     network     = SB::NetworkInfo::current();
                SB::TemperatureInfo temperature = SB::TemperatureInfo::current();

                SB::Window left( 0, 0, screen.width() - 32, screen.height() );
                SB::Window right( screen.width() - 32, 0, 32, screen.height() );

                bool hasLeftData  = false;
                bool hasRightData = false;

                if( options.cpu() )
                {
                    if( hasLeftData )
                    {
                        left.print( "    " );
                    }

                    displayCPU( options.cpuColor(), left, cpu );

                    hasLeftData = true;
                }

                if( options.memory() && memory.total() > 0 )
                {
                    if( hasLeftData )
                    {
                        left.print( "    " );
                    }

                    displayMemory( options.memoryColor(),left, memory );

                    hasLeftData = true;
                }

                if( options.temperature() && temperature.temperature() > 0 )
                {
                    if( hasLeftData )
                    {
                        left.print( "    " );
                    }

                    displayTemperature( options.temperatureColor(), left, temperature );

                    hasLeftData = true;
                }

                if( options.battery() && battery.isAvailable() )
                {
                    if( hasLeftData )
                    {
                        left.print( "    " );
                    }

                    displayBattery( options.batteryColor(), left, battery );

                    hasLeftData = true;
                }

                if( options.network() && network.name().empty() == false )
                {
                    if( hasLeftData )
                    {
                        left.print( "    " );
                    }

                    displayNetwork( options.networkColor(), left, network );

                    hasLeftData = true;
                }

                if( options.date() )
                {
                    if( hasRightData )
                    {
                        right.print( "    " );
                    }

                    displayDate( options.dateColor(), right, time( nullptr ) );

                    hasRightData = true;
                }

                if( options.time() )
                {
                    if( hasRightData )
                    {
                        right.print( "    " );
                    }

                    displayTime( options.timeColor(), right, time( nullptr ) );

                    hasRightData = true;
                }

                left.refresh();
                right.refresh();
            }
        );

        screen.start( 500 );
    }

    return 0;
}

void displayCPU( const SB::Color & color, SB::Window & window, const SB::CPULoad & cpu )
{
    window.print( color, SB::Color::clear(), "\uf2db CPU: %.0f%% ", cpu.total() );

    for( int i = 0; i < 10; i++ )
    {
        if( cpu.total() / 10 < i )
        {
            window.print( color, SB::Color::clear(), "\uf096 " );
        }
        else
        {
            window.print( color, SB::Color::clear(), "\uf0c8 " );
        }
    }

    window.print( color, SB::Color::clear(), " \uf007 %.0f%% \uf013 %.0f%%", cpu.user(), cpu.system() );
}

void displayMemory( const SB::Color & color, SB::Window & window, const SB::MemoryInfo & memory )
{
    window.print( color, SB::Color::clear(), "\uf1c0 Memory %.0f%% : ", memory.percentUsed() );

    for( int i = 0; i < 10; i++ )
    {
        if( memory.percentUsed() / 10 < i )
        {
            window.print( color, SB::Color::clear(), "\uf096 " );
        }
        else
        {
            window.print( color, SB::Color::clear(), "\uf0c8 " );
        }
    }

    window.print
    (
        color,
        SB::Color::clear(),
        " %s / %s",
        SB::String::bytesToHumanReadable( memory.used() ).c_str(),
        SB::String::bytesToHumanReadable( memory.total() ).c_str()
    );
}

void displayBattery( const SB::Color & color, SB::Window & window, const SB::BatteryInfo & battery )
{
    int64_t      capacity = battery.capacity();
    const char * icon;

         if( capacity >= 80 ) { icon = "\uf240"; }
    else if( capacity >= 60 ) { icon = "\uf241"; }
    else if( capacity >= 40 ) { icon = "\uf242"; }
    else if( capacity >= 20 ) { icon = "\uf243"; }
    else                      { icon = "\uf244"; }

    window.print( color, SB::Color::clear(), "%s  Battery: %lli%%", icon, capacity );

    if( battery.isCharging() )
    {
        window.print( color, SB::Color::clear(), " \uf0e7" );
    }
}

void displayTemperature( const SB::Color & color, SB::Window & window, const SB::TemperatureInfo & temperature )
{
    window.print( color, SB::Color::clear(), " \uf2c7 Temperature: %.0f°", temperature.temperature() );
}

void displayNetwork( const SB::Color & color, SB::Window & window, const SB::NetworkInfo & network )
{
    window.print( color, SB::Color::clear(), " \uead0 Network: %s (%s)", network.address().c_str(), network.name().c_str() );
}

void displayDate( const SB::Color & color, SB::Window & window, time_t time )
{
    struct tm * local = localtime( &time );

    char buf[ 1024 ];

    memset( buf, 0, sizeof( buf ) );
    strftime( buf, sizeof( buf ), "%d %B %Y", local );

    window.print( color, SB::Color::clear(), "\uf073  %s ", buf );
}

void displayTime( const SB::Color & color, SB::Window & window, time_t time )
{
    struct tm * local = localtime( &time );

    char buf[ 1024 ];

    memset( buf, 0, sizeof( buf ) );
    strftime( buf, sizeof( buf ), "%H:%M:%S", local );

    window.print( color, SB::Color::clear(), "\uf017  %s ", buf );
}

void showHelp()
{
    std::cout << "Usage: statusbar [OPTIONS]"
              << std::endl
              << std::endl
              << "Options:"
              << std::endl
              << std::endl
              << "    --help                 Shows this help dialog"   << std::endl
              << "    --cpu                  Displays CPU load"        << std::endl
              << "    --memory               Displays memory usage"     << std::endl
              << "    --temperature          Displays temperature"      << std::endl
              << "    --battery              Displays battery charge"   << std::endl
              << "    --network              Displays network address"  << std::endl
              << "    --date                 Displays current date"     << std::endl
              << "    --time                 Displays current time"     << std::endl
              << "    --cpu-color            Color for CPU load"        << std::endl
              << "    --memory-color         Color for memory usage"    << std::endl
              << "    --temperature-color    Color for temperature"     << std::endl
              << "    --battery-color        Color for battery charge"  << std::endl
              << "    --network-color        Color for network address" << std::endl
              << "    --date-color           Color for current date"    << std::endl
              << "    --time-color           Color for current time"    << std::endl
              << std::endl
              << "Available Colors: red, yellow, green, cyan, blue, magenta, black, white, clear"
              << std::endl;
}
