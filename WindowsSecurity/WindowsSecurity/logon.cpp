// QueryKey - Enumerates the subkeys of key and its associated values.
//     hKey - Key whose subkeys and values are to be enumerated.

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <typeinfo>
#include "FileDescription.cpp"
#include "TransStr.cpp"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

void QueryKey(HKEY hKey)
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
    DWORD type;

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

    // Enumerate the key values. 
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
                &type,
                achData,
                &cchData);

            if (retCode == ERROR_SUCCESS)
            {
                unsigned long j;
                string name = wstring2string(achValue);
                string image_path = "";
                for (j = 0; j < cchData; j = j + 2) 
                {
                    image_path = image_path + (char)achData[j];
                }
                image_path = format_image_path(image_path); 
                TCHAR* fileDescription = new TCHAR[1024];
                wstring str = string2wstring(image_path);
                BOOL bRet = GetFileVersionString(str.c_str(), _T("FileDescription"), fileDescription, 1024);
                string file_description = TCHAR2char(fileDescription);
                int n;
                if ((n = file_description.find("?")) != string::npos)
                    file_description = "";
                if (name == "" && file_description != "")
                    name = file_description;
                name.append(" ");
                name.append(file_description);
                name.append(" ");
                name.append(image_path);
                printf("%s\n", name.c_str());

            }
        }
    }
}

