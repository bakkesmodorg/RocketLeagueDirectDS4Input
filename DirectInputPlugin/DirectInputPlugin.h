#pragma once
#define DIRECTINPUT_VERSION 0x0800
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <dinput.h>

/*
Lightweight plugin that reads DS4 controller values directly, skipping all abstraction layers.
Configuration such as deadzones and multipliers are done at compile time to increase performance.
Right now, it does not yet block Rocket League from reading the input, so it is recommended to unbind controls you're overriding with this plugin
*/

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/PlayerControllerWrapper.h"
#include <hidapi.h>
#include <windows.h>

#pragma comment( lib, "bakkesmod.lib" )

/*
Configuration settings
*/

/*
Uncomment one of the control schemes or add your own
*/
#define USE_BAKKESMOD_CONTROLSCHEME
//#define USE_DEFAULT_CONTROLSCHEME

/*
Deadzone configuration in bytes (Controller range from 0-256 where 0 is fully to the left, 256 fully to the right)
Use the painter to get values that fit you
*/

/*
Inner deadzone for controller (useful for when your stick is slightly stuck to one side
Example: 10% inner deadzone all inputs 123 +- 12 are ignored, so if you want 10% deadzone, set LS_INNER_DEADZONE to 12.
*/
#define LS_INNER_DEADZONE 10 //10 is ~8.3%

/*
Outer deadzone
*/
#define LS_OUTER_DEADZONE 2

/*
Left stick sensitivity multiplier, if not defined will use 1
*/
#define LS_MULTIPLIER 1.3f

/*
Throttle deadzones
*/
#define R2_INNER_DEADZONE 5
#define R2_OUTER_DEADZONE 20

/*
Reverse deadzones
*/
#define L2_INNER_DEADZONE 5
#define L2_OUTER_DEADZONE 20

/*
If false it will block the input reading thread and wait until a new controller state is received from ReadFile.
*/
#define READ_NONBLOCKING true

/*
The amount of time (in ms) to sleep between controller readings, lower value = more readings but might impact performance.
There is probably a minimum threshold since the DS4 controller has a maximum polling rate, but cba looking it up now
*/
#define READ_THREAD_SLEEP_INTERVAL 1

/*
DS4 product info
*/

/*Vendor, always 0x54c I think*/
#define VENDOR_ID 0x54c

/*The product ID of the hid, might differ depending on which version you have, use hid_list to list all devices and look for Sony to find yours*/
#define PRODUCT_ID 0x09cc

/*
Calculations
*/

/*
Scales 0-256 from -1 to 1 with given inner deadzone
*/
#define SCALE_BYTE(val, minval, maxval, innerdeadzone) (abs(val - 123) < innerdeadzone ? 0.f : 2.f * (val - minval) / (maxval - minval) - 1.f)

#ifndef LS_MULTIPLIER
#define SCALE_LEFTSTICK(val) SCALE_BYTE(val, LS_OUTER_DEADZONE, 256-LS_OUTER_DEADZONE, LS_INNER_DEADZONE)
#else
#define SCALE_LEFTSTICK(val) (SCALE_BYTE(val, LS_OUTER_DEADZONE, 256-LS_OUTER_DEADZONE, LS_INNER_DEADZONE) * LS_MULTIPLIER)
#endif

/*Scales 0-256 from 0 to 1 */
#define SCALE_BYTE_SINGLE(val, minval, maxval, innerdeadzone) (val < innerdeadzone ? 0.f : (val - minval) / (maxval - minval))

#define SCALE_R2(val) SCALE_BYTE_SINGLE(val, R2_OUTER_DEADZONE.f, 256-R2_OUTER_DEADZONE, R2_INNER_DEADZONE.f)
#define SCALE_L2(val) SCALE_BYTE_SINGLE(val, L2_OUTER_DEADZONE.f, 256-L2_OUTER_DEADZONE, L2_INNER_DEADZONE.f)

/*
DS4 button constants
*/
#define BUTTON_TRIANGLE (1 << 7)
#define BUTTON_CIRCLE (1 << 6)
#define BUTTON_CROSS (1 << 5)
#define BUTTON_SQUARE (1 << 4)
#define BUTTON_L1 (1 << 8)
#define BUTTON_R1 (1 << 9)

#define BUTTON_TRIANGLE_PRESSED(x) ((x >> 7) & 1)
#define BUTTON_CIRCLE_PRESSED(x) ((x >> 6) & 1)
#define BUTTON_CROSS_PRESSED(x) ((x >> 5) & 1)
#define BUTTON_SQUARE_PRESSED(x) ((x >> 4) & 1)
#define BUTTON_L1_PRESSED(x) ((x >> 8) & 1)
#define BUTTON_R1_PRESSED(x) ((x >> 9) & 1)


/*
DirectInput for rocket league, code based on https://github.com/uucidl/pre.neige/blob/master/ds4.cpp
*/
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
class DirectInputPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	bool inputDeviceConnected = false;
	std::thread inputThread;


public:
	HWND hDlg = NULL;
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_inputDevice;

	virtual void onLoad();
	virtual void onUnload();

	/*Function that is executed after every playermove*/
	void InputTick(PlayerControllerWrapper cw, void* params, string funcName);

	/*Establishes a connection to a ds4 controller*/
	void connect_to_ds4();

	/*Disconnects from the ds4 controller*/
	void disconnect_ds4();
	BOOL CALLBACK InternalEnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance,
		VOID* pContext);
	void OnConsoleCommand(std::vector<std::string> parameters);
};
