#ifndef RAPIDJSON_UTILS_H_
#define RAPIDJSON_UTILS_H_

#include "document.h"
#include <string>
#include <cstdlib>

#if !(defined(_WIN32) || defined(_WIN64))

#ifndef _stricmp
#define _stricmp strcasecmp
#endif

#ifndef _atoi64
#define _atoi64(val) strtoll(val, NULL, 10)
#endif

#endif

RAPIDJSON_NAMESPACE_BEGIN

inline std::string ToString(const Value& value)
{
    if (value.IsString())
        return value.GetString();
    else if (value.IsFalse())
        return "false";
    else if (value.IsTrue())
        return "true";
    else if (value.IsInt())
        return std::to_string(value.GetInt());
    else if (value.IsUint())
        return std::to_string(value.GetUint());
    else if (value.IsInt64())
        return std::to_string(value.GetInt64());
    else if (value.IsUint64())
        return std::to_string(value.GetUint64());
    else if (value.IsDouble())
    {
        std::string str = std::to_string(value.GetDouble());
        str.erase(str.find_last_not_of('0') + 1, std::string::npos); // remove trailing zeros
        str.erase(str.find_last_not_of('.') + 1, std::string::npos); // remove trailing dot
        return str;
    }
    else if (value.IsNull())
        return "";
    else
        return value.GetString(); // trigger assert
}

inline bool ToBool(const Value& value)
{
    if (value.IsBool())
        return value.GetBool();
    else if (value.IsInt())
        return value.GetInt() != 0;
    else if (value.IsString())
        return _stricmp(value.GetString(), "false") != 0;
    else if (value.IsUint())
        return value.GetUint() != 0;
    else if (value.IsInt64())
        return value.GetInt64() != 0;
    else if (value.IsUint64())
        return value.GetUint64() != 0;
    else if (value.IsDouble())
        return value.GetDouble() != 0.0;
    else if (value.IsNull())
        return false;
    else
        return value.GetBool(); // trigger assert
}

inline int ToInt(const Value& value)
{
    if (value.IsInt())
        return value.GetInt();
    else if (value.IsString())
        return std::atoi(value.GetString());
    else if (value.IsUint())
        return static_cast<int>(value.GetUint());
    else if (value.IsInt64())
        return static_cast<int>(value.GetInt64());
    else if (value.IsUint64())
        return static_cast<int>(value.GetUint64());
    else if (value.IsDouble())
        return static_cast<int>(value.GetDouble());
    else if (value.IsTrue())
        return 1;
    else if (value.IsFalse())
        return 0;
    else if (value.IsNull())
        return 0;
    else
        return value.GetInt(); // trigger assert
}

inline unsigned int ToUint(const Value& value)
{
    if (value.IsUint())
        return value.GetUint();
    else if (value.IsString())
        return static_cast<unsigned int>(std::atoll(value.GetString()));
    else if (value.IsInt())
        return static_cast<unsigned int>(value.GetInt());
    else if (value.IsInt64())
        return static_cast<unsigned int>(value.GetInt64());
    else if (value.IsUint64())
        return static_cast<unsigned int>(value.GetUint64());
    else if (value.IsDouble())
        return static_cast<unsigned int>(value.GetDouble());
    else if (value.IsTrue())
        return 1;
    else if (value.IsFalse())
        return 0;
    else if (value.IsNull())
        return 0;
    else
        return value.GetUint(); // trigger assert
}

inline int64_t ToInt64(const Value& value)
{
    if (value.IsInt64())
        return value.GetInt64();
    else if (value.IsString())
        return _atoi64(value.GetString());
    else if (value.IsInt())
        return static_cast<int64_t>(value.GetInt());
    else if (value.IsUint())
        return static_cast<int64_t>(value.GetUint());
    else if (value.IsUint64())
        return static_cast<int64_t>(value.GetUint64());
    else if (value.IsDouble())
        return static_cast<int64_t>(value.GetDouble());
    else if (value.IsTrue())
        return 1;
    else if (value.IsFalse())
        return 0;
    else if (value.IsNull())
        return 0;
    else
        return value.GetInt64(); // trigger assert
}

inline uint64_t ToUint64(const Value& value)
{
    if (value.IsUint64())
        return value.GetUint64();
    else if (value.IsString())
        return static_cast<uint64_t>(_atoi64(value.GetString()));
    else if (value.IsInt())
        return static_cast<uint64_t>(value.GetInt());
    else if (value.IsUint())
        return static_cast<uint64_t>(value.GetUint());
    else if (value.IsInt64())
        return static_cast<uint64_t>(value.GetInt64());
    else if (value.IsDouble())
        return static_cast<uint64_t>(value.GetDouble());
    else if (value.IsTrue())
        return 1;
    else if (value.IsFalse())
        return 0;
    else if (value.IsNull())
        return 0;
    else
        return value.GetUint64(); // trigger assert
}

inline double ToDouble(const Value& value)
{
    if (value.IsDouble())
        return value.GetDouble();
    else if (value.IsString())
        return static_cast<double>(std::atof(value.GetString()));
    else if (value.IsInt())
        return static_cast<double>(value.GetInt());
    else if (value.IsUint())
        return static_cast<double>(value.GetUint());
    else if (value.IsInt64())
        return static_cast<double>(value.GetInt64());
    else if (value.IsUint64())
        return static_cast<double>(value.GetUint64());
    else if (value.IsTrue())
        return 1.0;
    else if (value.IsFalse())
        return 0.0;
    else if (value.IsNull())
        return 0;
    else
        return value.GetDouble(); // trigger assert
}

///////////////////////////////////////////////////////////////////////////////

inline bool GetStringMember(rapidjson::Value& obj, const char* name, std::string& val)
{
    if (!obj.HasMember(name))
        return false;

    const auto& m = obj[name];
    if (!m.IsString())
        return false;

    val = m.GetString();
    return true;
}

inline bool GetOptionalStringMember(rapidjson::Value& obj, const char* name, std::string& val, const std::string& defaultVal = "")
{
    if (!obj.HasMember(name))
    {
        val = defaultVal;
        return true;
    }

    const auto& m = obj[name];
    if (!m.IsString())
        return false;

    val = m.GetString();
    return true;
}

inline bool GetBoolMember(rapidjson::Value& obj, const char* name, bool& val)
{
    if (!obj.HasMember(name))
        return false;

    const auto& m = obj[name];
    if (!m.IsBool())
        return false;

    val = m.GetBool();
    return true;
}

inline bool GetOptionalBoolMember(rapidjson::Value& obj, const char* name, bool& val, bool defaultVal = false)
{
    if (!obj.HasMember(name))
    {
        val = defaultVal;
        return true;
    }

    const auto& m = obj[name];
    if (!m.IsBool())
        return false;

    val = m.GetBool();
    return true;
}

inline bool GetIntMember(rapidjson::Value& obj, const char* name, int& val)
{
    if (!obj.HasMember(name))
        return false;

    const auto& m = obj[name];
    if (!m.IsInt())
        return false;

    val = obj[name].GetInt();
    return true;
}

inline bool GetOptionalIntMember(rapidjson::Value& obj, const char* name, int& val, int defaultVal = 0)
{
    if (!obj.HasMember(name))
    {
        val = defaultVal;
        return true;
    }

    const auto& m = obj[name];
    if (!m.IsInt())
        return false;

    val = obj[name].GetInt();
    return true;
}

inline bool GetUintMember(rapidjson::Value& obj, const char* name, unsigned& val)
{
    if (!obj.HasMember(name))
        return false;

    const auto& m = obj[name];
    if (!m.IsUint())
        return false;

    val = obj[name].GetUint();
    return true;
}

inline bool GetOptionalUintMember(rapidjson::Value& obj, const char* name, unsigned& val, unsigned defaultVal = 0)
{
    if (!obj.HasMember(name))
    {
        val = defaultVal;
        return true;
    }

    const auto& m = obj[name];
    if (!m.IsUint())
        return false;

    val = obj[name].GetUint();
    return true;
}

inline bool GetInt64Member(rapidjson::Value& obj, const char* name, int64_t& val)
{
    if (!obj.HasMember(name))
        return false;

    const auto& m = obj[name];
    if (!m.IsInt64())
        return false;

    val = obj[name].GetInt64();
    return true;
}

inline bool GetOptionalInt64Member(rapidjson::Value& obj, const char* name, int64_t& val, int64_t defaultVal = 0)
{
    if (!obj.HasMember(name))
    {
        val = defaultVal;
        return true;
    }

    const auto& m = obj[name];
    if (!m.IsInt64())
        return false;

    val = obj[name].GetInt64();
    return true;
}

inline bool GetUint64Member(rapidjson::Value& obj, const char* name, uint64_t& val)
{
    if (!obj.HasMember(name))
        return false;

    const auto& m = obj[name];
    if (!m.IsUint64())
        return false;

    val = obj[name].GetUint64();
    return true;
}

inline bool GetOptionalUintMember(rapidjson::Value& obj, const char* name, uint64_t& val, uint64_t defaultVal = 0)
{
    if (!obj.HasMember(name))
    {
        val = defaultVal;
        return true;
    }

    const auto& m = obj[name];
    if (!m.IsUint64())
        return false;

    val = obj[name].GetUint64();
    return true;
}

inline bool GetDoubleMember(rapidjson::Value& obj, const char* name, double& val)
{
    if (!obj.HasMember(name))
        return false;

    const auto& m = obj[name];
    if (!m.IsDouble())
        return false;

    val = obj[name].GetDouble();
    return true;
}

inline bool GetOptionalDoubleMember(rapidjson::Value& obj, const char* name, double& val, double defaultVal = 0.0)
{
    if (!obj.HasMember(name))
    {
        val = defaultVal;
        return true;
    }

    const auto& m = obj[name];
    if (!m.IsDouble())
        return false;

    val = obj[name].GetDouble();
    return true;
}

RAPIDJSON_NAMESPACE_END

#endif//RAPIDJSON_UTILS_H_
