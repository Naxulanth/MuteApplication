#include <windows.h>
#include <iostream>
#include <string>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <comdef.h>

// Macros
#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000

// Exit on HRESULT error
#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto exit; }

// Release pointer
#define SAFE_RELEASE(p)  \
			   if ((p) != NULL)  \
                { (p)->Release(); (p) = NULL; } 

char title[256];
std::string s_title;
HWND hwnd;
MSG msg = { 0 };
DWORD pid;

// Enumerations
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
const IID IID_IAudioStreamVolume = __uuidof(IAudioStreamVolume);
const IID IID_IAudioSessionControl = __uuidof(IAudioSessionControl);
const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);
const IID IID_IAudioSessionManager = __uuidof(IAudioSessionManager);


void muteProcess(DWORD pid) {
    HRESULT hr = S_OK;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDeviceCollection *pCollection = NULL;
    IAudioSessionControl *pControl = NULL;
    IAudioSessionManager2 *pManager2 = NULL;
    IAudioSessionManager *pManager = NULL;
    IAudioSessionEnumerator *pSessions = NULL;
    IAudioSessionControl2 *pControl2 = NULL;
    ISimpleAudioVolume *psVolume = NULL;
    IMMDevice *pEndpoint = NULL;
    IPropertyStore *pProps = NULL;
    REFERENCE_TIME ref = REFTIMES_PER_SEC;
    UINT count;
    int sessionCount = 0;
    PROPVARIANT varName;
    DWORD pProcessId;

    // Initialize COM library
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    // Create object
    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);
    // Enumerate endpoints
    hr = pEnumerator->EnumAudioEndpoints(
        eRender, DEVICE_STATE_ACTIVE,
        &pCollection);
    EXIT_ON_ERROR(hr);
    hr = pCollection->GetCount(&count);
    EXIT_ON_ERROR(hr);
    if (count == 0)
    {
        printf("No endpoints found.\n");
    }
    // Get default endpoint
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pEndpoint);
    EXIT_ON_ERROR(hr);

    // Get endpoint name
    hr = pEndpoint->OpenPropertyStore(
        STGM_READ, &pProps);
    EXIT_ON_ERROR(hr);

    PropVariantInit(&varName);

    hr = pProps->GetValue(
        PKEY_Device_FriendlyName, &varName);
    EXIT_ON_ERROR(hr);

    // Print endpoint name
    printf("Default endpoint: \"%S\"\n", varName.pwszVal);

    // Initialize necessary interfaces

    hr = pEndpoint->Activate(IID_IAudioSessionManager2, CLSCTX_ALL, NULL, (void**)&pManager2);
    EXIT_ON_ERROR(hr);

    hr = pEndpoint->Activate(IID_IAudioSessionManager, CLSCTX_ALL, NULL, (void**)&pManager);
    EXIT_ON_ERROR(hr);

    // Enumerate all sessions

    hr = pManager2->GetSessionEnumerator(&pSessions);
    EXIT_ON_ERROR(hr);

    // Get the amount of sessions

    hr = pSessions->GetCount(&sessionCount);
    EXIT_ON_ERROR(hr);

    // Search for the chosen process in all audio sessions, and toggle mute

    for (int i = 0; i < sessionCount; ++i)
    {
        BOOL mute;

        SAFE_RELEASE(pControl);

        hr = pSessions->GetSession(i, &pControl);
        EXIT_ON_ERROR(hr);
        hr = pControl->QueryInterface<IAudioSessionControl2>(&pControl2);
        EXIT_ON_ERROR(hr);
        pControl2->GetProcessId(&pProcessId);
        if (pProcessId == pid) {
            std::cout << "Attached to PID: " + std::to_string(pProcessId) << std::endl;
            pControl2->QueryInterface(IID_ISimpleAudioVolume, (void**)&psVolume);
            EXIT_ON_ERROR(hr);
            hr = psVolume->GetMute(&mute);
            EXIT_ON_ERROR(hr);
            if (mute) hr = psVolume->SetMute(FALSE, NULL);
            else hr = psVolume->SetMute(TRUE, NULL);
            EXIT_ON_ERROR(hr);
        }
    }

    // Cleanup
exit:
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    std::cout << errMsg << std::endl;
    PropVariantClear(&varName);
    SAFE_RELEASE(pSessions);
    SAFE_RELEASE(pManager2);
    SAFE_RELEASE(pManager);
    SAFE_RELEASE(pProps);
    SAFE_RELEASE(pEndpoint);
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pCollection);
    SAFE_RELEASE(psVolume);
    SAFE_RELEASE(pControl);
    SAFE_RELEASE(pControl2);
    return;
}


int main() {
    std::cout << "Mute Focused Application" << std::endl;
    // Register global hotkeys
    if (RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x70)) std::cout << (("Mute / Unmute: F1")) << std::endl; // 0x70 = F1	
    if (RegisterHotKey(NULL, 2, MOD_NOREPEAT, 0x72)) std::cout << (("Exit: F3")) << std::endl; // 0x72 = F3
        while (GetMessage(&msg, NULL, 0, 0) != 0) {
            if (msg.message == WM_HOTKEY) {
                if (msg.wParam == 1) {
                    // Open handle to focused window
                    hwnd = GetForegroundWindow();
                    // Pull pID from handle
                    GetWindowThreadProcessId(hwnd, &pid);
                    // Pull window name from handle
                    GetWindowText(hwnd, title, sizeof(title));
                    s_title = title;
                    std::cout << "Window: " + s_title << std::endl;
                    muteProcess(pid);
                }
                if (msg.wParam == 2) {
                    std::cout << "a" << std::endl;
                    break; 
                }
            }
        }
    }
