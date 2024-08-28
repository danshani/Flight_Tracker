#include "DrawThread.h"

// Function to search for airports based on a user input airport search term IATA
void SearchAirports(const std::string& search_term, const std::unordered_map<std::string, Airport>& airports, std::vector<Airport>& results) {
    results.clear(); // Clear any previous search results

    // Convert the search term to uppercase for case-insensitive comparison
    std::string search_upper = search_term;
    std::transform(search_upper.begin(), search_upper.end(), search_upper.begin(), ::toupper);

    // Iterate through the airports map
    for (const auto& [_, airport] : airports) {
        // Check if the airport's IATA code matches the search term exactly
        if (airport.iata == search_upper) {
            // Insert the airport at the beginning of the results vector
            results.insert(results.begin(), airport);
        }
        // Check if the search term is a substring of the airport's IATA code, name, or city
        else 
            if (airport.iata.find(search_upper) != std::string::npos ||
            airport.name.find(search_upper) != std::string::npos ||
            airport.city.find(search_upper) != std::string::npos) {
            // Add the airport to the end of the results vector
            results.push_back(airport);
        }
    }
}

// Function to fetch flight data from the OpenSky Network API
nlohmann::json FetchFlightData(const std::string& endpoint, const std::string& icao_code) {
    // Create an HTTP client for the OpenSky Network API
    httplib::Client cli("https://opensky-network.org");

    // Set basic authentication credentials
    cli.set_basic_auth("danshani11", "6LS6eecDG@BDx2y");

    // Get the current time and calculate the start and end times for the last 24 hours
    auto now = std::chrono::system_clock::now();
    auto end_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    auto start_time = end_time - (24 * 60 * 60);

    // Construct the URL for the API request
    std::string url = "/api/flights/" + endpoint + "?airport=" + icao_code + "&begin=" + std::to_string(start_time) + "&end=" + std::to_string(end_time);

    std::cout << "Fetching data from URL: " << url << std::endl;

    // Send the GET request to the API
    auto res = cli.Get(url.c_str());
    if (res && res->status == 200) {
        std::cout << "Data fetched successfully" << std::endl;
        // Parse and return the JSON response
        return nlohmann::json::parse(res->body);
    } else {
        std::cerr << "Failed to fetch data. Status: " << (res ? res->status : 0) << std::endl;
    }
    // Return an empty JSON array if the request fails
    return nlohmann::json::array();
}

// helper for displaying Airport Arrivals and Depratures flights tables in the app
void AirportFlightTable(const char* title, const nlohmann::json& flights, bool is_arrival) {
    // Display the table title
    ImGui::Text("%s", title);

    // Begin the table with 5 columns and specific flags
    if (ImGui::BeginTable("Flight Table", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Setup the table columns
        ImGui::TableSetupColumn("Flight");
        ImGui::TableSetupColumn(is_arrival ? "From" : "To");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Aircraft");
        ImGui::TableSetupColumn("Callsign");
        ImGui::TableHeadersRow();

        // Iterate through the flights and populate the table rows
        for (const auto& flight : flights) {
            ImGui::TableNextRow();

            // Column 0: Flight callsign
            ImGui::TableSetColumnIndex(0);
            std::string callsign = flight.contains("callsign") && !flight["callsign"].is_null()
                ? flight["callsign"].get<std::string>()
                : "Unknown";
            ImGui::Text("%s", callsign.c_str());

            // Column 1: Departure or arrival airport
            ImGui::TableSetColumnIndex(1);
            std::string airport = flight.contains(is_arrival ? "estDepartureAirport" : "estArrivalAirport") && !flight[is_arrival ? "estDepartureAirport" : "estArrivalAirport"].is_null()
                ? flight[is_arrival ? "estDepartureAirport" : "estArrivalAirport"].get<std::string>()
                : "Unknown";

            // Display city if available, otherwise display airport code
            std::string city = flight.contains("city") && !flight["city"].is_null() && !flight["city"].get<std::string>().empty()
                ? flight["city"].get<std::string>()
                : airport;
            ImGui::Text("%s", city.c_str());

            // Column 2: Last seen time
            ImGui::TableSetColumnIndex(2);
            long long lastSeen = flight.contains("lastSeen") && !flight["lastSeen"].is_null()
                ? flight["lastSeen"].get<long long>()
                : 0;

            std::time_t t = static_cast<std::time_t>(lastSeen);
            std::tm* tm = std::gmtime(&t);
            std::stringstream ss;
            ss << std::put_time(tm, "%H:%M:%S");
            ImGui::Text("%s", ss.str().c_str());

            // Column 3: Aircraft ICAO24 code
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", flight["icao24"].get<std::string>().c_str());

            // Column 4: Callsign (repeated)
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", callsign.c_str());
        }
        // End the table
        ImGui::EndTable();
    }
}

// helper for displaying all flights table in the app
void ShowAllActiveFlights(CommonObjects* common) {
    // Begin the table with 8 columns
    if (ImGui::BeginTable("Flights", 8)) {
        // Setup the table columns
        ImGui::TableSetupColumn("Flight ID");
        ImGui::TableSetupColumn("ICAO24");
        ImGui::TableSetupColumn("Latitude");
        ImGui::TableSetupColumn("Longitude");
        ImGui::TableSetupColumn("Altitude");
        ImGui::TableSetupColumn("Velocity");
        ImGui::TableSetupColumn("Heading");
        ImGui::TableSetupColumn("Origin Country");
        ImGui::TableHeadersRow();

        // Iterate through the flights and populate the table rows
        for (const auto& [flight_id, flight] : common->flights) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(flight.flight_id.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(flight.icao24.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.6f", flight.latitude);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.6f", flight.longitude);
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%.2f", flight.altitude);
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%.2f", flight.velocity);
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%.2f", flight.heading);
            ImGui::TableSetColumnIndex(7);
            ImGui::Text(flight.origin_country.c_str());
        }
        // End the table
        ImGui::EndTable();
    }
}

// Helper for searching and displaying flights in GUI
void SearchAndDisplayFlights(CommonObjects* common, const char* search_buf) {
    // Convert the search buffer to uppercase for case-insensitive comparison
    std::string search_upper = search_buf;
    std::transform(search_upper.begin(), search_upper.end(), search_upper.begin(), ::toupper);

    bool found_any = false;

    // Iterate through the flights to find matches
    for (const auto& pair : common->flights) {
        const auto& flight = pair.second;
        std::string flight_id_upper = flight.flight_id;
        std::transform(flight_id_upper.begin(), flight_id_upper.end(), flight_id_upper.begin(), ::toupper);

        // Check if the flight ID contains the search term
        if (flight_id_upper.find(search_upper) != std::string::npos) {
            ImGui::PushID(flight.flight_id.c_str());

            // Display flight details
            ImGui::Text("Flight ID: %s", flight.flight_id.c_str());
            ImGui::SameLine();

            // Check if the flight is a favorite and display the appropriate button
            bool is_favorite = common->favorite_flights.find(flight.flight_id) != common->favorite_flights.end();
            if (is_favorite) {
                if (ImGui::Button("Remove from Favorites")) {
                    common->favorite_flights.erase(flight.flight_id);
                }
            }
            else {
                if (ImGui::Button("Add to Favorites")) {
                    common->favorite_flights.insert(flight.flight_id);
                }
            }

            // Display additional flight information
            ImGui::Text("ICAO24: %s", flight.icao24.c_str());
            ImGui::Text("Latitude: %.6f", flight.latitude);
            ImGui::Text("Longitude: %.6f", flight.longitude);
            ImGui::Text("Altitude: %.2f", flight.altitude);
            ImGui::Text("Velocity: %.2f", flight.velocity);
            ImGui::Text("Heading: %.2f", flight.heading);
            ImGui::Text("Origin Country: %s", flight.origin_country.c_str());
            ImGui::Separator();

            ImGui::PopID();
            found_any = true;
        }
    }

    // Display a message if no matching flight is found
    if (!found_any) {
        ImGui::Text("No matching flight found.");
    }
}

// Helper for displaying favorite flights in GUI
void DisplayFavoriteFlights(CommonObjects* common) {
    // Display the title for the favorite flights section
    ImGui::Text("Favorite Flights");
    ImGui::Separator();

    // Check if there are any favorite flights
    if (common->favorite_flights.empty()) {
        ImGui::Text("No favorite flights added yet.");
        return;
    }

    // Iterate through the favorite flights
    for (const auto& flight_id : common->favorite_flights) {
        auto it = common->flights.find(flight_id);
        if (it != common->flights.end()) {
            const auto& flight = it->second;
            ImGui::PushID(flight.flight_id.c_str());

            // Display flight details
            ImGui::Text("Flight ID: %s", flight.flight_id.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Remove from Favorites")) {
                // Remove the flight from favorites and break to avoid iterator invalidation
                common->favorite_flights.erase(flight.flight_id);
                ImGui::PopID();
                break;
            }

            // Display additional flight information
            ImGui::Text("ICAO24: %s", flight.icao24.c_str());
            ImGui::Text("Origin: %s", flight.origin_country.c_str());
            ImGui::Text("Latitude: %.6f, Longitude: %.6f", flight.latitude, flight.longitude);
            ImGui::Text("Altitude: %.2f, Velocity: %.2f", flight.altitude, flight.velocity);
            ImGui::Separator();

            ImGui::PopID();
        }
    }
}

// Draw Functions for GUI
void DrawSearchBar(char* search_buf, size_t buf_size, bool& showAllFlightsInBackground, bool& show_airport_search, bool& show_favorites, Airport& selected_airport, nlohmann::json& arrivals_data, nlohmann::json& departures_data, char* airport_search_buf, size_t airport_buf_size) {
    float margin = 10.0f;
    float buttonWidth = 100.0f;
    float buttonHeight = 20.0f;

    ImGui::SetCursorPos(ImVec2(margin, margin));
    ImGui::Text("Search:");
    ImGui::SameLine();

    bool main_search_active = !showAllFlightsInBackground && !show_airport_search && !show_favorites;

    // Main search bar
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, main_search_active ? 1.0f : 0.5f);
    if (ImGui::InputText("##search", search_buf, buf_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
        // When Enter is pressed, perform the search
        showAllFlightsInBackground = false;
        show_airport_search = false;
        show_favorites = false;
        selected_airport = Airport();
        arrivals_data.clear();
        departures_data.clear();
        memset(airport_search_buf, 0, airport_buf_size);  // Clear airport search
    }
    if (ImGui::IsItemClicked() && !main_search_active) {
        // When clicked and not already active, reset to main search
        showAllFlightsInBackground = false;
        show_airport_search = false;
        show_favorites = false;
        memset(search_buf, 0, buf_size);
        selected_airport = Airport();
        arrivals_data.clear();
        departures_data.clear();
        memset(airport_search_buf, 0, airport_buf_size);  // Clear airport search
    }
    ImGui::PopStyleVar();

    ImGui::SameLine();

    // All Flights button
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, showAllFlightsInBackground ? 1.0f : 0.5f);
    if (ImGui::Button("All Flights", ImVec2(buttonWidth, buttonHeight))) {
        showAllFlightsInBackground = !showAllFlightsInBackground;
        show_airport_search = false;
        show_favorites = false;
        memset(search_buf, 0, buf_size);
        selected_airport = Airport();
        arrivals_data.clear();
        departures_data.clear();
        memset(airport_search_buf, 0, airport_buf_size);  // Clear airport search
    }
    ImGui::PopStyleVar();

    ImGui::SameLine();

    // Airport button
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, show_airport_search ? 1.0f : 0.5f);
    if (ImGui::Button("Airport", ImVec2(buttonWidth, buttonHeight))) {
        show_airport_search = !show_airport_search;
        showAllFlightsInBackground = false;
        show_favorites = false;
        memset(search_buf, 0, buf_size);
        selected_airport = Airport();
        arrivals_data.clear();
        departures_data.clear();
        if (!show_airport_search) {
            memset(airport_search_buf, 0, airport_buf_size);  // Clear airport search when leaving airport section
        }
    }
    ImGui::PopStyleVar();

    ImGui::SameLine();

    // Favorites button
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, show_favorites ? 1.0f : 0.5f);
    if (ImGui::Button("Favorites", ImVec2(buttonWidth, buttonHeight))) {
        show_favorites = !show_favorites;
        showAllFlightsInBackground = false;
        show_airport_search = false;
        memset(search_buf, 0, buf_size);
        selected_airport = Airport();
        arrivals_data.clear();
        departures_data.clear();
        memset(airport_search_buf, 0, airport_buf_size);  // Clear airport search
    }
    ImGui::PopStyleVar();
}

// Draw Airport Search
void DrawAirportSearch(CommonObjects* common, char* airport_search_buf, size_t buf_size, std::vector<Airport>& search_results, Airport& selected_airport, nlohmann::json& arrivals_data, nlohmann::json& departures_data, float margin, float searchBarHeight, bool& show_airport_search) {
    // Set the cursor position for the search input box
    ImGui::SetCursorPos(ImVec2(margin, searchBarHeight + margin * 2));

    // Calculate the width of the text
    float textWidth = ImGui::CalcTextSize("Search Airport:").x;

    // Display the text
    ImGui::Text("Search Airport:");

    // Set the cursor position for the input box, right after the text
    ImGui::SameLine();
    ImGui::SetCursorPosX(margin + textWidth + 5); // Add a small gap between text and input box

    // Calculate remaining width for the input box
    float remainingWidth = ImGui::GetContentRegionAvail().x;

    // Create the input box with the remaining width
    ImGui::PushItemWidth(remainingWidth);
    ImGui::InputText("##AirportSearch", airport_search_buf, buf_size);
    ImGui::PopItemWidth();

    // Check if the search buffer is not empty
    if (strlen(airport_search_buf) > 0) {
        // Perform the airport search
        SearchAirports(airport_search_buf, common->Airports, search_results);

        // Set the cursor position for the search results list box
        ImGui::SetCursorPos(ImVec2(margin, searchBarHeight + margin * 4));
        if (ImGui::BeginListBox("##AirportResults", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
            // Iterate through the search results and display them
            for (const auto& airport : search_results) {
                std::string display_text = airport.iata + " - " + airport.name + " (" + airport.city + ", " + airport.country + ")";
                if (ImGui::Selectable(display_text.c_str())) {
                    // Set the selected airport and hide the search
                    selected_airport = airport;
                    show_airport_search = false;

                    try {
                        // Fetch arrival and departure data for the selected airport
                        std::cout << "Fetching arrival data for " << selected_airport.icao << std::endl;
                        arrivals_data = FetchFlightData("arrival", selected_airport.icao);
                        std::cout << "Fetching departure data for " << selected_airport.icao << std::endl;
                        departures_data = FetchFlightData("departure", selected_airport.icao);
                        std::cout << "Data fetched successfully" << std::endl;
                    }
                    catch (const std::exception& e) {
                        // Handle any errors during data fetching
                        std::cerr << "Error fetching flight data: " << e.what() << std::endl;
                    }
                }
            }
            ImGui::EndListBox();
        }
    }
}

// Draw Selected Airport
void DrawSelectedAirport(const Airport& selected_airport, const nlohmann::json& arrivals_data, const nlohmann::json& departures_data, float margin, float searchBarHeight) {
    // Get the main viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Calculate the width and height for the tables
    float table_width = (viewport->WorkSize.x - margin * 3) / 2;
    float table_height = viewport->WorkSize.y - (searchBarHeight + margin * 3); // Adjusted height

    // Set the cursor position for the arrivals table
    ImGui::SetCursorPos(ImVec2(margin, searchBarHeight + margin));
    ImGui::BeginChild("ArrivalsTable", ImVec2(table_width, table_height), true);
    // Display the arrivals table
    AirportFlightTable("Arrivals", arrivals_data, true);
    ImGui::EndChild();

    // Add a space between the two tables (seprate the two tables apart)
    ImGui::SameLine();

    ImGui::BeginChild("DeparturesTable", ImVec2(table_width, table_height), true);
    // Display the departures table
    AirportFlightTable("Departures", departures_data, false);
    ImGui::EndChild();
}

// Main Draw func for GUI
void DrawAppWindow(void* common_ptr) {
    auto common = static_cast<CommonObjects*>(common_ptr);
    static char search_buf[200] = "";
    static bool showAllFlightsInBackground = false;
    static bool show_airport_search = false;
    static bool show_favorites = false;
    static char airport_search_buf[200] = "";

    static std::vector<Airport> search_results;
    static Airport selected_airport;

    static nlohmann::json arrivals_data;
    static nlohmann::json departures_data;

    // Get the main viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("Background", nullptr, window_flags);

    float margin = 10.0f;
    float searchBarHeight = 30.0f;

    DrawSearchBar(search_buf, sizeof(search_buf), showAllFlightsInBackground, show_airport_search, show_favorites, selected_airport, arrivals_data, departures_data, airport_search_buf, sizeof(airport_search_buf));

    // Search results if search buffer is not empty and no other views are active
    if (common->data_ready && search_buf[0] != '\0' && !showAllFlightsInBackground && !show_airport_search && !show_favorites) {
        ImGui::SetCursorPos(ImVec2(margin, searchBarHeight + margin * 2));
        SearchAndDisplayFlights(common, search_buf);
    }

    // Display all active flights if the option is selected
    if (showAllFlightsInBackground && common->data_ready) {
        ImGui::SetCursorPos(ImVec2(margin, searchBarHeight + margin * 2));
        ShowAllActiveFlights(common);
    }

    // Display airport search results if the option is selected
    if (show_airport_search) {
        DrawAirportSearch(common, airport_search_buf, sizeof(airport_search_buf), search_results, selected_airport, arrivals_data, departures_data, margin, searchBarHeight, show_airport_search);
    }

    // Display selected airport's flight data if an airport is selected
    if (!selected_airport.icao.empty()) {
        DrawSelectedAirport(selected_airport, arrivals_data, departures_data, margin, searchBarHeight);
    }

    // Display favorite flights if the option is selected
    if (show_favorites) {
        ImGui::SetCursorPos(ImVec2(margin, searchBarHeight + margin * 2));
        DisplayFavoriteFlights(common);
    }

    ImGui::End();
}

// DrawThread operator overload
void DrawThread::operator()(CommonObjects& common) {
    // Log that the draw thread has started
    std::cout << "DrawThread started." << std::endl;
    // Loop until the exit flag is set
    while (!common.exit_flag) {
        // Call for main GUI function to drawing the app
        GuiMain(DrawAppWindow, &common);
        // Sleep for approximately 16 milliseconds to achieve ~60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}