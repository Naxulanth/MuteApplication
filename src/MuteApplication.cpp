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

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);


#define SAFE_RELEASE(p)  \
			   if ((p) != NULL)  \
                { (p)->Release(); (p) = NULL; } 

void failCheck(HRESULT hres, IMMDeviceEnumerator *pEnumerator, IMMDevice *pEndpoint, IPropertyStore *pProps, IMMDeviceCollection *pCollection,
	LPWSTR pwszID) {
	if (FAILED(hres)) {
		// Decode error message
		_com_error err(hres);
		LPCTSTR errMsg = err.ErrorMessage();
		printf(errMsg);
		CoTaskMemFree(pwszID);
		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pCollection);
		SAFE_RELEASE(pEndpoint);
		SAFE_RELEASE(pProps);
	}
}

void findEndpoint() {
	HRESULT hr = S_OK;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDeviceCollection *pCollection = NULL;
	IMMDevice *pEndpoint = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR pwszID = NULL;

	UINT  count;

	PROPVARIANT varName;

	// Initialize COM library
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	// Create object
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	failCheck(hr, pEnumerator, pEndpoint, pProps, pCollection, pwszID);
	// Enumerate endpoints
	hr = pEnumerator->EnumAudioEndpoints(
		eRender, DEVICE_STATE_ACTIVE,
		&pCollection);
	failCheck(hr, pEnumerator, pEndpoint, pProps, pCollection, pwszID);
	hr = pCollection->GetCount(&count);
	failCheck(hr, pEnumerator, pEndpoint, pProps, pCollection, pwszID);
	if (count == 0)
	{
		printf("No endpoints found.\n");
	}
	// Get default endpoint
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pEndpoint);
	failCheck(hr, pEnumerator, pEndpoint, pProps, pCollection, pwszID);
	// Get the endpoint ID
	hr = pEndpoint->GetId(&pwszID);
	failCheck(hr, pEnumerator, pEndpoint, pProps, pCollection, pwszID);

	hr = pEndpoint->OpenPropertyStore(
		STGM_READ, &pProps);
	failCheck(hr, pEnumerator, pEndpoint, pProps, pCollection, pwszID);

	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the endpoint's friendly name.
	hr = pProps->GetValue(
		PKEY_Device_FriendlyName, &varName);
	failCheck(hr, pEnumerator, pEndpoint, pProps, pCollection, pwszID);

	// Print endpoint friendly name and endpoint ID.
	printf("Default endpoint: \"%S\" (%S)\n", varName.pwszVal, pwszID);

	// Cleanup
	CoTaskMemFree(pwszID);
	pwszID = NULL;
	PropVariantClear(&varName);
	SAFE_RELEASE(pProps)
	SAFE_RELEASE(pEndpoint)
	SAFE_RELEASE(pEnumerator)
	SAFE_RELEASE(pCollection)
	system("pause");
	return;
}


int main() {
	std::cout << "Mute Focused Application" << std::endl;
	if (RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x70)) std::cout << (("Mute / Unmute: F1")) << std::endl; // 0x70 = F1	
	if (RegisterHotKey(NULL, 2, MOD_NOREPEAT, 0x73)) std::cout << (("Exit: F3")) << std::endl; // 0x73 = F4
	while (GetMessage(&msg, NULL, 0, 0) != 0) {
		if (msg.message == WM_HOTKEY) {
			if (msg.wParam = 1) {
				findEndpoint();
			}
			if (msg.wParam = 2) {
				break;
			}
		}
	}
}
