#ifndef DEFINES_H
    #define DEFINES_H

    // HTTP CODES
    #define HTTP_OK 200
    #define HTTP_SEE_OTHER 303
    #define HTTP_FILE_NOT_FOUND 404
    #define HTTP_UNAUTHORISED_ACCESS 401

    // MAX LIMITS
    #define MAX_WORKER_THERAD_COUNT 100
    #define MAX_IMAGE_SIZE_BYTES 10000000
    #define MAX_HEADER_ARG_LIST 40
    #define MAX_GET_ARGS 10
    #define MAX_COOKIE_COUNT 10

    // Content Types
    #define NONE    1 // "Error in URL"
    #define ICO  2 // "ico"
    #define JS  3   // "js"
    #define PNG 4   // "png"
    #define JPEG    5 // "jpeg"
    #define JSON    6//"json"
    #define CSS     7//"css"
    #define HTML    8//"html"
    #define HTML_GET 9
    #define DEFAULT  10// "/"

    // Server Types 
    #define Thread_Based_Server  1
    #define Process_Based_Server  2


    #define NUM 1
    #define CHAR 2

    //Debug Tools
    #define DEBUG   true
    #define print_debug(x) if(DEBUG) cout<<#x<<"\t"<<x<<endl

    /* Type to content received in the POST method 
    */
    #define form_urlencoded "application/x-www-form-urlencoded\r"
    #define multipart_form_data  "multipart/form-data\r"

    // reflection 
    #define REFLECT(x) template<typename R> void reflect(R& r) { r x; }
    #define NULL_str ""


    /**
     * @brief To check the reflection module working
     * 
     */
    // #define REFLECTION_CHECK

#endif