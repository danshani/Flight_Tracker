# Real-Time Flight Tracking Application

## Overview
This project is a real-time flight tracking application.
that uses the OpenSky Network API to fetch and display data about active flights.
Users can search for specific flights, view detailed information, and manage their favorite flights.
Additionally, users can search for airports and view arrival and departure information for those airports.
The application features a user-friendly graphical interface using the ImGui library.

## Features
Real-time flight tracking
Search functionality for flights and airports
View detailed flight information
Manage favorite flights
Display arrivals and departures for selected airports
Graphical User Interface (GUI) rendered with ImGui

## Prerequisites
C++ compiler
CMake
ImGui library
OpenSky Network API credentials

## Installation
```
git clone https://github.com/danshani/CppApp.git
cd CppApp

mkdir build
cd build

cmake ..

cmake --build .

./flight_tracking_app
```

## Contributing
If you want to contribute to this project, please submit a pull request or open an issue to discuss potential changes or improvements.

## Acknowledgements
ImGui library for GUI rendering
OpenSky Network API for flight data
For more details, visit the repository.

## License
MIT License
Copyright (c) [2024] Dan Shani,Eyal Zamero
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
