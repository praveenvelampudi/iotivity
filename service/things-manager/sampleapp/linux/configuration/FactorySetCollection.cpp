//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

///
/// This sample shows how one could create a resource (collection) with children.
///

#include <functional>
#include <thread>

#include "OCPlatform.h"
#include "OCApi.h"
#include "ThingsManager.h"
#include "FactorySetCollection.h"

using namespace OC;

FactorySetResource::FactorySetResource()
{
    m_configurationUri = "/factorySet"; // URI of the resource
    m_configurationTypes.clear();
    m_configurationTypes.push_back("factorySet"); // resource type name.
    m_configurationRep.setUri(m_configurationUri);
    m_configurationRep.setResourceTypes(m_configurationTypes);
}

FactorySetResource::~FactorySetResource(){}

/// This function internally calls registerResource API.
void FactorySetResource::createResources(ResourceEntityHandler callback)
{
    using namespace OC::OCPlatform;

    if (callback == NULL)
    {
        std::cout << "callback should be binded\t";
        return;
    }

    // This will internally create and register the resource.
    OCStackResult result = registerResource(m_configurationHandle, m_configurationUri,
            m_configurationTypes[0], m_configurationInterfaces[0], callback,
            OC_DISCOVERABLE | OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        std::cout << "Resource creation (configuration) was unsuccessful\n";
    }

    std::cout << "FactorySet Resource is Created!\n";
}

void FactorySetResource::setFactorySetRepresentation(OCRepresentation& rep)
{
    string value;

    if (rep.getValue("loc", value))
    {
        m_location = value;
        std::cout << "\t\t\t\t" << "m_location: " << m_location << std::endl;
    }

    if (rep.getValue("st", value))
    {
        std::cout << "\t\t\t\t" << "SystemTime is not allowed to be written." << std::endl;
    }

    if (rep.getValue("c", value))
    {
        m_currency = value;
        std::cout << "\t\t\t\t" << "m_currency: " << m_currency << std::endl;
    }

    if (rep.getValue("r", value))
    {
        m_region = value;
        std::cout << "\t\t\t\t" << "m_region: " << m_region << std::endl;
    }
}

OCRepresentation FactorySetResource::getFactorySetRepresentation()
{
    m_configurationRep.setValue("loc", m_location);
    m_configurationRep.setValue("st", m_systemTime);
    m_configurationRep.setValue("c", m_currency);
    m_configurationRep.setValue("r", m_region);

    return m_configurationRep;
}

std::string FactorySetResource::getUri()
{
    return m_configurationUri;
}

