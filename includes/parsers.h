#ifndef PARSERS_H
#define PARSERS_H


#include "includes.h"
#include "def.h"
#include "structDef.h"

/**
 * @brief Parses the Reqest header
 * 
 * @param header 
 * @return struct reqHeader 
 */
struct reqHeader parseReqHeader(std::string header);

/**
 * @brief Parses the filename and returns a value associated with it
 * 
 * @param filename 
 * @return int 
 */
int parseContentType(std::string filename);

/**
 * @brief Parses the URL and slices the contents to fill the 
 * urlDecode structure
 * 
 * @param url 
 * @return struct urlDecode 
 */
struct urlDecode  urlParser(std::string url);

#endif