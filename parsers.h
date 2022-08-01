#ifndef PARSERS_H
#define PARSERS_H


#include "includes.h"
#include "def.h"
#include "structDef.h"

struct reqHeader parseReqHeader(std::string header);
int parseContentType(std::string filename);
struct urlDecode  urlParser(std::string url);
template<typename T>
int parseTemplate(std::string htmlTemplate, T& dataStruct);

#endif