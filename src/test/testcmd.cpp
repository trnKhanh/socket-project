#include <iostream>
// #include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>

int main() {
    IShellItem *pApps = nullptr;
    SHCreateItemFromParsingName(L"shell:appsFolder", nullptr, IID_PPV_ARGS(&pApps)); // get apps folder
    // if (FAILED(hr));

    IEnumShellItems *ItemList = nullptr;
    HRESULT hr = pApps->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&ItemList)); // start the file scanning process
    // if (FAILED(hr)) {
    //     pApps->Release();
    // }

    IShellItem *Item = nullptr;
    while (ItemList->Next(1, &Item, nullptr) == S_OK) // and the list loop
    {
        LPWSTR pDisplayName = nullptr;
        HRESULT hr = Item->GetDisplayName(SIGDN_NORMALDISPLAY, &pDisplayName); // get the name
        if (SUCCEEDED(hr)) {
            wprintf(L"found %s\n", pDisplayName);
            CoTaskMemFree(pDisplayName);
        }
        Item->Release();
    }

    ItemList->Release();
    pApps->Release();
    return 0;
}