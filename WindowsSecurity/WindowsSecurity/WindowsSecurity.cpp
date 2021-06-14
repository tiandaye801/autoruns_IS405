#include "FileDescription.cpp"
#include "TransStr.cpp"
#include "logon.cpp"
#include "services.cpp"
#include "tasks.cpp"
#include <iostream>
#include <vector>
#include <string>
void checklogon_lists()
{
    HKEY hTestKey;
    DWORD dwAccess = KEY_READ | KEY_WOW64_64KEY;

    string root = "HKLM";
    vector<string> reg_path;
    reg_path.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    reg_path.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
    reg_path.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    reg_path.push_back("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
    reg_path.push_back("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
    int encounter = 0;

    for (int flag = 0; flag < 5; flag++)
    {

        wstring str = string2wstring(reg_path[flag]);

        if (root == "HKLM")
        {
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, str.c_str(), 0, dwAccess, &hTestKey) == ERROR_SUCCESS)
                QueryKey(hTestKey);
        }
        else {
            if (RegOpenKeyEx(HKEY_CURRENT_USER, str.c_str(), 0, dwAccess, &hTestKey) == ERROR_SUCCESS)
                QueryKey(hTestKey);
        }
        RegCloseKey(hTestKey);
        if (flag == 4)
        {
            encounter++;
            root = "HKCU";
            flag = 0;
            if (encounter == 2) flag = 9;
        }
    }
}

void checkServices_and_Drivers()
{
    HKEY hTestKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\Services"), 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
    {
        QueryGroup(hTestKey);
    }
    RegCloseKey(hTestKey);
}

void checktasks()
{
    //  ------------------------------------------------------
//  Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);

    //  ------------------------------------------------------
    //  Create an instance of the Task Service. 
    ITaskService* pService = NULL;
    hr = CoCreateInstance(CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&pService);
    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.
    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);

    //walkFoldersFormat(pRootFolder, hr);
    //getTasksFormat(pRootFolder, hr);

    walkFolders(pRootFolder, hr);
    getTasks(pRootFolder, hr);
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();
    //system("pause");
}

int main()
{
    char cmdchar;
    cmdchar = 'r';
    while (cmdchar != 'q') 
    {
        do
        {
            cmdchar = getchar();
        } while (cmdchar == '\n' || cmdchar == '\t' || cmdchar == ' ');  

        switch (cmdchar)  
        {
        case 'l':
            checklogon_lists();
            break; 
        case 's':
            checkServices_and_Drivers();
            break;
        case 't':
            checktasks();
            break; 
        default:
            break;
        }
    }
    return 1;

}
