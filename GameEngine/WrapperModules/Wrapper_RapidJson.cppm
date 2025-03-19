module;
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/prettywriter.h"

export module Wrapper.RapidJson;

export namespace Json
{
    using rapidjson::MemoryPoolAllocator;
    using rapidjson::SizeType;
    using rapidjson::GenericStringRef;
    
    using rapidjson::kNullType;
    using rapidjson::kFalseType;
    using rapidjson::kTrueType;
    using rapidjson::kObjectType;
    using rapidjson::kArrayType;
    using rapidjson::kStringType;
    using rapidjson::kNumberType;

    using rapidjson::StringBuffer;
    using rapidjson::Writer;
    using rapidjson::PrettyWriter;
}

export using JsonDocument = rapidjson::Document;
export using JsonObject = rapidjson::Value;
export using rapidjson::IStreamWrapper;