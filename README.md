statusbar
=========

[![Build Status](https://img.shields.io/github/actions/workflow/status/macmade/statusbar/ci-mac.yaml?label=macOS&logo=apple)](https://github.com/macmade/statusbar/actions/workflows/ci-mac.yaml)
[![Issues](http://img.shields.io/github/issues/macmade/statusbar.svg?logo=github)](https://github.com/macmade/statusbar/issues)
![Status](https://img.shields.io/badge/status-active-brightgreen.svg?logo=git)
![License](https://img.shields.io/badge/license-mit-brightgreen.svg?logo=open-source-initiative)  
[![Contact](https://img.shields.io/badge/follow-@macmade-blue.svg?logo=twitter&style=social)](https://twitter.com/macmade)
[![Sponsor](https://img.shields.io/badge/sponsor-macmade-pink.svg?logo=github-sponsors&style=social)](https://github.com/sponsors/macmade)

### About

Statusbar for your terminal:

![Screenshot](Assets/statusbar.png "Screenshot")
![CPU](Assets/cpu.png "CPU")
![GPU](Assets/gpu.png "GPU")
![Memory](Assets/memory.png "Memory")
![Battery](Assets/battery.png "Battery")
![Temperature](Assets/temperature.png "Temperature")
![Network](Assets/network.png "Network")
![DateTime](Assets/datetime.png "DateTime")

### Installation

```
brew install --HEAD macmade/tap/statusbar 
```

For updates, run:

```
brew reinstall macmade/tap/statusbar 
```

### Usage

```
Usage: statusbar [OPTIONS]

Options:

    --help                 Show the help dialog
    --cpu                  Display CPU load
    --gpu                  Display GPU load
    --memory               Display memory usage
    --temperature          Display temperature
    --battery              Display battery charge
    --network              Display network address
    --date                 Display current date
    --time                 Display current time
    --no-cpu               Don't display CPU load
    --no-gpu               Don't display GPU load
    --no-memory            Don't display memory usage
    --no-temperature       Don't display temperature
    --no-battery           Don't display battery charge
    --no-network           Don't display network address
    --no-date              Don't display current date
    --no-time              Don't display current time
    --cpu-color            Color for CPU load
    --gpu-color            Color for GPU load
    --memory-color         Color for memory usage
    --temperature-color    Color for temperature
    --battery-color        Color for battery charge
    --network-color        Color for network address
    --date-color           Color for current date
    --time-color           Color for current time

Available Colors: red, yellow, green, cyan, blue, magenta, black, white, clear
```

License
-------

Project is released under the terms of the MIT License.

Repository Infos
----------------

    Owner:          Jean-David Gadina - XS-Labs
    Web:            www.xs-labs.com
    Blog:           www.noxeos.com
    Twitter:        @macmade
    GitHub:         github.com/macmade
    LinkedIn:       ch.linkedin.com/in/macmade/
    StackOverflow:  stackoverflow.com/users/182676/macmade
