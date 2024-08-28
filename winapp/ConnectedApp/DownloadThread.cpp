#include "DownloadThread.h"
#include "CommonObject.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

// Define serialization/deserialization for FlightData and Airport using nlohmann::json
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FlightData, icao24, flight_id, latitude, longitude, altitude, velocity, heading, origin_country)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Airport, icao, iata, name, city, state, country, elevation, lat, lon, tz)

// Function to load airport data from a JSON file
void DownloadThread::LoadAirportsFromJson(CommonObjects& common_objects, const std::string& airports_file) {
    // Open the JSON file
    std::ifstream file(airports_file);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << airports_file << std::endl;
        return;
    }

    // Parse JSON data
    nlohmann::json jsonData;
    file >> jsonData;
    file.close();

    // Lock the mutex to ensure thread safety while accessing shared data
    std::lock_guard<std::mutex> lock(common_objects.mutex);

    // Iterate over each airport entry in the JSON data
    for (const auto& [key, value] : jsonData.items()) {
        Airport airport;

        // Extract relevant fields from the JSON data, checking for null and correct types
        if (!value["icao"].is_null() && value["icao"].is_string()) airport.icao = value["icao"].get<std::string>();
        if (!value["iata"].is_null() && value["iata"].is_string()) airport.iata = value["iata"].get<std::string>();
        if (!value["name"].is_null() && value["name"].is_string()) airport.name = value["name"].get<std::string>();
        if (!value["city"].is_null() && value["city"].is_string()) airport.city = value["city"].get<std::string>();
        if (!value["state"].is_null() && value["state"].is_string()) airport.state = value["state"].get<std::string>();
        if (!value["country"].is_null() && value["country"].is_string()) airport.country = value["country"].get<std::string>();
        if (!value["tz"].is_null() && value["tz"].is_string()) airport.tz = value["tz"].get<std::string>();

        // Use value() with default values for elevation, lat, and lon
        airport.elevation = value.value("elevation", 0);
        airport.lat = value.value("lat", 0.0);
        airport.lon = value.value("lon", 0.0);

        // Add the airport to the shared map if it has a valid ICAO code
        if (!airport.icao.empty()) {
            common_objects.Airports[airport.icao] = airport;
        }
    }
    // Mark data as ready after loading all airports
    common_objects.data_ready = true;
    std::cout << "Loaded " << common_objects.Airports.size() << " airports." << std::endl;
}

// Main function for the DownloadThread
void DownloadThread::operator()(CommonObjects& common) {
    try {
        // Load airport data
        LoadAirportsFromJson(common, airports_file);

        std::cout << "Airports loaded, proceeding to fetch flight data..." << std::endl;

        // Set up HTTP client for OpenSky Network API
        std::string url = _download_url.empty() ? "http://opensky-network.org" : _download_url;
        httplib::Client cli(url);

        // Set authentication for OpenSky Network API
        cli.set_basic_auth(opensky_username.c_str(), opensky_password.c_str());

        // Main loop for fetching flight data
        while (!common.exit_flag) {
            // Make API request to get all states (flights)
            auto res = cli.Get("/api/states/all");
            if (res && res->status == 200) {
                nlohmann::json json_result = nlohmann::json::parse(res->body);

                std::unordered_map<std::string, FlightData> flights;

                // Process each flight state
                for (const auto& state : json_result["states"]) {
                    if (state.size() >= 8) {
                        FlightData flight;
                        // Extract flight data from API response
                        flight.icao24 = state[0].get<std::string>();
                        flight.flight_id = state[1].is_null() ? "" : state[1].get<std::string>();
                        flight.latitude = state[6].is_null() ? 0.0 : state[6].get<double>();
                        flight.longitude = state[5].is_null() ? 0.0 : state[5].get<double>();
                        flight.altitude = state[7].is_null() ? 0.0 : state[7].get<double>();
                        flight.velocity = state[9].is_null() ? 0.0 : state[9].get<double>();
                        flight.heading = state[10].is_null() ? 0.0 : state[10].get<double>();
                        flight.origin_country = state[2].get<std::string>();

                        // Add flight to the map if it has a valid flight ID
                        if (!flight.flight_id.empty()) {
                            flights[flight.flight_id] = flight;
                        }
                    }
                }
                
                // Update shared flight data
                {
                    std::unique_lock<std::mutex> lock(common.mutex);
                    common.flights = std::move(flights);
                    common.data_ready = true;
                }
                std::cout << "Updated flight data. Total flights: " << common.flights.size() << std::endl;
            } else {
                std::cerr << "HTTP request failed with status: " << (res ? res->status : 0) << std::endl;
            }
            // Wait for 60 seconds before next update
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in DownloadThread: " << e.what() << std::endl;
    }
}

// Function to set a new URL for the OpenSky Network API
void DownloadThread::SetUrl(std::string_view new_url) {
    _download_url = new_url;
}