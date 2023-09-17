#include "pch.h"
#include "InputLogger.h"

#include <fstream>
#include <filesystem>
#include <chrono>

using namespace std;
using namespace chrono;

BAKKESMOD_PLUGIN(InputLogger, "Rocket League Input Logger", plugin_version, PLUGINTYPE_FREEPLAY)

shared_ptr<CVarManagerWrapper> _globalCvarManager;

void InputLogger::onLoad()
{
	_globalCvarManager = cvarManager;

	filesystem::create_directory(gameWrapper->GetDataFolder() / "inputLogs");

	initVars();
	initHooks();

	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	//DEBUGLOG("InputLogger debug mode enabled");

	// LOG and DEBUGLOG use fmt format strings https://fmt.dev/latest/index.html
	//DEBUGLOG("1 = {}, 2 = {}, pi = {}, false != {}", "one", 2, 3.14, true);

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](vector<string> args) {
	//	LOG("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](string cvarName, CVarWrapper newCvar) {
	//	LOG("the cvar with name: {} changed", cvarName);
	//	LOG("the new value is: {}", newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(bind(&InputLogger::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](vector<string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", bind(&InputLogger::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](string eventName) {
	//	LOG("Your hook got called and the ball went POOF");
	//});
	// You could also use bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", bind(&InputLogger::YourPluginMethod, this);
}

void InputLogger::initVars()
{
	lastToggle = system_clock::now();

	cvarManager->registerCvar("inputlogger_savekey", savekey, "Keybind to toggle logging inputs.")
		.addOnValueChanged([this](string old, CVarWrapper now) {
		savekey = now.getStringValue();
			});

	/*
	cvarManager->registerCvar("inputlogger_path", path, "Path to save those logs.")
		.addOnValueChanged([this](string old, CVarWrapper now) {
		path = now.getStringValue();
	});*/

	//persistentStorage = make_shared<PersistentStorage>(this, "InputLoggerConfig", true, true);

	//// Save key
	//persistentStorage->RegisterPersistentCvar("inputlogger_savekey", "F9", "Keybind to toggle logging inputs.")
	//	.addOnValueChanged([this](string old, CVarWrapper now) {
	//		savekey = now.getStringValue();
	//	});

	//const string userProfile = getenv("USERPROFILE");
	//path = userProfile + "/\Documents/\My Games/\Rocket League/\InputLogs";

	//persistentStorage->RegisterPersistentCvar("inputlogger_path", path, "Path to save those logs.")
	//	.addOnValueChanged([this](string old, CVarWrapper now) {
	//		path = now.getStringValue();
	//});
}

void InputLogger::initHooks()
{
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", bind(&InputLogger::onTick, this, placeholders::_1));
}

void InputLogger::onTick(string eventName)
{
	if (!gameWrapper->IsInReplay()) {
		// Check if save key has been pressed
		if (gameWrapper->IsKeyPressed(gameWrapper->GetFNameIndexByString(savekey))) {
			InputLogger::toggleInputLogging();
		}

		// When saving activated get each move ingame
		if (saving) {
			InputLogger::registerMove();
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

		std::ofstream stream(path, ios::out);
		stream << contentBuffer;

		// And reset the properties for a next call
		saving = false;
		tick = 0;
		contentBuffer = "";
	}
	else {
		LOG("Starting input logging.");

		startingPoint = system_clock::now();

		// Put a 1st line of heading
		contentBuffer += "time;jump;boost;dodgeForward;dodgeStrage;handBrake;pitch;roll;steer;throttle;yaw\n";

		// This property on `true` will tell to register each move for each tick
		saving = true;
	}
}

void InputLogger::registerMove()
{
	if (!InputLogger::shouldSaveCurrentMove()) { return; }

	// Compute time since registering started
	const auto time = to_string((system_clock::now() - startingPoint).count());

	const auto inputs = gameWrapper->GetPlayerController().GetLastInputs();
	const auto jump = to_string(inputs.Jump);
	const auto boost = to_string(inputs.ActivateBoost);
	const auto dodgeForward = to_string(inputs.DodgeForward);
	const auto dodgeStrafe = to_string(inputs.DodgeStrafe);
	const auto handBrake = to_string(inputs.Handbrake);
	const auto pitch = to_string(inputs.Pitch);
	const auto roll = to_string(inputs.Roll);
	const auto steer = to_string(inputs.Steer);
	const auto throttle = to_string(inputs.Throttle);
	const auto yaw = to_string(inputs.Yaw);

	const string line = time + ";" + jump + ";" + boost + ";" + dodgeForward + ";" + dodgeStrafe + ";" + handBrake + ";" + pitch + ";" + roll + ";" + steer + ";" + throttle + ";" + yaw + "\n";

	contentBuffer += line;

	tick++;
}

bool InputLogger::shouldSaveCurrentMove()
{
	const auto currentInputs = gameWrapper->GetPlayerController().GetLastInputs();

	// When registering 1st inputs save them directly
	if (!lastInputs) {
		lastInputs = currentInputs;
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
		&& currentInputs.Yaw == lastInputs.value().Yaw)
	{
		return false;
	};

	return true;
}