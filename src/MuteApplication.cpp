#include <windows.h>
#include <iostream>
#include <string>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <comdef.h>


char title[256];
std::string s_title;
HWND hwnd;
MSG msg = { 0 };
HRESULT hr = S_OK;
IMMDeviceEnumerator *pEnumerator = NULL;
IMMDeviceCollection *pCollection = NULL;
IMMDevice *pEndpoint = NULL;
IPropertyStore *pProps = NULL;
LPWSTR pwszID = NULL;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

void printEndpoints() {
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	if (FAILED(hr)) {
		// Decode error message
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		printf(errMsg);
	}
	system("pause");
}


int main() {
	std::cout << "Mute Focused Application" << std::endl;
	if (RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x70)) std::cout << (("Mute / Unmute: F1")) << std::endl; // 0x70 = F1	
	if (RegisterHotKey(NULL, 2, MOD_NOREPEAT, 0x73)) std::cout << (("Exit: F3")) << std::endl; // 0x73 = F4
	while (GetMessage(&msg, NULL, 0, 0) != 0) {
		if (msg.message == WM_HOTKEY) {
			if (msg.wParam = 1) {
				printEndpoints();
			}
			if (msg.wParam = 2) {
				break;
			}
		}
	}
}
