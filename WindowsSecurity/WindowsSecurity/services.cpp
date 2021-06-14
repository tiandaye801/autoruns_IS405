// QueryKey - Enumerates the subkeys of key and its associated values.
//     hKey - Key whose subkeys and values are to be enumerated.

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <typeinfo>
#include "FileDescription.cpp"
#include "TransStr.cpp"
using namespace std;

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

void QueryKey(HKEY hKey, string name, string path);
void QueryGroup(HKEY hKey);
string GetSvchost(HKEY hKey);

void QueryGroup(HKEY hKey)
{
    TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys = 0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode;

    string path = "System\\CurrentControlSet\\Services\\";

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 

    // Enumerate the subkeys, until RegEnumKeyEx fails.

    if (cSubKeys)
    {
        for (i = 0; i < cSubKeys; i++)
        {
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                achKey,
                &cbName,
                NULL,
                NULL,
                NULL,
                &ftLastWriteTime);
            if (retCode == ERROR_SUCCESS)
            {
                HKEY hTestKey;
                string tmp = wchar2string(achKey);
                tmp = path + tmp;
                wstring str = string2wstring(tmp); 
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, str.c_str(), 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
                    QueryKey(hTestKey, wchar2string(achKey), tmp);
                RegCloseKey(hTestKey);
            }
        }
    }
}

void QueryKey(HKEY hKey, string name, string path)
{
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys = 0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode;

    TCHAR  achValue[MAX_VALUE_NAME];
    BYTE  achData[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cchData = MAX_VALUE_NAME;
    DWORD Type;

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 

    if (cValues)
    {
        //printf("\nNumber of values: %d\n", cValues);
        bool autorun_flag = false;  
        string type(""), start(""), image_path("");

        for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            cchValue = MAX_VALUE_NAME;
            cchData = MAX_VALUE_NAME;
            achValue[0] = '\0';
            achData[0] = '\0';
            retCode = RegEnumValue(hKey, i,
                achValue,
                &cchValue,
                NULL,
                &Type,
                achData,
                &cchData);

            if (retCode == ERROR_SUCCESS)
            {
                string key = wstring2string(achValue);
                if (key == "")
                    continue;

                if (key == "Start") 
                {
                    transbyte(start,cchData,achData);
                    if (start[0] < 3)
                        autorun_flag = true;
                }
                else if (key == "ImagePath") 
                {
                    transbyte(image_path,cchData,achData);
                    image_path = format_image_path(image_path);
                }
            }
        }
        if (autorun_flag) {
            if (image_path == "")
                return;

            int n;
            if ((n = image_path.find("svchost")) != string::npos) {
                HKEY hTestKey;
                path = path + "\\Parameters";
                wstring str = string2wstring(path); 
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, str.c_str(), 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
                    image_path = GetSvchost(hTestKey);
                else
                    image_path = "";
                RegCloseKey(hTestKey);
            }

            TCHAR* fileDescription = new TCHAR[1024];
            wstring str = string2wstring(image_path);
            BOOL bRet = GetFileVersionString(str.c_str(), _T("FileDescription"), fileDescription, 1024);
            string file_description = TCHAR2char(fileDescription);
            if ((n = file_description.find("?")) != string::npos)
                file_description = "";

            if (image_path != "") {
                string split_sign(" ");
                string info = name + split_sign + file_description + split_sign + image_path;
                printf("%s\n", info.c_str());
            }
        }
        else {
            return;
        }
    }
}

string GetSvchost(HKEY hKey)
{
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys = 0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode;

    TCHAR  achValue[MAX_VALUE_NAME];
    BYTE  achData[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cchData = MAX_VALUE_NAME;
    DWORD Type;

    string image_path("");

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time  

    if (cValues)
    {

        for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            cchValue = MAX_VALUE_NAME;
            cchData = MAX_VALUE_NAME;
            achValue[0] = '\0';
            achData[0] = '\0';
            retCode = RegEnumValue(hKey, i,
                achValue,
                &cchValue,
                NULL,
                &Type,
                achData,
                &cchData);

            if (retCode == ERROR_SUCCESS)
            {
                string name = wstring2string(achValue);
                if (name == "ServiceDll") {
                    transbyte(image_path,cchData,achData);
                    image_path = format_image_path(image_path);
                }
            }
        }
    }

    return image_path;
}

int __cdecl _tmain()
{
    HKEY hTestKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\Services"), 0, KEY_READ, &hTestKey) == ERROR_SUCCESS) {
        QueryGroup(hTestKey);
    }

    RegCloseKey(hTestKey);
    return 1;
}