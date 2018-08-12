#include "DirectInputPlugin.h"

BAKKESMOD_PLUGIN(DirectInputPlugin, "Direct Input plugin", "0.1", 0)



static DS4 lastInput;
static int lastResult = 0;

hid_device *handle = NULL;
bool running = false;

void read_inputs()
{
	DWORD bytes_read = 0;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hid_set_nonblocking(handle, READ_NONBLOCKING);
	while (running)
	{
		ResetEvent(overlapped.hEvent);
		if (handle) {
			lastResult = hid_read(handle, (u8*)&lastInput, 64);
			//if (lastResult == -1)
			//{
			//	//Error = GetLastError();
			//}
		}
		Sleep(READ_THREAD_SLEEP_INTERVAL);
	}
}
void DirectInputPlugin::onLoad()
{
	gameWrapper->HookEventWithCallerPost<PlayerControllerWrapper>("Function TAGame.PlayerController_TA.PlayerMove", bind(&DirectInputPlugin::InputTick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	connect_to_ds4();
}

void DirectInputPlugin::onUnload()
{
	disconnect_ds4();
}



void DirectInputPlugin::InputTick(PlayerControllerWrapper cw, void * params, string funcName)
{
	if (!handle)
		return;
	
	ControllerInput input = cw.GetVehicleInput();
#ifdef USE_BAKKESMOD_CONTROLSCHEME
	input.Steer = SCALE_LEFTSTICK(lastInput.leftx);
	input.Yaw = SCALE_LEFTSTICK(lastInput.leftx);
	input.Pitch = SCALE_LEFTSTICK(lastInput.lefty);
	input.Throttle = SCALE_R2(lastInput.r2) - SCALE_L2(lastInput.l2);
	input.Roll = BUTTON_R1_PRESSED(lastInput.buttons) ? 1 : 0;
	input.Roll -= BUTTON_L1_PRESSED(lastInput.buttons) ? 1 : 0;

	cw.SetVehicleInput(input);
	cw.ToggleJump(BUTTON_CROSS_PRESSED(lastInput.buttons));
	cw.ToggleHandbrake(BUTTON_L1_PRESSED(lastInput.buttons) | BUTTON_R1_PRESSED(lastInput.buttons));
	cw.ToggleBoost(BUTTON_CIRCLE_PRESSED(lastInput.buttons));
#elif defined(USE_DEFAULT_CONTROLSCHEME)
	input.Throttle = SCALE_R2(lastInput.r2) - SCALE_L2(lastInput.l2);
	input.Steer = SCALE_LEFTSTICK(lastInput.leftx);
	input.Pitch = SCALE_LEFTSTICK(lastInput.lefty);

	//Use ternary since we actually need to reset to 0 if button not pressed
	bool airRolling = BUTTON_SQUARE_PRESSED(lastInput.buttons);
	input.Yaw = airRolling ? 0 : SCALE_LEFTSTICK(lastInput.leftx);
	input.Roll = airRolling ? SCALE_LEFTSTICK(lastInput.leftx) : 0;


	cw.SetVehicleInput(input);
	cw.ToggleJump(BUTTON_CROSS_PRESSED(lastInput.buttons));
	cw.ToggleHandbrake(BUTTON_L1_PRESSED(lastInput.buttons) | BUTTON_R1_PRESSED(lastInput.buttons));
	cw.ToggleBoost(BUTTON_CIRCLE_PRESSED(lastInput.buttons));
#endif
}

void DirectInputPlugin::connect_to_ds4()
{
	if (!(handle = hid_open(0x54c, 0x09cc, NULL))) {//0x5c4
		cvarManager->log("Unable to connect to controller");
		hid_exit();
		return;
	}

	running = true;
	DS4Out ds4_out = { 0x0004ff05, 0, 0, 255, 0, 185 }; //Make bar pink to indicate we have a connection
	int res = hid_write(handle, (u8 *)&ds4_out, sizeof(ds4_out));
	inputThread = std::thread(read_inputs);
}

void DirectInputPlugin::disconnect_ds4()
{
	running = false;
	inputThread.join();
	if (!handle)
		return;
	hid_close(handle);
	hid_exit();
}
