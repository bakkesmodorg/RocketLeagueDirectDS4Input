#include "DirectInputPlugin.h"
#include <dinputd.h>
BAKKESMOD_PLUGIN(DirectInputPlugin, "Direct Input plugin", "0.1", 0)



struct XINPUT_DEVICE_NODE
{
	DWORD dwVidPid;
	XINPUT_DEVICE_NODE* pNext;
};

struct DI_ENUM_CONTEXT
{
	DIJOYCONFIG* pPreferredJoyCfg;
	bool bPreferredJoyCfgValid;
};


bool running = false;

void read_inputs()
{

}
DirectInputPlugin* pluginInstance = NULL;
void DirectInputPlugin::onLoad()
{
	pluginInstance = this;
	cvarManager->registerNotifier("hid_list", std::bind(&DirectInputPlugin::OnConsoleCommand, this, std::placeholders::_1), "Lists all currently connected HIDs", PERMISSION_ALL);
	gameWrapper->HookEventWithCallerPost<PlayerControllerWrapper>("Function TAGame.PlayerController_TA.PlayerMove", 
		bind(&DirectInputPlugin::InputTick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	connect_to_ds4();
}

void DirectInputPlugin::onUnload()
{
	disconnect_ds4();
}


void DirectInputPlugin::InputTick(PlayerControllerWrapper cw, void * params, string funcName)
{
	HRESULT hr;
	TCHAR strText[512] = { 0 }; // Device state text
	DIJOYSTATE2 js;           // DInput joystick state 

	if (!m_inputDevice)
		return;

	// Poll the device to read the current state
	hr = m_inputDevice->Poll();
	if (FAILED(hr))
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = m_inputDevice->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = m_inputDevice->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return;
	}

	// Get the input's device state
	if (FAILED(hr = m_inputDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js)))
		return;

	cvarManager->log("input test: " + std::to_string(js.lX));
}

BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance,
	VOID* pContext)
{
	return pluginInstance->InternalEnumJoysticksCallback(pdidInstance, pContext);
}

BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi,
	VOID* pContext)
{
	HWND hDlg = (HWND)pContext;

	static int nSliderCount = 0;  // Number of returned slider controls
	static int nPOVCount = 0;     // Number of returned POV controls

	// For axes that are returned, set the DIPROP_RANGE property for the
	// enumerated axis in order to scale min/max values.
	if (pdidoi->dwType & DIDFT_AXIS)
	{
		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof(DIPROPRANGE);
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		diprg.diph.dwHow = DIPH_BYID;
		diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
		diprg.lMin = -1000;
		diprg.lMax = +1000;

		// Set the range for the axis
		if (FAILED(pluginInstance->m_inputDevice->SetProperty(DIPROP_RANGE, &diprg.diph)))
			return DIENUM_STOP;

	}

	return DIENUM_CONTINUE;
}

BOOL CALLBACK DirectInputPlugin::InternalEnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance,
	VOID* pContext)
{
	auto pEnumContext = reinterpret_cast<DI_ENUM_CONTEXT*>(pContext);
	HRESULT hr;

	// Skip anything other than the perferred joystick device as defined by the control panel.  
	// Instead you could store all the enumerated joysticks and let the user pick.
	if (pdidInstance->guidInstance.Data1 == 164365644L || pdidInstance->guidInstance.Data1 == 170061648L) //Filter out @bakkes wireless controller
	{
		cvarManager->log("Skipping input device " + std::to_string(pdidInstance->guidInstance.Data1));
		//SAFE_RELEASE(m_inputDevice);
		return DIENUM_CONTINUE;
	}
	cvarManager->log("Received device " + to_string(pdidInstance->guidInstance.Data1));
	/*if (pEnumContext->bPreferredJoyCfgValid &&
		!IsEqualGUID(pdidInstance->guidInstance, pEnumContext->pPreferredJoyCfg->guidInstance))
		return DIENUM_CONTINUE;*/

	// Obtain an interface to the enumerated joystick.
	hr = m_directInput->CreateDevice(pdidInstance->guidInstance, &m_inputDevice, nullptr);

	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if (FAILED(hr))
		return DIENUM_CONTINUE;
	DIDEVICEINSTANCE deviceInst;
	deviceInst.dwSize = sizeof(DIDEVICEINSTANCE);
	m_inputDevice->GetDeviceInfo(&deviceInst);

	cvarManager->log("Found input device: " + std::string(deviceInst.tszProductName) + " - " + 
		std::to_string(deviceInst.guidProduct.Data1) + "-" + std::to_string(deviceInst.guidProduct.Data2) + "-" + 
		std::to_string(deviceInst.guidProduct.Data3) + "-" + std::string((char*)deviceInst.guidProduct.Data4));
	
	

	// Stop enumeration. Note: we're just taking the first joystick we get. You
	// could store all the enumerated joysticks and let the user pick.
	return DIENUM_STOP;
}


void DirectInputPlugin::connect_to_ds4()
{
	HRESULT result;
	result = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		cvarManager->log("Could not initialize direct input");
		return;
	}
	DIJOYCONFIG PreferredJoyCfg = { 0 };
	DI_ENUM_CONTEXT enumContext;
	enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
	enumContext.bPreferredJoyCfgValid = false;

	IDirectInputJoyConfig8* pJoyConfig = nullptr;
	if (FAILED(result = m_directInput->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&pJoyConfig))) {
		cvarManager->log("Could not initialize direct input (2)");
		return;
	}


	PreferredJoyCfg.dwSize = sizeof(PreferredJoyCfg);
	if (SUCCEEDED(pJoyConfig->GetConfig(0, &PreferredJoyCfg, DIJC_GUIDINSTANCE))) // This function is expected to fail if no joystick is attached
		enumContext.bPreferredJoyCfgValid = true;
	SAFE_RELEASE(pJoyConfig);
	result = m_directInput->EnumDevices(DI8DEVCLASS_GAMECTRL,
		EnumJoysticksCallback,
		&enumContext, DIEDFL_ATTACHEDONLY);
	// Look for a simple joystick we can use for this sample program.
	if (FAILED(result) || m_inputDevice == NULL)
	{
		cvarManager->log("No suitable input devices found");
		return;
	}


	if (FAILED(result = m_inputDevice->SetDataFormat(&c_dfDIJoystick2)))
	{
		cvarManager->log("Something wrong with input device stuff (1)");
		return;
	}

	hDlg = GetActiveWindow();

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
	if (FAILED(result = m_inputDevice->SetCooperativeLevel(hDlg, DISCL_EXCLUSIVE |
		DISCL_FOREGROUND)))
	{
		cvarManager->log("Something wrong with input device stuff (2)");
		return;
	}

	// Enumerate the joystick objects. The callback function enabled user
	// interface elements for objects that are found, and sets the min/max
	// values property for discovered axes.
	if (FAILED(result = m_inputDevice->EnumObjects(EnumObjectsCallback,
		(VOID*)hDlg, DIDFT_ALL)))
	{
		cvarManager->log("Something wrong with input device stuff (3)");
		return;
	}

}

void DirectInputPlugin::disconnect_ds4()
{

}


std::string string_format(const std::string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::string(formatted.get());
}

void DirectInputPlugin::OnConsoleCommand(std::vector<std::string> parameters)
{
	
}
