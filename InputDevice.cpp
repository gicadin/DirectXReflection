#include "includefiles.h"

InputDevice::InputDevice()
{
	m_pDevice = NULL;
	m_x = 0;
	m_y = 0;

	ZeroMemory(m_keyLock, sizeof(BOOL) * 256);
	ZeroMemory(&m_mouseState, sizeof(DIMOUSESTATE));
	ZeroMemory(m_keyboardState, 256);
	ZeroMemory(m_pressedKeys, 256);
	ZeroMemory(m_pressedButtons, 4);
}

BOOL InputDevice::init(LPDIRECTINPUT8 pDI, HWND hWnd, DIRECTINPUTTYPE type)
{
	// Check for a valid parent DIRECTINPUT8 interface
	if (pDI == NULL || type == DIT_FORCE_DWORD)
	{
		return FALSE;
	}
	release();

	DIDATAFORMAT* pDataFormat;
	m_hWnd = hWnd;
	m_type = type;

	switch (type) 
	{
	case DIT_KEYBOARD:
		if (FAILED(pDI->CreateDevice(GUID_SysKeyboard, &m_pDevice, NULL)))
		{
			MessageBox(0, TEXT("Unable to create keyboard device."), 0, 0);
			return FALSE;
		}
		pDataFormat = (DIDATAFORMAT*)&c_dfDIKeyboard;
		break;
	case DIT_MOUSE:
		if (FAILED(pDI->CreateDevice(GUID_SysMouse, &m_pDevice, NULL)))
		{
			MessageBox(0, TEXT("Unable to create mouse device."), 0, 0);
			return FALSE;
		}
		pDataFormat = (DIDATAFORMAT*)&c_dfDIMouse;
	default:
		return FALSE;
	}

	// Set the data format and puts it in the pointer we give it 
	// pointe describes what format the data returns eg. keyboard / mouse / etc...
	if (FAILED(m_pDevice->SetDataFormat(pDataFormat)))
	{
		MessageBox(0, TEXT("Unable to set input data format."), 0, 0);
		return FALSE;
	}

	// Set the cooperative level ?!?!?!?!? what the heck is this?!?! and WHY
	/*
	The cooperative level determines how this instance of the device interacts with other 
	instances of the device and the rest of the system. 
	Application needs to be in the forground and access to the device
	does not interfere with other applications that are using the same device... 
	(fine...we'll play nice)
	*/
	if (FAILED(m_pDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		MessageBox(0, TEXT("Unable to set input cooperative level."), 0, 0);
		return FALSE;
	}

	// Acquire the device
	if (FAILED(m_pDevice->Acquire()))
	{
		MessageBox(0, TEXT("Unable to acquire the input device."), 0, 0);
		return FALSE;
	}

	return TRUE;			// finally
}

void InputDevice::read()
{
	if (!m_pDevice)
	{
		return; // something went wrong or nothing there
	}

	// Get data
	if (m_type == DIT_MOUSE)
	{
		// We don't care about mouse :)
	}
	else if (m_type == DIT_KEYBOARD)
	{
		HRESULT hr = m_pDevice->GetDeviceState(256, (LPVOID)&m_keyboardState);\
		if ( FAILED(hr) ){
			if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
				m_pDevice->Acquire();		// Device is lost, try to reaquire it
			return;
		}

		// Get pressed keys and release locks on key up
		for (int i = 0; i < 256; i++)
		{
			if (!(m_keyboardState[i] & 0x80))
			{
				// Key is up so release lock
				m_keyLock[i] = FALSE;
				m_pressedKeys[i] = FALSE;
			}
			else
			{
				// Key is pressed if it isn't locked
				m_pressedKeys[i] = !(m_keyLock[i]);
			}
		}
	}


}

// So the key doesn't get pressed every frame
void InputDevice::lockKey(DWORD key)
{
	m_keyLock[key] = TRUE;
}

void InputDevice::release()
{
	if (m_pDevice)
	{
		m_pDevice->Unacquire();
		m_pDevice->Release();
	}
}