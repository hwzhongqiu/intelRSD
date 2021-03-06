/*!
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file requests/network/get_remote_switch_info.cpp
 * @brief Network request get remote switch information implementation
 * */

#include "agent-framework/module-ref/requests/network/get_remote_ethernet_switch_info.hpp"
#include "agent-framework/module-ref/constants/network.hpp"
#include <json/json.h>

using namespace agent_framework::model::requests;
using namespace agent_framework::model::literals;

GetRemoteEthernetSwitchInfo::GetRemoteEthernetSwitchInfo(const std::string& switch_v):
    m_switch(switch_v){}

Json::Value GetRemoteEthernetSwitchInfo::to_json() const {
    Json::Value value;
    value[Switch::SWITCH] = m_switch;
    return value;
}

GetRemoteEthernetSwitchInfo GetRemoteEthernetSwitchInfo::from_json(const Json::Value& json) {
    return GetRemoteEthernetSwitchInfo{
        json[Switch::SWITCH].asString()
    };
}
