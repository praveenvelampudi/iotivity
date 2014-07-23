//******************************************************************
//
// Copyright 2014 Intel Corporation All Rights Reserved.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//

//-----------------------------------------------------------------------------
// Internal include file used by lower layers of the OC stack
//-----------------------------------------------------------------------------
#ifndef OCSTACKINTERNAL_H_
#define OCSTACKINTERNAL_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "ocstack.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct rsrc_t;

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef struct resourcetype_t {
    struct resourcetype_t *next; // linked list; for multiple types on resource

    // Name of the type; this string is ‘.’ (dot) separate list of segments where each segment is a
    // namespace and the final segment is the type; type and sub-types can be separate with ‘-‘ (dash)
    // usually only two segments would be defined. Either way this string is meant to be human friendly
    // and is used opaquely and not parsed by code
    // This name is used in the “rt=” parameter of a resource description when resources are introspected
    // and is also use in the <base URI>/types list of available types
    char *resourcetypename;
    // An array of strings; each string defines the attribute and attribute data type.
    // NOTE: this is not the same as the request/response payload representation;
    // but is the definition of attribute/value pairs that go into the payload
    char *typerepresentation;
} OCResourceType;

typedef struct attr_t {
    struct attr_t *next; // Points to next resource in list

    // The name of the attribute; used to look up the attribute in list;
    // for a given attribute SHOULD not be changed once assigned
    const char *attrName;
    char *attrValue; // value of the attribute as string
} OCAttribute;

typedef struct resourceinterface_t {
    struct resourceinterface_t *next; // linked list; for multiple interfaces on resource

    // Name of the interface; this is ‘.’ (dot) separate list of segments where each segment is
    // a namespace and the final segment is the interface; usually only two segments would be defined.
    // Either way this string is opaque and not parsed by segment
    char *name ;
    // Allowed methods on this resource through this interface
    // Use the methods in OCMethod to create a bitmap
    // Example: OC_REST_GET|OC_REST_PUT.
    uint8_t allowedMethods;
    // Supported content types to serialize request and response on this interface
    // (REMOVE for V1 – only jSON for all but core.ll that uses Link Format)
#if 0
    char *inputContentType ;
    char *outputContentType ;
#endif
    /*** Future placeholder for access control and policy ***/
} OCResourceInterface;

typedef struct rsrc_t {
    struct rsrc_t *next; // Points to next resource in list
    // Relative path on the device; will be combined with base url to create fully qualified path
    char *uri;
    OCResourceType *rsrcType; // Resource type(s); linked list
    OCResourceInterface *rsrcInterface; // Resource interface(s); linked list
    OCAttribute *rsrcAttributes; // Resource interface(s); linked list
    // Array of pointers to resources; can be used to represent a container of resources
    // (i.e. hierarchies of resources) or for reference resources (i.e. for a resource collection)
    struct rsrc_t *rsrcResources[MAX_CONTAINED_RESOURCES];
    //struct rsrc_t *rsrcResources;
    // Pointer to function that handles the entity bound to the resource.
    // This handler has to be explicitly defined by the programmer
    OCEntityHandler entityHandler;
    // Properties on the resource – defines meta information on the resource
    OCResourceProperty resourceProperties ; /* ACTIVE, DISCOVERABLE etc */
    // Pointer to an opaque object where app/user specific data can be placed with the resource;
    // this could be information for the entity handler between invocations
    void *context;
    // NOTE: Methods supported by this resource should be based on the interface targeted
    // i.e. look into the interface structure based on the query request Can be removed here; place holder for the note above
    /* method_t methods; */
} OCResource;

// following structure will be created in occoap and passed up the stack on the server side
typedef struct {
    // resourceUrl will be filled in occoap using the path options in received request PDU
    unsigned char * resourceUrl;
    // resourceUrl will be filled in occoap using the query options in received request PDU
    unsigned char * query;
    // qos is indicating if the request is CON or NON
    OCQualityOfService qos;
    // this structure will be passed to entity handler
    OCEntityHandlerRequest * entityHandlerRequest;
} OCRequest;

// following structure will be created in occoap and passed up the stack on the client side
typedef struct {
    // token is copied from the received response PDU
    OCToken * token;
    // this structure will be passed to client
    OCClientResponse * clientResponse;
} OCResponse;

//-----------------------------------------------------------------------------
// Internal function prototypes
//-----------------------------------------------------------------------------

OCStackResult OCStackHandleReceiveRequest(OCRequest * request);
void OCStackHandleReceiveResponse(OCResponse * response);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* OCSTACKINTERNAL_H_ */