#include "CommonObject.h"
#include "DrawThread.h"
#include "DownloadThread.h"  

int main() {
    CommonObjects common;
    DrawThread draw;
    DownloadThread down;

    down.SetUrl("https://opensky-network.org");  // Set the URL for downloading data from OpenSky

    // Launch the draw thread to handle the GUI rendering in a separate thread
    auto draw_th = std::jthread([&] { draw(common); });

    // Launch the download thread to handle fetching data in a separate thread
    auto down_th = std::jthread([&] { down(common); });

    std::cout << "Exe is launched...\n";  // Output a message indicating that the application is running

    // Wait for both threads (draw and download) to finish before exiting
    down_th.join();
    draw_th.join();

    return 0;  // Return 0 to indicate successful execution
}