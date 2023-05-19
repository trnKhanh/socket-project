// #include <iostream>
// #include <Windows.h>
// #include <set>
// #include <string>


// std::multiset <std::string> s;

// void EnumerateInstalledApplications(HKEY key, const char* path)
// {
//     int cnt = 0;
//     HKEY hUninstallKey;
//     if (RegOpenKeyEx(key, path, 0, KEY_READ, &hUninstallKey) == ERROR_SUCCESS)
//     {
//         char subKeyName[256];
//         DWORD subKeyNameSize = sizeof(subKeyName);

//         for (DWORD index = 0; RegEnumKeyEx(hUninstallKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS; ++index)
//         {
//             HKEY hAppKey;
//             if (RegOpenKeyEx(hUninstallKey, subKeyName, 0, KEY_READ, &hAppKey) == ERROR_SUCCESS)
//             {
//                 char displayName[256];
//                 DWORD tmp;
//                 DWORD tmpsize;
//                 DWORD displayNameSize = sizeof(displayName);
//                 if (RegQueryValueEx(hAppKey, "SystemComponent", NULL, NULL, reinterpret_cast<LPBYTE>(&tmp), &tmpsize) == ERROR_SUCCESS){
//                     // std::cout << tmp << '\n';
//                     if(tmp == (DWORD)1){
                        
//                          RegCloseKey(hAppKey);
//                         subKeyNameSize = sizeof(subKeyName);
//                         continue;
//                     }
//                 }
//                 if (RegQueryValueEx(hAppKey, "DisplayName", NULL, NULL, reinterpret_cast<LPBYTE>(displayName), &displayNameSize) == ERROR_SUCCESS)
//                 {
//                     std::cout << cnt++ << " " << hUninstallKey << ' ';
//                     s.insert(displayName);
//                     // std::cout << "Display Name: " << displayName << std::endl;
//                 }

//                 RegCloseKey(hAppKey);
//             }

//             subKeyNameSize = sizeof(subKeyName);
//         }

//         RegCloseKey(hUninstallKey);
//     }
// }

// int main()
// {
//     EnumerateInstalledApplications(HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
//     EnumerateInstalledApplications(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
//     EnumerateInstalledApplications(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
//     EnumerateInstalledApplications(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\Repository\\Packages");

//     // EnumerateInstalledApplications(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths");
//     // std::cout << s.size() << '\n';
//     // for (auto c: s)
//     //     std::cout << c << std::endl;
//     return 0;
// }

// #include <ShObjIdl.h>
// #include <Windows.h>
// #include <ShlObj.h>
// #include <stdio.h>
// #include <iostream>
// #include <propvarutil.h>
// #include <string>
// #include <fcntl.h>
// #include <ShlGuid.h>

// #pragma comment(lib, "propsys.lib")
// #pragma comment(lib, "shell32.lib")
// // const GUID BHID_EnumItems = { 0x94f60519, 0x2850, 0x4924, { 0xaa, 0x5a, 0xd1, 0x5e, 0x84, 0x86, 0x80, 0x39} };

// int main()
// {
//     _setmode(_fileno(stdout), _O_U16TEXT);
//     CoInitialize(NULL);
//     IShellItem* folder;
//     if (SUCCEEDED(SHCreateItemFromParsingName(L"shell:appsFolder", nullptr, IID_PPV_ARGS(&folder))))
//     {
//         IEnumShellItems* enumItems;
//         if (SUCCEEDED(folder->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&enumItems))))
//         {
//             IShellItem* items;
//             while (enumItems->Next(1, &items, nullptr) == S_OK)
//             {
//                 IShellItem* item = items;
//                 LPWSTR name = NULL;
//                 if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)))
//                 {
//                     // wprintf(L"%s\n", name);
//                     // int cnt = 0;
//                     // while (cnt < 10) wprintf(L"%s", name + cnt++);
//                     // printf("%d", *(name + 1));
//                     std::wcout << name << std::endl;
//                     // std::wstring buffer(name);
//                     // std::wcout << buffer << "\n";

//                     CoTaskMemFree(name);
//                 }
                

//                 // dump all properties
//                 //CComPtr<IPropertyStore> store;
//                 //if (SUCCEEDED(item->BindToHandler(NULL, BHID_PropertyStore, IID_PPV_ARGS(&store))))
//                 //{
//                 //    DWORD count = 0;
//                 //    store->GetCount(&count);
//                 //    for (DWORD i = 0; i < count; i++) {
//                 //        PROPERTYKEY pk;
//                 //        if (SUCCEEDED(store->GetAt(i, &pk)))
//                 //        {
//                 //            CComHeapPtr<wchar_t> pkName;
//                 //            PSGetNameFromPropertyKey(pk, &pkName); // needs propsys.lib

//                 //            PROPVARIANT pv;
//                 //            PropVariantInit(&pv);
//                 //            if (SUCCEEDED(store->GetValue(pk, &pv)))
//                 //            {
//                 //                CComHeapPtr<wchar_t> pvs;
//                 //                pvs.Allocate(512);
//                 //                PropVariantToString(pv, pvs, 512); // needs propvarutil.h and propsys.lib
//                 //                PropVariantClear(&pv);
//                 //                wprintf(L" %s=%s\n", pkName, pvs);
//                 //            }
//                 //            else
//                 //            {
//                 //                wprintf(L" %s=???\n", pkName);
//                 //            }
//                 //        }
//                 //    }
//                 //}
//             }
//         }
//     }
//     CoUninitialize();
// }

#include <Windows.h>
#include <Windows.Management.Deployment.h>

#pragma comment(lib, "windowsapp.lib")

int main()
{
    // Your code using the Windows.Management.Deployment APIs
    // ...
    return 0;
}