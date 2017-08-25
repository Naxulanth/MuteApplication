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
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto exit; }

#define SAFE_RELEASE(p)  \
			   if ((p) != NULL)  \
                { (p)->Release(); (p) = NULL; } 

void failCheck(HRESULT hres, IMMDeviceEnumerator *pEnumerator, IMMDevice *pEndpoint, IPropertyStore *pProps, IMMDeviceCollection *pCollection,
	LPWSTR pwszID, IAudioClient *pClient) {
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
		SAFE_RELEASE(pClient);
	}
}

void findEndpoint() {
	HRESULT hr = S_OK;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDeviceCollection *pCollection = NULL;
	IMMDevice *pEndpoint = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR pwszID = NULL;
	IAudioClient *pClient = NULL;
	WAVEFORMATEX *wave = NULL;
	ISimpleAudioVolume *pVolume = NULL;
	REFERENCE_TIME ref = REFTIMES_PER_SEC;

	UINT  count;

	PROPVARIANT varName;

	// Initialize COM library
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	// Create object
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	EXIT_ON_ERROR(hr)
		// Enumerate endpoints
		hr = pEnumerator->EnumAudioEndpoints(
			eRender, DEVICE_STATE_ACTIVE,
			&pCollection);
	EXIT_ON_ERROR(hr)
		hr = pCollection->GetCount(&count);
	EXIT_ON_ERROR(hr)
		if (count == 0)
		{
			printf("No endpoints found.\n");
		}
	// Get default endpoint
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pEndpoint);
	EXIT_ON_ERROR(hr)
		// Get the endpoint ID
		hr = pEndpoint->GetId(&pwszID);
	EXIT_ON_ERROR(hr)

		hr = pEndpoint->OpenPropertyStore(
			STGM_READ, &pProps);
	EXIT_ON_ERROR(hr)

		// Initialize container for property value.
		PropVariantInit(&varName);

	// Get the endpoint's friendly name.
	hr = pProps->GetValue(
		PKEY_Device_FriendlyName, &varName);
	EXIT_ON_ERROR(hr)

		// Print endpoint friendly name and endpoint ID.
		printf("Default endpoint: \"%S\" (%S)\n", varName.pwszVal, pwszID);

	hr = pEndpoint->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pClient);
	EXIT_ON_ERROR(hr)

		hr = pClient->GetMixFormat(&wave);
	EXIT_ON_ERROR(hr)

		hr = pClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, ref, 0, wave, NULL);
	EXIT_ON_ERROR(hr)

		hr = pClient->GetService(IID_ISimpleAudioVolume, (void**)&pVolume);
	EXIT_ON_ERROR(hr)

		hr = pVolume->SetMute(TRUE, NULL);
	EXIT_ON_ERROR(hr)

		// Cleanup
		exit:
	_com_error err(hr);
	LPCTSTR errMsg = err.ErrorMessage();
	printf(errMsg);
	CoTaskMemFree(pwszID);
	pwszID = NULL;
	PropVariantClear(&varName);
	SAFE_RELEASE(pProps)
		SAFE_RELEASE(pEndpoint)
		SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pCollection)
		SAFE_RELEASE(pClient)
		SAFE_RELEASE(pVolume)
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
