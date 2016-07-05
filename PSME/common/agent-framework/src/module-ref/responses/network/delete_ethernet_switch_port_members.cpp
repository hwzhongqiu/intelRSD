/*!
 * @copyright
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * @copyright
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * @copyright
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * @copyright
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * @file responses/network/delete_ethernet_switch_port_members.cpp
 * @brief DeleteEthernetSwitchPortMembers response
 * */

#include "agent-framework/module-ref/responses/network/delete_ethernet_switch_port_members.hpp"
#include "agent-framework/module-ref/constants/network.hpp"
#include <json/json.h>

using namespace agent_framework::model::responses;
using namespace agent_framework::model::literals;

DeleteEthernetSwitchPortMembers::DeleteEthernetSwitchPortMembers(Oem oem):
    m_oem{oem} {}


Json::Value DeleteEthernetSwitchPortMembers::to_json() const {
    Json::Value value;
    value[SwitchPort::OEM] = m_oem.to_json();
    return value;
}

DeleteEthernetSwitchPortMembers DeleteEthernetSwitchPortMembers::from_json
    (const Json::Value& json) {
    return DeleteEthernetSwitchPortMembers{Oem::from_json(
        json[SwitchPort::OEM])};
}