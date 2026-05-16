// cmd line 
// g++ firewall_win_api.cpp -lole32 -loleaut32

// firewall_win_api.cpp
#include <windows.h>
#include <netfw.h>
#include <iostream>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using namespace std;

// =============================
HRESULT AddRule(int port, bool inbound) {
    HRESULT hr;

    INetFwPolicy2* pPolicy = NULL;
    INetFwRules* pRules = NULL;
    INetFwRule* pRule = NULL;

    CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        (void**)&pPolicy);

    if (FAILED(hr)) return hr;

    hr = pPolicy->get_Rules(&pRules);
    if (FAILED(hr)) return hr;

    hr = CoCreateInstance(
        __uuidof(NetFwRule),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwRule),
        (void**)&pRule);

    if (FAILED(hr)) return hr;

    pRule->put_Name(L"Block Port Rule");
    pRule->put_Protocol(NET_FW_IP_PROTOCOL_TCP);
    pRule->put_LocalPorts(_bstr_t(to_string(port).c_str()));
    pRule->put_Direction(inbound ? NET_FW_RULE_DIR_IN : NET_FW_RULE_DIR_OUT);
    pRule->put_Action(NET_FW_ACTION_BLOCK);
    pRule->put_Enabled(VARIANT_TRUE);

    hr = pRules->Add(pRule);

    pRule->Release();
    pRules->Release();
    pPolicy->Release();

    CoUninitialize();

    return hr;
}

// =============================
int main() {
    cout << "Blocking port 80 inbound...\n";
    AddRule(80, true);

    cout << "Blocking port 22 outbound...\n";
    AddRule(22, false);

    return 0;
}

