#include "nic.h"

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h> // for inet_ntop function
#include <iphlpapi.h>
#include <shellapi.h>

#include <sstream>

#include "utf8.h"


struct Interface
{
    // NOTE: all std::string are utf-8 encoded
    str name;
    str description;
    str ip;
    u32 subnet {0};
    str gateway;
    str dns;
    str dns_suff;
    u32 metric {0};
    bool automatic_metric {false};
    bool connected {false};
    IF_LUID luid {};
    IF_INDEX index {};
};

struct Heap_Deleter
{
    void operator()(void* mem) const;
};

void Heap_Deleter::operator()(void *mem) const
{
    if (mem)
        HeapFree(GetProcessHeap(), NULL, mem);
}

struct WSA_Startup
{
    WSA_Startup(WORD version);
    ~WSA_Startup();

    WSADATA wsa_data {};
    int res {};
};

WSA_Startup::WSA_Startup(WORD version)
{
    res = WSAStartup(version, &wsa_data);
}

WSA_Startup::~WSA_Startup()
{
    // we ignore return code here
    WSACleanup();
}


// forward declaration of private stuff

str to_UTF8(wstr_cref wide_str);
wstr to_wide(str_cref utf8_str);
str last_error_as_string(DWORD last_error);
void update_nic_metric_for_luid(str_cref interface_name,
                                IF_LUID luid,
                                ULONG new_metric,
                                bool automatic_metric);
vec<str> split_string_by_newline(str_cref text);


// public stuff

vec<shared<Interface>> collect_nic_info()
{
    ULONG buffer_size = 0;
    ULONG adapters_flags =
        GAA_FLAG_INCLUDE_WINS_INFO |
        GAA_FLAG_INCLUDE_PREFIX |
        GAA_FLAG_INCLUDE_GATEWAYS;

    GetAdaptersAddresses(AF_INET, adapters_flags, NULL, NULL, &buffer_size);

    auto* mem_ = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buffer_size);
    std::unique_ptr<void, Heap_Deleter> mem(mem_);

    if (not mem)
    {
        throw std::format("[ERROR] cannot allocate memory!");
    }

    DWORD result = GetAdaptersAddresses(
        AF_INET,
        adapters_flags,
        NULL,
        (IP_ADAPTER_ADDRESSES*)mem.get(), &buffer_size);

    if (result != NO_ERROR)
    {
        throw std::format("[ERROR] cannot get adapters addresses: {}",
                          last_error_as_string(result));
    }

    vec<shared<Interface>> interfaces;

    IP_ADAPTER_ADDRESSES* adapter = (IP_ADAPTER_ADDRESSES*)mem.get();

    while (adapter != nullptr)
    {
        Interface itf {};

        itf.name = to_UTF8(adapter->FriendlyName);
        itf.description = to_UTF8(adapter->Description);
        itf.connected = adapter->OperStatus == IfOperStatusUp;
        itf.dns_suff = to_UTF8(adapter->DnsSuffix);
        itf.metric = adapter->Ipv4Metric;
        itf.index = adapter->IfIndex;
        itf.luid = adapter->Luid;

        MIB_IPINTERFACE_ROW interface_row {};
        interface_row.Family = AF_INET;
        interface_row.InterfaceLuid = adapter->Luid;

        result = GetIpInterfaceEntry(&interface_row);

        if (result != NO_ERROR)
        {
            throw std::format("[ERROR] GetIpInterfaceEntry failed: {}",
                              last_error_as_string(result));
        }

        itf.automatic_metric = interface_row.UseAutomaticMetric;

        // get all the IPs
        for (IP_ADAPTER_UNICAST_ADDRESS_LH* unicast_addr = adapter->FirstUnicastAddress;
             unicast_addr != nullptr;
             unicast_addr = unicast_addr->Next)
        {
            sockaddr_in* sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(unicast_addr->Address.lpSockaddr);
            wchar_t ip_str[INET_ADDRSTRLEN] {};
            InetNtopW(AF_INET, &(sockaddr_ipv4->sin_addr), ip_str, INET_ADDRSTRLEN);

            itf.ip.append(to_UTF8(ip_str)).append(" ");
            itf.subnet = unicast_addr->OnLinkPrefixLength;
        }

        // get all the Gateway
        for (IP_ADAPTER_GATEWAY_ADDRESS_LH* gateway_addr = adapter->FirstGatewayAddress;
             gateway_addr != nullptr;
             gateway_addr = gateway_addr->Next)
        {
            sockaddr_in* sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(gateway_addr->Address.lpSockaddr);
            wchar_t gateway_str[INET_ADDRSTRLEN] {};
            InetNtopW(AF_INET, &sockaddr_ipv4->sin_addr, gateway_str, INET_ADDRSTRLEN);

            itf.gateway.append(to_UTF8(gateway_str)).append(" ");
        }

        // get all the DNS
        for (IP_ADAPTER_DNS_SERVER_ADDRESS_XP* dns_addr = adapter->FirstDnsServerAddress;
             dns_addr != nullptr;
             dns_addr = dns_addr->Next)
        {
            sockaddr_in* sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(dns_addr->Address.lpSockaddr);
            wchar_t dns_str[INET_ADDRSTRLEN] {};
            InetNtopW(AF_INET, &sockaddr_ipv4->sin_addr, dns_str, INET_ADDRSTRLEN);

            itf.dns.append(to_UTF8(dns_str)).append(" ");
        }

        interfaces.push_back(std::make_shared<Interface>(itf));

        adapter = adapter->Next;
    }

    return interfaces;
}

u32 update_nic_metric(const vec<shared<Interface>> &interfaces,
                       str_cref nic_list)
{
    u32 skipped = 0;

    auto lines = split_string_by_newline(nic_list);

    u32 pos = 1;
    for (str_cref line : lines)
    {
        auto* target_name = line.data();

        auto it = std::find_if(
            interfaces.begin(),
            interfaces.end(),
            [target_name](const shared<Interface>& itf)
            {
                auto* src1 = reinterpret_cast<const utf8_int8_t*>(itf->name.c_str());
                auto* src2 = reinterpret_cast<const utf8_int8_t*>(target_name);
                return utf8cmp(src1, src2) == 0;
            }
        );

        if (it == interfaces.end())
        {
            ++skipped;
            continue;
        }

        ULONG new_metric = (pos++) * 10;

        update_nic_metric_for_luid(target_name,
                                   (*it)->luid,
                                   new_metric,
                                   (*it)->automatic_metric);
    }

    return skipped;
}


bool is_running_as_administrator()
{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup {};

    BOOL success = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);

    if (not success)
    {
        FreeSid(AdministratorsGroup);
        throw std::format("[ERROR] Cannot allocate SID: {}",
                          last_error_as_string(GetLastError()));
    }

    BOOL is_member = FALSE;

    success = CheckTokenMembership(NULL, AdministratorsGroup, &is_member);

    if (not success)
    {
        FreeSid(AdministratorsGroup);
        throw std::format("[ERROR] CheckTokenMembership failed: {}",
                          last_error_as_string(GetLastError()));
    }

    FreeSid(AdministratorsGroup);

    return is_member == TRUE;
}

DWORD restart_as_admin()
{
    wchar_t szPath[MAX_PATH];

    auto res = GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));

    if (res == MAX_PATH)
    {
        return GetLastError();
    }

    SHELLEXECUTEINFO sei {0};
    sei.cbSize = sizeof(sei);
    // sei.fMask = SEE_MASK_NOASYNC;
    sei.lpVerb = L"runas";
    sei.lpFile = szPath;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;

    auto success = ShellExecuteExW(&sei);

    return (success == TRUE) ? NO_ERROR : GetLastError();
}


str_cref get_name(const shared<Interface>& nic)
{
    return nic->name;
}

str_cref get_description(const shared<Interface> &nic)
{
    return nic->description;
}

// private stuff

str to_UTF8(wstr_cref wide_str)
{
    int size = WideCharToMultiByte(
        CP_UTF8, 0, wide_str.c_str(), -1,
        nullptr, 0, nullptr, nullptr);

    if (size == 0)
        return "";

    auto utf8_str = std::string(size, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0, wide_str.c_str(), -1,
        &utf8_str[0], size, nullptr, nullptr);

    return utf8_str;
}

wstr to_wide(str_cref utf8_str)
{
    int size = MultiByteToWideChar(
        CP_UTF8, 0, utf8_str.data(),
        -1, nullptr, 0);

    if (size == 0)
        return L"";

    wstr wide_str(size, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, utf8_str.data(),
        -1, &wide_str[0], size);

    return wide_str;
}

str last_error_as_string(DWORD last_error)
{
    auto constexpr buffer_count = 1024;
    WCHAR buffer[buffer_count] {};

    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL,
        last_error,
        0,
        (wchar_t*)&buffer,
        buffer_count,
        NULL);

    return to_UTF8(wstr(buffer, size));
}

void update_nic_metric_for_luid(str_cref interface_name,
                                IF_LUID luid,
                                ULONG new_metric,
                                bool automatic_metric)
{
    // Retrieve the IP interface table
    MIB_IPINTERFACE_ROW row {};
    row.Family = AF_INET; // IPv4
    row.InterfaceLuid = luid;
    row.SitePrefixLength = 32; // For an IPv4 address, any value greater than 32 is an illegal value.

    DWORD result = GetIpInterfaceEntry(&row);

    if (result != NO_ERROR)
    {
        throw std::format("[ERROR] cannot get interface entry: {}",
                          last_error_as_string(result));
    }

    if (automatic_metric)
    {
        row.UseAutomaticMetric = 0;
    }

    // Change the metric
    row.Metric = new_metric; // Set the desired metric

    // Set the modified IP interface entry
    result = SetIpInterfaceEntry(&row);

    if (result != NO_ERROR)
    {
        throw std::format("[ERROR] Cannot update metric for interface '{}': {}",
                          interface_name, last_error_as_string(result));
    }

}

vec<str> split_string_by_newline(str_cref text)
{
    vec<str> lines;
    std::istringstream stream(text);
    str line;

    while (std::getline(stream, line))
    {
        lines.push_back(line);
    }

    return lines;
}

/* void run_as_administrator(wchar_t* argv[])
{
    std::wstringstream ss;

    for (wchar_t** args = argv + 1;
         *args != nullptr;
         ++args)
    {
        ss << *args << " ";
    }

    auto s_argv = ss.str();

    // Prompt the user with a UAC dialog for elevation
    SHELLEXECUTEINFO shell_execute_info {};
    shell_execute_info.cbSize = sizeof(SHELLEXECUTEINFO);
    shell_execute_info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_UNICODE | SEE_MASK_NO_CONSOLE;
    shell_execute_info.lpVerb = L"runas"; // Request elevation
    shell_execute_info.lpFile = argv[0]; // Path to your application executable
    shell_execute_info.lpParameters = s_argv.c_str(); // Optional parameters for your application
    shell_execute_info.nShow = SW_SHOWNORMAL;

    if (BOOL res = ShellExecuteExW(&shell_execute_info);
        not res)
    {
        throw std::format("[ERROR] cannot start app as Administrator: {}",
                          last_error_as_string(GetLastError()));
    }

    if (DWORD res = WaitForSingleObject(shell_execute_info.hProcess, INFINITE);
        res == WAIT_FAILED)
    {
        throw std::format("[ERROR] WaitForSingleObject failed: {}",
                          last_error_as_string(GetLastError()));
    }

    if (BOOL res = CloseHandle(shell_execute_info.hProcess);
        res == 0)
    {
        throw std::format("[ERROR] CloseHandle failed: {}",
                          last_error_as_string(GetLastError()));
    }
}
*/
