#pragma once
#include "CommonObject.h"
#include "DownloadThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"

class DrawThread {
public:
    void operator()(CommonObjects& common);
    void ShowAllActiveFlights(CommonObjects* common, char* search_buf = nullptr);
};