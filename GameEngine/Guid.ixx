module;
#include <stdio.h>
#include <windows.h>
export module Engine.Guid;
import std;
import std.compat;

export struct Guid
{
    Guid() { CoCreateGuid(&impl); }

    explicit Guid(const std::string& str);

    std::string toString() const;

    GUID impl;
    friend bool operator==(const Guid& a, const Guid& b) { return a.impl == b.impl; }
};

Guid::Guid(const std::string& str)
{
    GUID guid;
    unsigned short data4[8];
    if (sscanf_s(str.c_str(),
                 "%8x-%4hx-%4hx-%2hx%2hx-%2hx%2hx%2hx%2hx%2hx%2hx",
                 &guid.Data1, &guid.Data2, &guid.Data3,
                 &data4[0], &data4[1], &data4[2], &data4[3],
                 &data4[4], &data4[5], &data4[6], &data4[7]) != 11)
    {
        throw std::invalid_argument("Invalid GUID string format");
    }
    for (int i = 0; i < 8; ++i)
    {
        guid.Data4[i] = static_cast<uint8_t>(data4[i]);
    }
    impl = guid;
}

std::string Guid::toString() const
{
    char guidStr[37];
    snprintf(guidStr, sizeof(guidStr),
                  "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                  impl.Data1, impl.Data2, impl.Data3,
                  impl.Data4[0], impl.Data4[1], impl.Data4[2], impl.Data4[3],
                  impl.Data4[4], impl.Data4[5], impl.Data4[6], impl.Data4[7]);
    return std::string(guidStr);
}

template <>
struct std::hash<Guid>
{
    size_t operator()(const Guid& guid) const noexcept
    {
        RPC_STATUS status = RPC_S_OK;
        return ::UuidHash(&const_cast<GUID&>(guid.impl), &status);
    }
};
