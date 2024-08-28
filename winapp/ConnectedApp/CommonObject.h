#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <d3d11.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <string_view>

// Struct for storing Flight data
struct FlightData {
    std::string icao24;
    std::string flight_id;
    double latitude;
    double longitude;
    double altitude;
    double velocity;
    double heading;
    std::string origin_country;
};

// Struct for storing Airport data
struct Airport {
    std::string icao;
    std::string iata;
    std::string name;
    std::string city;
    std::string state;
    std::string country;
    int elevation = 0;
    double lat = 0.0;
    double lon = 0.0;
    std::string tz;
};

struct CommonObjects {
    std::atomic_bool exit_flag = false; // Flag to indicate if the application should exit
    std::atomic_bool start_download = false; // Flag to indicate if download should start
    std::atomic_bool data_ready = false; // Flag to indicate if data is ready for processing
    std::string url; // URL for downloading data
    std::unordered_map<std::string, FlightData> flights; // Map to store flight data
    std::unordered_map<std::string, Airport> Airports; // Map to store airport data
    std::mutex mutex; // Mutex to protect shared data access
    std::set<std::string> favorite_flights; // Set to store favorite flights
};
