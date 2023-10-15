#include "pch.h"
#include "InputLogger.h"

void InputLogger::RenderSettings() 
{
    if (ImGui::InputText("Key to toggle inputs logging.", &toggleInputLogsKey)) {
        cvarManager->getCvar("toggleInputLogsKey").setValue(toggleInputLogsKey);
    }

    ImGui::Text("Logs will be saved in %appdata%/bakkesmod/bakkesmod/data/inputLogs/*.csv");
}