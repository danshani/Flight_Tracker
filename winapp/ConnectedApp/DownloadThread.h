#pragma once
#include "CommonObject.h"
#include <string_view>

class DownloadThread{
public:
    void operator()(CommonObjects& common);
    void SetUrl(std::string_view new_url);
    void LoadAirportsFromJson(CommonObjects& common_objects, const std::string& airports_file);
    std::unordered_map<std::string, Airport> LoadAirportsData(const std::string& file_path);
private:
    std::string _download_url;
    std::string opensky_username = "danshani11";
    std::string opensky_password = "6LS6eecDG@BDx2y";
    std::string airports_file = "D:/Json/airports.json";
};
