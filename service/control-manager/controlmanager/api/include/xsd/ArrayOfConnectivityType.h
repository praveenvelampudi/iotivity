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



#if !defined(_ARRAYOFCONNECTIVITYTYPE_H)
#define _ARRAYOFCONNECTIVITYTYPE_H

#include "CMSerialization.h"
#include "list"
#include "string"
#include "sstream"

#include "ConnectivityType.h"

/**
*  @addtogroup xsd
*  @{
*/

/**
 * @class   ArrayOfConnectivityType
 *
 * ArrayOfConnectivityType class implements OC::Cm::Serialization::ISerializable, defines methods to serialize and deserialize content of XSD Simple Type ArrayOfConnectivityType List.
 * @since   1.0.0
 */
class ArrayOfConnectivityType : public OC::Cm::Serialization::ISerializable
{
    public:

        /**
         * Default constructor of ArrayOfConnectivityType class.
         * @since   1.0.0
         */
        ArrayOfConnectivityType();

        /**
         * Default virtual destructor of ArrayOfConnectivityType class.
         * @since   1.0.0
         */
        virtual ~ArrayOfConnectivityType();

        /**
         * Serializes the contents of ArrayOfConnectivityType object.
         *
         * @param[in]   serializer  Serializer object that provides methods to serialize.
         *
         * @return  @c  True    if serialization is successful@n
         *          @c  False   if serialization failed
         * @since   1.0.0
         */
        virtual bool serializeContents(OC::Cm::Serialization::ISerializer &serializer);

        /**
         * Deserializes data and prepares the ArrayOfConnectivityType object contents.
         *
         * @param[in]   deserializer    Deserializer object that provides methods to deserialize.
         *
         * @return  @c  True    if deserialization is successful@n
         *          @c  False   if deserialization failed
         * @since   1.0.0
         */
        virtual bool deSerializeContents(OC::Cm::Serialization::IDeserializer &deserializer);

        /**
         * Returns name of the serializable.
         *
         * @return  Name of the serializable
         * @since   1.0.0
         */
        virtual std::string getElementName();

        /**
         * Variable that contains ConnectivityType objects in the List.
         */
        std::list< ::ConnectivityType> members;
};
/** @} */
#endif