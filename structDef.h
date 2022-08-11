#include "includes.h"
#include "def.h"

#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

    // required Structures
    struct params{
        int client_socket;
        sockaddr_in commSock;
        socklen_t commlen;
    };

    /********************************************

        /index.html ?   name=john   &   pass=doe

    _name
    _ext
    _numberOfArg
    _arg1Name
    _arg1Value
    _arg2Name
    _arg2Value
    _arg3Name
    _arg3Value
    ....
    ...
    ..
    .
    *********************************************/



   /**
    * @brief  Structure to store the arguments received in the client request
    * 
    */
    struct requestArg{
        std::string name;
        std::string value;
    };

   /**
    * @brief  URL structure to store the different sections of URL 
    * 
    */
    struct urlDecode{
        std::string service;
        std::string ext;
        int numberOfArg;
        struct requestArg  cg[MAX_GET_ARGS];
        bool valid;
    };

    /**
     * @brief  Request header structure to decode and store the different parts of the Header section of the client Request 
     * 
     */
    struct reqHeader{
        std::string  typeOfReq;
        std::string  url;
        std::string  httpVersion;
        int reqArgCnt;
        std::string inputType;
        std::string multipartBoundary ;
        struct requestArg inputArgs[MAX_GET_ARGS];
        int inputArgsCount;
        struct requestArg headerArgList[MAX_HEADER_ARG_LIST];
        bool valid;
    };

    /**
     * @brief To Response to the client Request below mentioned things are required 
     * 
     */
    struct serviceResponse{
        std::string responseHeader;
        std::string response;
        int responseType;
        int responseCode;
    };

#endif