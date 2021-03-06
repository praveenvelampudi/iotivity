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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "ocstack.h"
#include "logger.h"
#include "occlientslow.h"

static int UNICAST_DISCOVERY = 0;
static int TEST_CASE = 0;
static const char * UNICAST_DISCOVERY_QUERY = "coap://%s:6298/oc/core";
static std::string putPayload = "{\"state\":\"off\",\"power\":10}";
static std::string coapServerIP = "255.255.255.255";
static std::string coapServerPort = "5683";
static std::string coapServerResource = "/a/led";

//The following variable determines the interface protocol (IPv4, IPv6, etc)
//to be used for sending unicast messages. Default set to IPv4.
static OCConnectivityType OC_CONNTYPE = OC_IPV4;
static const char * MULTICAST_RESOURCE_DISCOVERY_QUERY = "/oc/core";
static int IPV4_ADDR_SIZE = 16;
void StripNewLineChar(char* str);

int gQuitFlag = 0;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

static void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : occlient -c <0|1> -u <0|1> -t <1|2|3>");
    OC_LOG(INFO, TAG, "-c <0|1> : IPv4/IPv6 (IPv6 not currently supported)");
    OC_LOG(INFO, TAG, "-u <0|1> : Perform multicast/unicast discovery of resources");
    OC_LOG(INFO, TAG, "-t 1 : Discover Resources");
    OC_LOG(INFO, TAG, "-t 2 : Discover Resources and Initiate Nonconfirmable Get Request");
    OC_LOG(INFO, TAG, "-t 3 : Discover Resources and Initiate Confirmable Get Request");
}

OCStackResult InvokeOCDoResource(std::ostringstream &query,
        OCMethod method, OCQualityOfService qos,
        OCClientResponseHandler cb, OCHeaderOption * options, uint8_t numOptions)
{
    OCStackResult ret;
    OCCallbackData cbData;

    cbData.cb = cb;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, method, query.str().c_str(), 0,
            NULL, OC_CONNTYPE, qos, &cbData, options, numOptions);

    if (ret != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "OCDoResource returns error %d with method %d", ret, method);
    }

    return ret;
}

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse)
{
    if(clientResponse == NULL)
    {
        OC_LOG(INFO, TAG, "The clientResponse is NULL");
        return   OC_STACK_DELETE_TRANSACTION;
    }
    if(ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for GET query recvd successfully");
    }

    OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
    OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
    OC_LOG_V(INFO, TAG, "JSON = %s =============> Get Response",
            clientResponse->resJSONPayload);

    if(clientResponse->rcvdVendorSpecificHeaderOptions &&
            clientResponse->numRcvdVendorSpecificHeaderOptions)
    {
        OC_LOG (INFO, TAG, "Received vendor specific options");
        uint8_t i = 0;
        OCHeaderOption * rcvdOptions = clientResponse->rcvdVendorSpecificHeaderOptions;
        for( i = 0; i < clientResponse->numRcvdVendorSpecificHeaderOptions; i++)
        {
            if(((OCHeaderOption)rcvdOptions[i]).protocolID == OC_COAP_ID)
            {
                OC_LOG_V(INFO, TAG, "Received option with OC_COAP_ID and ID %u with",
                        ((OCHeaderOption)rcvdOptions[i]).optionID );

                OC_LOG_BUFFER(INFO, TAG, ((OCHeaderOption)rcvdOptions[i]).optionData,
                    MAX_HEADER_OPTION_DATA_LENGTH);
            }
        }
    }
    return OC_STACK_DELETE_TRANSACTION;
}

// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
        OCClientResponse * clientResponse)
{
    uint8_t remoteIpAddr[4];
    uint16_t remotePortNu;

    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for DISCOVER query recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));

        OCDevAddrToIPv4Addr((OCDevAddr *) clientResponse->addr, remoteIpAddr,
                remoteIpAddr + 1, remoteIpAddr + 2, remoteIpAddr + 3);
        OCDevAddrToPort((OCDevAddr *) clientResponse->addr, &remotePortNu);

        OC_LOG_V(INFO, TAG,
                "Device =============> Discovered %s @ %d.%d.%d.%d:%d",
                clientResponse->resJSONPayload, remoteIpAddr[0], remoteIpAddr[1],
                remoteIpAddr[2], remoteIpAddr[3], remotePortNu);

        parseClientResponse(clientResponse);

        switch(TEST_CASE)
        {
            case TEST_NON_CON_OP:
                InitGetRequest(OC_LOW_QOS);
                break;
            case TEST_CON_OP:
                InitGetRequest(OC_HIGH_QOS);
                break;
            default:
                PrintUsage();
                break;
        }
    }

    return (UNICAST_DISCOVERY) ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION ;

}

int InitGetRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << "coap://" << coapServerIP << ":" << coapServerPort << coapServerResource;

    return (InvokeOCDoResource(query, OC_REST_GET, (qos == OC_HIGH_QOS)?
            OC_HIGH_QOS:OC_LOW_QOS, getReqCB, NULL, 0));
}

int InitDiscovery()
{
    OCStackResult ret;
    OCCallbackData cbData;
    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };
    if (UNICAST_DISCOVERY)
    {
        char ipv4addr[IPV4_ADDR_SIZE];
        printf("Enter IPv4 address of the Server hosting resource (Ex: 192.168.0.15)\n");
        if (fgets(ipv4addr, IPV4_ADDR_SIZE, stdin))
        {
            //Strip newline char from ipv4addr
            StripNewLineChar(ipv4addr);
            snprintf(szQueryUri, sizeof(szQueryUri), UNICAST_DISCOVERY_QUERY, ipv4addr);
        }
        else
        {
            OC_LOG(ERROR, TAG, "!! Bad input for IPV4 address. !!");
            return OC_STACK_INVALID_PARAM;
        }
    }
    else
    {
        strcpy(szQueryUri, MULTICAST_RESOURCE_DISCOVERY_QUERY);
    }
    cbData.cb = discoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    if(UNICAST_DISCOVERY)
    {
        ret = OCDoResource(NULL, OC_REST_GET, szQueryUri, 0, 0, OC_CONNTYPE,
                OC_LOW_QOS, &cbData, NULL, 0);
    }
    else
    {
        ret = OCDoResource(NULL, OC_REST_GET, szQueryUri, 0, 0, OC_ALL,
                OC_LOW_QOS, &cbData, NULL, 0);
    }
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int main(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "u:t:c:")) != -1)
    {
        switch(opt)
        {
            case 'u':
                UNICAST_DISCOVERY = atoi(optarg);
                break;
            case 't':
                TEST_CASE = atoi(optarg);
                break;
            case 'c':
                // TODO: re-enable IPv4/IPv6 command line selection when IPv6 is supported
                // OC_CONNTYPE = OCConnectivityType(atoi(optarg));
                OC_CONNTYPE = OC_IPV4;
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if ((UNICAST_DISCOVERY != 0 && UNICAST_DISCOVERY != 1) ||
            (TEST_CASE < TEST_DISCOVER_REQ || TEST_CASE >= MAX_TESTS) )
    {
        PrintUsage();
        return -1;
    }

    /* Initialize OCStack*/
    if (OCInit(NULL, 0, OC_CLIENT) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    InitDiscovery();

    // Break from loop with Ctrl+C
    OC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(2);
    }
    OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}

std::string getIPAddrTBServer(OCClientResponse * clientResponse)
{
    if(!clientResponse) return "";
    if(!clientResponse->addr) return "";
    uint8_t a, b, c, d = 0;
    if(0 != OCDevAddrToIPv4Addr(clientResponse->addr, &a, &b, &c, &d) ) return "";

    char ipaddr[16] = {'\0'};
    // ostringstream not working correctly here, hence snprintf
    snprintf(ipaddr,  sizeof(ipaddr), "%d.%d.%d.%d", a,b,c,d);
    return std::string (ipaddr);
}

std::string getPortTBServer(OCClientResponse * clientResponse)
{
    if(!clientResponse) return "";
    if(!clientResponse->addr) return "";
    uint16_t p = 0;
    if(0 != OCDevAddrToPort(clientResponse->addr, &p) ) return "";
    std::ostringstream ss;
    ss << p;
    return ss.str();
}

std::string getQueryStrForGetPut(OCClientResponse * clientResponse)
{
    return "/a/led";
}

void parseClientResponse(OCClientResponse * clientResponse)
{
    coapServerIP = getIPAddrTBServer(clientResponse);
    coapServerPort = getPortTBServer(clientResponse);
    coapServerResource = getQueryStrForGetPut(clientResponse);
}

