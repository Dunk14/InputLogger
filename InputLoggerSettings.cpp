#include "pch.h"
#include "InputLogger.h"

void InputLogger::RenderSettings() 
{
    ImGui::InputText("Key to toggle inputs logging.", &savekey);

    ImGui::Text("Logs will be saved in %appdata%/bakkesmod/bakkesmod/data/inputLogs/*.csv");
}