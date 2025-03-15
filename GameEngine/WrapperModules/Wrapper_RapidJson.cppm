module;
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

export module Wrapper.RapidJson;

export using JsonDocument = rapidjson::Document;
export using JsonObject = rapidjson::Value;
export using rapidjson::IStreamWrapper;