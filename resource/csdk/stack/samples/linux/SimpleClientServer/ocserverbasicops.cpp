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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "ocstack.h"
#include "logger.h"
#include "cJSON.h"
#include "ocserverbasicops.h"

//string length of "/a/led/" + std::numeric_limits<int>::digits10 + '\0'"
// 7 + 9 + 1 = 17
const int URI_MAXSIZE = 17;

volatile sig_atomic_t gQuitFlag = 0;

static LEDResource LED;
// This variable determines instance number of the LED resource.
// Used by POST method to create a new instance of LED resource.
static int gCurrLedInstance = 0;
#define SAMPLE_MAX_NUM_POST_INSTANCE 2
static LEDResource gLedInstance[SAMPLE_MAX_NUM_POST_INSTANCE];

char *gResourceUri= (char *)"/a/led";

//This function takes the request as an input and returns the response
//in JSON format.
char* constructJsonResponse (OCEntityHandlerRequest *ehRequest)
{
    cJSON *json = cJSON_CreateObject();

    if(!json)
    {
        OC_LOG (ERROR, TAG, "json object not created properly");
        return NULL;
    }

    cJSON *format;
    char *jsonResponse;
    LEDResource *currLEDResource = &LED;

    if (ehRequest->resource == gLedInstance[0].handle)
    {
        currLEDResource = &gLedInstance[0];
        gResourceUri = (char *) "a/led/0";
    }
    else if (ehRequest->resource == gLedInstance[1].handle)
    {
        currLEDResource = &gLedInstance[1];
        gResourceUri = (char *) "a/led/1";
    }

    if(OC_REST_PUT == ehRequest->method)
    {
        cJSON *putJson = cJSON_Parse((char *)ehRequest->reqJSONPayload);

        if(!putJson)
        {
            OC_LOG (ERROR, TAG, "putJson object not created properly");
            cJSON_Delete(json);
            return NULL;
        }
        currLEDResource->state = ( !strcmp(cJSON_GetObjectItem(putJson,"state")->valuestring ,
                "on") ? true:false);
        currLEDResource->power = cJSON_GetObjectItem(putJson,"power")->valuedouble;
        cJSON_Delete(putJson);
    }

    cJSON_AddStringToObject(json,"href",gResourceUri);
    format = cJSON_CreateObject();

    if(!format)
    {
        OC_LOG (ERROR, TAG, "format object not created properly");
        cJSON_Delete(json);
        return NULL;
    }

    cJSON_AddItemToObject(json, "rep", format);
    cJSON_AddStringToObject(format, "state", (char *) (currLEDResource->state ? "on":"off"));
    cJSON_AddNumberToObject(format, "power", currLEDResource->power);

    jsonResponse = cJSON_Print(json);
    cJSON_Delete(json);
    return jsonResponse;
}

OCEntityHandlerResult ProcessGetRequest (OCEntityHandlerRequest *ehRequest, char *payload,
        uint16_t maxPayloadSize)
{
    OCEntityHandlerResult ehResult;
    char *getResp = constructJsonResponse(ehRequest);

    if(getResp)
    {
        if (maxPayloadSize > strlen ((char *)getResp))
        {
            strncpy(payload, getResp, strlen((char *)getResp));
            ehResult = OC_EH_OK;
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                    maxPayloadSize);
            ehResult = OC_EH_ERROR;
        }

        free(getResp);
    }
    else
    {
        ehResult = OC_EH_ERROR;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessPutRequest (OCEntityHandlerRequest *ehRequest, char *payload,
        uint16_t maxPayloadSize)
{
    OCEntityHandlerResult ehResult;
    char *putResp = constructJsonResponse(ehRequest);

    if(putResp)
    {
        if (maxPayloadSize > strlen ((char *)putResp))
        {
            strncpy(payload, putResp, strlen((char *)putResp));
            ehResult = OC_EH_OK;
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                    maxPayloadSize);
            ehResult = OC_EH_ERROR;
        }

        free(putResp);
    }
    else
    {
        ehResult = OC_EH_ERROR;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessPostRequest (OCEntityHandlerRequest *ehRequest, char *payload,
        uint16_t maxPayloadSize)
{
    char *respPLPost_led = NULL;
    cJSON *json;
    cJSON *format;
    OCEntityHandlerResult ehResult;

    /*
     * The entity handler determines how to process a POST request.
     * Per the REST paradigm, POST can also be used to update representation of existing
     * resource or create a new resource.
     * In the sample below, if the POST is for /a/led then a new instance of the LED
     * resource is created with default representation (if representation is included in
     * POST payload it can be used as initial values) as long as the instance is
     * lesser than max new instance count. Once max instance count is reached, POST on
     * /a/led updated the representation of /a/led (just like PUT)
     */

    if (ehRequest->resource == LED.handle)
    {
        if (gCurrLedInstance < SAMPLE_MAX_NUM_POST_INSTANCE)
        {
            // Create new LED instance
            char newLedUri[URI_MAXSIZE ];
            snprintf(newLedUri, URI_MAXSIZE, "/a/led/%d", gCurrLedInstance);

            json = cJSON_CreateObject();
            if(!json)
            {
                return OC_EH_ERROR;
            }

            cJSON_AddStringToObject(json,"href",gResourceUri);
            format = cJSON_CreateObject();

            if(!format)
            {
                return OC_EH_ERROR;
            }

            cJSON_AddItemToObject(json, "rep", format);
            cJSON_AddStringToObject(format, "createduri", (char *) newLedUri);

            if (0 == createLEDResource (newLedUri, &gLedInstance[gCurrLedInstance], false, 0))
            {
                OC_LOG (INFO, TAG, "Created new LED instance");
                gLedInstance[gCurrLedInstance].state = 0;
                gLedInstance[gCurrLedInstance].power = 0;
                gCurrLedInstance++;
                respPLPost_led = cJSON_Print(json);
            }

            cJSON_Delete(json);
        }
        else
        {
            respPLPost_led = constructJsonResponse(ehRequest);
        }
    }
    else
    {
        for (int i = 0; i < SAMPLE_MAX_NUM_POST_INSTANCE; i++)
        {
            if (ehRequest->resource == gLedInstance[i].handle)
            {
                if (i == 0)
                {
                    respPLPost_led = constructJsonResponse(ehRequest);
                    break;
                }
                else if (i == 1)
                {
                    respPLPost_led = constructJsonResponse(ehRequest);
                }
            }
        }
    }

    if ((respPLPost_led != NULL) && (maxPayloadSize > strlen ((char *)respPLPost_led)))
    {
        strncpy(payload, respPLPost_led, strlen((char *)respPLPost_led));
        ehResult = OC_EH_OK;
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                maxPayloadSize);
        ehResult = OC_EH_ERROR;
    }

    free(respPLPost_led);

    return ehResult;
}

OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest)
{
    OC_LOG_V (INFO, TAG, "Inside entity handler - flags: 0x%x", flag);

    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCEntityHandlerResponse response;
    char payload[MAX_RESPONSE_LENGTH] = {0};

    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        if (entityHandlerRequest)
        {
            if (OC_REST_GET == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_GET from client");
                ehResult = ProcessGetRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
            }
            else if (OC_REST_PUT == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_PUT from client");
                ehResult = ProcessPutRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
            }
            else if (OC_REST_POST == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_POST from client");
                ehResult = ProcessPostRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
            }
            else
            {
                OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                        entityHandlerRequest->method);
            }

            if (ehResult == OC_EH_OK)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = entityHandlerRequest->requestHandle;
                response.resourceHandle = entityHandlerRequest->resource;
                response.ehResult = ehResult;
                response.payload = payload;
                response.payloadSize = strlen(payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
                memset(response.resourceUri, 0, sizeof(response.resourceUri));
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;

                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error sending response");
                    ehResult = OC_EH_ERROR;
                }
            }
        }
    }
    return ehResult;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

int main(int argc, char* argv[])
{
    OC_LOG(DEBUG, TAG, "OCServer is starting...");
    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Declare and create the example resource: LED
     */
    createLEDResource(gResourceUri, &LED, false, 0);

    // Break from loop with Ctrl-C
    OC_LOG(INFO, TAG, "Entering ocserver main loop...");
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

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

int createLEDResource (char *uri, LEDResource *ledResource, bool resourceState, int resourcePower)
{
    if (!uri)
    {
        OC_LOG(ERROR, TAG, "Resource URI cannot be NULL");
        return -1;
    }

    ledResource->state = resourceState;
    ledResource->power= resourcePower;
    OCStackResult res = OCCreateResource(&(ledResource->handle),
            "core.led",
            OC_RSRVD_INTERFACE_DEFAULT,
            uri,
            OCEntityHandlerCb,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created LED resource with result: %s", getResult(res));

    return 0;
}

