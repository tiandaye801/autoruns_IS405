#pragma once
#include <string>
#include <string.h>
#include <tchar.h>
#include <windows.h>
using namespace std;

char* TCHAR2char(const TCHAR* STR)
{
    int size = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, FALSE);
    char* str = new char[sizeof(char) * size];
    WideCharToMultiByte(CP_ACP, 0, STR, -1, str, size, NULL, FALSE);
    return str;
}

void transbyte(string key, DWORD cchdata, BYTE achdata[])
{
    for (unsigned long j = 0; j < cchdata; j = j + 2)
    {
        key = key + (char)achdata[j];
    }
}

string wchar2string(wchar_t* pWCStrKey)
{

    int pSize = WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), NULL, 0, NULL, NULL);
    char* pCStrKey = new char[pSize + 1];
    WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), pCStrKey, pSize, NULL, NULL);
    pCStrKey[pSize] = '\0';
    string pKey = pCStrKey;
    delete[] pCStrKey;
    return pKey;
}
 
wstring string2wstring(string str)
{
    wstring result; 
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR* buffer = new TCHAR[len + 1];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0';           
    result.append(buffer);
    delete[] buffer;
    return result;
}

string wstring2string(const wstring& wstr)
{
    string str;
    int nLen = (int)wstr.length();
    str.resize(nLen, ' ');
    int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);
    if (nResult == 0)
    {
        return "";
    }
    return str;
}

string format_image_path(string image_path)
{
    int n;
    if ((n = image_path.find(" -")) != string::npos)   
        image_path = image_path.substr(0, n);
    if ((n = image_path.find(" /")) != string::npos)  
        image_path = image_path.substr(0, n);
    if ((n = image_path.find("\"")) != string::npos) 
    {    
        if (n == 0)
        {
            image_path = image_path.substr(1);
            if ((n = image_path.find("\"")) != string::npos)
                image_path = image_path.substr(0, n);
        }
    }
    if ((n = image_path.find("system32")) != string::npos)   
        image_path = image_path.substr(n).insert(0, "C:\\Windows\\");
    if ((n = image_path.find("System32")) != string::npos)
        image_path = image_path.substr(n).insert(0, "C:\\Windows\\");
    if ((n = image_path.find("SysWOW64\\")) != string::npos)
        image_path = image_path.substr(n).insert(0, "C:\\Windows\\");
    if ((n = image_path.find("\\??\\")) != string::npos)
        image_path = image_path.substr(4);
    if ((n = image_path.find("%ProgramFiles%")) != string::npos)
        image_path = image_path.substr(14).insert(0, "C:\\Program Files");
    if ((n = image_path.find("%PROGRAMFILES%")) != string::npos)
        image_path = image_path.substr(14).insert(0, "C:\\Program Files");
    if ((n = image_path.find("%ProgramData%")) != string::npos)
        image_path = image_path.substr(13).insert(0, "C:\\ProgramData");
    if ((n = image_path.find("%SystemRoot%")) != string::npos)
        image_path = image_path.substr(12).insert(0, "C:\\Windows");
    if ((n = image_path.find("\\")) == string::npos)
        image_path = image_path.insert(0, "C:\\Windows\\System32\\");
    return image_path;
}