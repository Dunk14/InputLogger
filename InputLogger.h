#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"

using namespace std;

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class InputLogger: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	//shared_ptr<bool> enabled;

	//Boilerplate
	void onLoad() override;
	void initVars();
	void initHooks();

	//void onUnload() override; // Uncomment and implement if you need a unload method

public:
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window

	string savekey = "F9";

private:
	//shared_ptr<PersistentStorage> persistentStorage;
	void onTick(string eventName);
	void toggleInputLogging();
	void registerMove();
	bool shouldSaveCurrentMove();
	
	bool saving = false;
	int tick = 0;
	chrono::system_clock::time_point lastToggle;
	chrono::system_clock::time_point startingPoint;
	optional<ControllerInput> lastInputs;
	string contentBuffer = "";
};
