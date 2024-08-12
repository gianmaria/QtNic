#ifndef NIC_H
#define NIC_H

#include <assert.h>
#include <cstdint>
#include <print>
//#include <iostream>
//#include <map>
//#include <set>
#include <string_view>
#include <string>
#include <vector>
#include <memory>
//#include <cwchar>


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using std::string;
using std::string_view;
using std::vector;
using std::shared_ptr;

using str = std::string; // NOTE: all std::string are utf-8 encoded
using str_cref = std::string const&;
using wstr = std::wstring;
using wstr_cref = std::wstring const&;

template<typename T>
using vec = vector<T>;

template<typename T>
using shared = shared_ptr<T>;

using namespace std::string_literals;
using namespace std::string_view_literals;

typedef unsigned long DWORD;


struct Interface;

void dump_nic_info(const vec<shared<Interface>> &interfaces);

void update_nic_metric(const vec<Interface>& interfaces,
                       str_cref filename);

vec<shared<Interface>> collect_nic_info();

// NOTE: all this mumbo jumbo to hide windows.h from qt....
str_cref get_name(const shared<Interface>& nic);

bool is_running_as_administrator();

#endif // NIC_H
