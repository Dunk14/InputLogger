#include "pch.h"
#include "InputLogger.h"

#include <fstream>
#include <filesystem>
#include <chrono>

using namespace std;
using namespace chrono;

BAKKESMOD_PLUGIN(InputLogger, "Rocket League Input Logger", plugin_version, 0)

shared_ptr<CVarManagerWrapper> _globalCvarManager;

void InputLogger::onLoad()
{
	_globalCvarManager = cvarManager;

	filesystem::create_directory(gameWrapper->GetDataFolder() / "inputLogs");

	initVars();
	initHooks();
}

void InputLogger::onUnload()
{
	writeConfigFile();
}

void InputLogger::initVars()
{
	lastToggle = system_clock::now();

	cvarManager->registerCvar("toggleInputLogsKey", toggleInputLogsKey, "Keybind to toggle logging inputs.")
		 .addOnValueChanged([this](string old, CVarWrapper now) {
		toggleInputLogsKey = now.getStringValue();
		saveKeyIndex = gameWrapper->GetFNameIndexByString(toggleInputLogsKey);

		writeConfigFile();
	});

	saveKeyIndex = gameWrapper->GetFNameIndexByString(toggleInputLogsKey);

	if (ifstream(getConfigFilePath())) {
		cvarManager->loadCfg(getConfigFilePath().string());
	}

	for (auto& binding : gameWrapper->GetSettings().GetAllGamepadBindings()) {
		if (binding.second == "ToggleRoll") {
			airRollKeyIndex = gameWrapper->GetFNameIndexByString(binding.first);
		}
	}
}



void InputLogger::initHooks()
{
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", bind(&InputLogger::onTick, this, placeholders::_1));
}

void InputLogger::onTick(string eventName)
{
	if (gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) {
		// Check if save key has been pressed
		if (gameWrapper->IsKeyPressed(saveKeyIndex)) {
			toggleInputLogging();
		}

		// When saving activated get each move ingame
		if (saving) {
			registerMove();
		}
	}
}

void InputLogger::toggleInputLogging()
{
	const chrono::duration<double> diff = system_clock::now() - lastToggle;

	// Don't overreact if user stays on the save key, wait 500ms before toggling
	if (diff < duration_cast<system_clock::duration>(duration<double>(0.5))) { return; };
	lastToggle = chrono::system_clock::now();

	if (saving) {
		LOG("Stopping input logging.");

		// Prepare the files path, open it and save the content in it
		const auto time = system_clock::now().time_since_epoch();
		const string extension = ".csv";
		const string filename = to_string(time.count()) + extension;
		const auto path = gameWrapper->GetDataFolder() / "inputLogs" / filename;
		LOG("Path: " + path.string());

		ofstream stream(path, ios::out);
		stream << contentBuffer.value();

		// And reset the properties for a next call
		saving = false;
		contentBuffer.reset();
	}
	else {
		LOG("Starting input logging.");

		startingPoint = system_clock::now();

		// Put a 1st line of heading
		contentBuffer = "time,throttle,jump,boost,dodgeForward,dodgeStrafe,handBrake,roll,directionalAirRoll\n";

		// This property on `true` will tell to register each move for each tick
		saving = true;
	}
}

void InputLogger::registerMove()
{
	const auto inputs = gameWrapper->GetPlayerController().GetVehicleInput();
	const auto directionalAirRoll = gameWrapper->IsKeyPressed(airRollKeyIndex);

	if (!shouldSaveCurrentMove(inputs, directionalAirRoll)) { return; }

	// Compute time since registering started
	const auto time = to_string((system_clock::now() - startingPoint).count());

	const auto throttle = to_string(inputs.Throttle);
	const auto jump = to_string(inputs.Jump);
	const auto boost = to_string(inputs.ActivateBoost);
	const auto dodgeForward = to_string(inputs.DodgeForward);
	const auto dodgeStrafe = to_string(inputs.DodgeStrafe);
	const auto handBrake = to_string(inputs.Handbrake);
	const auto roll = to_string(inputs.Roll);

	const string line = time + "," + throttle + "," + jump + "," + boost + "," + dodgeForward + "," + dodgeStrafe + "," + handBrake + "," + roll + "," + to_string(directionalAirRoll) + "\n";

	contentBuffer = contentBuffer.value() + line;
}

bool InputLogger::shouldSaveCurrentMove(ControllerInput currentInputs, bool directionalAirRoll)
{
	// When registering 1st inputs save them directly
	if (!lastInputs) {
		lastInputs = currentInputs;
		lastDirectionalAirRoll = directionalAirRoll;
		return true;
	}

	// Otherwise check if last inputs are the same, if so don't save them
	if (currentInputs.Jump == lastInputs.value().Jump
		&& currentInputs.ActivateBoost == lastInputs.value().ActivateBoost
		&& currentInputs.DodgeForward == lastInputs.value().DodgeForward
		&& currentInputs.DodgeStrafe == lastInputs.value().DodgeStrafe
		&& currentInputs.Handbrake == lastInputs.value().Handbrake
		&& currentInputs.Pitch == lastInputs.value().Pitch
		&& currentInputs.Roll == lastInputs.value().Roll
		&& currentInputs.Steer == lastInputs.value().Steer
		&& currentInputs.Throttle == lastInputs.value().Throttle
		&& currentInputs.Yaw == lastInputs.value().Yaw
		&& directionalAirRoll == lastDirectionalAirRoll.value())
	{
		return false;
	};

	return true;
}


filesystem::path InputLogger::getConfigFilePath()
{
	return gameWrapper->GetBakkesModPath() / configFilePath;
}

void InputLogger::writeConfigFile()
{
	ofstream configurationFile;

	configurationFile.open(getConfigFilePath());

	configurationFile << "toggleInputLogsKey \"" + toggleInputLogsKey + "\"";
	configurationFile << "\n";

	configurationFile.close();
}