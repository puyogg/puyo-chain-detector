#include <map>
#include <string>

#ifdef _WIN32

#include <atlbase.h>
#include <dshow.h>

bool GetCaptureDevices(std::map<int, std::string>& map)
{
    // Create device enumerator.
    const GUID guidSystemDeviceEnum = {
        0x62BE5D10, 0x60EB, 0x11d0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86
    };
    CComPtr<ICreateDevEnum> deviceEnumerator;
    HRESULT hr = CoCreateInstance(guidSystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
    if (FAILED(hr))
    {
        return false;
    }

    CComPtr<IEnumMoniker> classEnumerator;
    const GUID guidVideoInputDeviceCategory = {
        0x860BB310, 0x5D01, 0x11d0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86
    };
    hr = deviceEnumerator->CreateClassEnumerator(guidVideoInputDeviceCategory, &classEnumerator, 0);
    if (FAILED(hr))
    {
        return false;
    }

    // No devices.
    if (hr == S_FALSE)
    {
        return true;
    }

    for (int i = 0; ; i++)
    {
        CComPtr<IMoniker> device;
        hr = classEnumerator->Next(1, &device, nullptr);
        if (hr == S_FALSE)
        {
            return true;
        }

        CComPtr<IPropertyBag> properties;
        hr = device->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&properties));
        if (FAILED(hr))
        {
            continue;
        }

        VARIANT desc;
        VariantInit(&desc);
        properties->Read(L"FriendlyName", &desc, nullptr);
        if (FAILED(hr))
        {
            VariantClear(&desc);
            properties->Read(L"Description", &desc, nullptr);
            if (FAILED(hr))
            {
                VariantClear(&desc);
                continue;
            }
        }

        auto* wStr = desc.bstrVal;
        int wLen = SysStringLen(wStr);
        int mbLen = WideCharToMultiByte(
            CP_ACP, 0, wStr, wLen, nullptr,
            0, nullptr, nullptr);
        std::string mbStr(mbLen, 0);
        WideCharToMultiByte(
            CP_ACP, 0, wStr, wLen,
            mbStr.data(), mbLen,
            nullptr, nullptr);
        map[i] = mbStr;
    }
}

#else

bool GetCaptureDevices(std::map<int, std::string>& map)
{
    // Only implemented for Windows.
    return false;
}

#endif
