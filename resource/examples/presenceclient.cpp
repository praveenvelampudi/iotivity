//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

// PresenceClient.cpp : A client example for presence notification
//
#include <string>
#include <cstdlib>
#include <pthread.h>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

std::shared_ptr<OCResource> curResource;

static int TEST_CASE = 0;

/**
 * List of methods that can be inititated from the client
 */
typedef enum {
    TEST_UNICAST_PRESENCE_NORMAL = 1,
    TEST_UNICAST_PRESENCE_WITH_FILTER,
    TEST_MULTICAST_PRESENCE_NORMAL,
    TEST_MULTICAST_PRESENCE_WITH_FILTER,
    MAX_TESTS
} CLIENT_TEST;

void printUsage()
{
    std::cout << "Usage : presenceclient -t <1|2>" << std::endl;
    std::cout << "-t 1 : Discover Resources and Initiate Unicast Presence" << std::endl;
    std::cout << "-t 2 : Discover Resources and Initiate Unicast Presence with Filter"
              << std::endl;
    std::cout << "-t 3 : Discover Resources and Initiate Multicast Presence" << std::endl;
    std::cout << "-t 4 : Discover Resources and Initiate Multicast Presence with Filter"
              << std::endl;
}

// Callback to presence
void presenceHandler(OCStackResult result, const unsigned int nonce)
{
    switch(result)
    {
        case OC_STACK_OK:
            std::cout << "Nonce# " << nonce << std::endl;
            break;
        case OC_STACK_PRESENCE_STOPPED:
            std::cout << "Presence Stopped\n";
            break;
        case OC_STACK_PRESENCE_DO_NOT_HANDLE:
            std::cout << "Presence do not handle\n";
            break;
        default:
            std::cout << "Error\n";
            break;
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    if(curResource)
    {
        std::cout << "Found another resource, ignoring"<<std::endl;
    }

    std::string resourceURI;
    std::string hostAddress;
    try
    {
        // Do some operations with resource object.
        if(resource)
        {
            std::cout<<"DISCOVERED Resource:"<<std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for(auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for(auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            if(resourceURI == "/a/light")
            {
                curResource = resource;
                OCPlatform::OCPresenceHandle presenceHandle;

                if(TEST_CASE == TEST_UNICAST_PRESENCE_NORMAL)
                {
                    OCPlatform::subscribePresence(presenceHandle, hostAddress,
                            &presenceHandler);
                    std::cout<< "Subscribed to unicast address:" << hostAddress <<std::endl;
                }
                else if(TEST_CASE == TEST_UNICAST_PRESENCE_WITH_FILTER)
                {
                    OCPlatform::subscribePresence(presenceHandle, hostAddress, "core.light",
                            &presenceHandler);
                    std::cout<< "Subscribed (with resource type) to unicast address:"
                                << hostAddress << std::endl;
                }
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch(std::exception& e)
    {
        //log(e.what());
    }
}

int main(int argc, char* argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "t:")) != -1)
    {
        switch(opt)
        {
            case 't':
                TEST_CASE = atoi(optarg);
                break;
            default:
                printUsage();
                return -1;
        }
    }
    if(TEST_CASE >= MAX_TESTS || TEST_CASE <= 0)
    {
        printUsage();
        return -1;
    }

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Client,
        "0.0.0.0",
        0,
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);

    try
    {
        std::cout << "Created Platform..."<<std::endl;

        OCPlatform::OCPresenceHandle presenceHandle;

        if(TEST_CASE == TEST_MULTICAST_PRESENCE_NORMAL)
        {
            OCPlatform::subscribePresence(presenceHandle, OC_MULTICAST_IP, presenceHandler);
            std::cout<< "Subscribed to multicast" <<std::endl;
        }
        else if(TEST_CASE == TEST_MULTICAST_PRESENCE_WITH_FILTER)
        {
            OCPlatform::subscribePresence(presenceHandle, OC_MULTICAST_IP, "core.light",
                    &presenceHandler);
            std::cout<< "Subscribed to multicast with resource type" <<std::endl;
        }
        else
        {
            // Find all resources
            OCPlatform::findResource("", "coap://224.0.1.187/oc/core", &foundResource);
            std::cout<< "Finding Resource... " <<std::endl;
        }
        while(true)
        {
            // some operations
        }

    }catch(OCException& e)
    {
        //log(e.what());
    }

    return 0;
}
