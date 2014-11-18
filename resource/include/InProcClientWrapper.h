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

#ifndef _IN_PROC_CLIENT_WRAPPER_H_
#define _IN_PROC_CLIENT_WRAPPER_H_

#include <thread>
#include <mutex>
#include <sstream>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <OCApi.h>
#include <ocstack.h>
#include <IClientWrapper.h>
#include <InitializeException.h>
#include <ResourceInitException.h>

namespace OC
{
    class InProcClientWrapper : public IClientWrapper
    {

    public:
        enum OCSecureType
        {
            IPV4Secure,
            IPV4
        };

        InProcClientWrapper(OC::OCPlatform_impl& owner, std::weak_ptr<std::recursive_mutex> csdkLock,
                            PlatformConfig cfg);
        virtual ~InProcClientWrapper();

        virtual OCStackResult ListenForResource(const std::string& serviceUrl,
            const std::string& resourceType, FindCallback& callback,
            QualityOfService QoS);

        virtual OCStackResult GetResourceRepresentation(const std::string& host,
            const std::string& uri, const QueryParamsMap& queryParams,
            const HeaderOptions& headerOptions,
            GetCallback& callback, QualityOfService QoS);

        virtual OCStackResult PutResourceRepresentation(const std::string& host,
            const std::string& uri, const OCRepresentation& attributes,
            const QueryParamsMap& queryParams, const HeaderOptions& headerOptions,
            PutCallback& callback, QualityOfService QoS);

        virtual OCStackResult PostResourceRepresentation(const std::string& host,
            const std::string& uri, const OCRepresentation& attributes,
            const QueryParamsMap& queryParams, const HeaderOptions& headerOptions,
            PostCallback& callback, QualityOfService QoS);

        virtual OCStackResult DeleteResource(const std::string& host, const std::string& uri,
             const HeaderOptions& headerOptions, DeleteCallback& callback, QualityOfService QoS);

        virtual OCStackResult ObserveResource(ObserveType observeType, OCDoHandle* handle,
            const std::string& host, const std::string& uri, const QueryParamsMap& queryParams,
            const HeaderOptions& headerOptions, ObserveCallback& callback, QualityOfService QoS);

        virtual OCStackResult CancelObserveResource(OCDoHandle handle, const std::string& host,
            const std::string& uri, const HeaderOptions& headerOptions, QualityOfService QoS);

        virtual OCStackResult SubscribePresence(OCDoHandle* handle, const std::string& host,
            const std::string& resourceType, SubscribeCallback& presenceHandler);

        virtual OCStackResult UnsubscribePresence(OCDoHandle handle);
        // Note: this should never be called by anyone but the handler for the listen command.
        // It is public becuase that needs to be a non-instance callback
        virtual std::shared_ptr<OCResource> parseOCResource(IClientWrapper::Ptr clientWrapper,
            OCDevAddr& addr, const boost::property_tree::ptree resourceNode);

        OCStackResult GetDefaultQos(QualityOfService& QoS);
    private:
        std::string convertOCAddrToString(OCDevAddr& addr,
        OCSecureType type, const std::string &portStr = std::string());
        void listeningFunc();
        std::string assembleSetResourceUri(std::string uri, const QueryParamsMap& queryParams);
        std::string assembleSetResourcePayload(const OCRepresentation& attributes);
        void assembleHeaderOptions(OCHeaderOption options[],
            const HeaderOptions& headerOptions);
        std::thread m_listeningThread;
        bool m_threadRun;
        std::weak_ptr<std::recursive_mutex> m_csdkLock;

    private:
        OC::OCPlatform_impl& m_owner;
        PlatformConfig  m_cfg;
    };
}

#endif