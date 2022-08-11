#include "includes.h"
#include "structDef.h"

#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Checks for number or character
 * 
 * @param c 
 * @return int 
 */
int isNumChar(char c);
/**
 * @brief  tokenizes string as per the delimeter
 * 
 * @param str 
 * @param delimeter 
 * @return std::vector<std::string> 
 */
std::vector<std::string> str_tok(std::string str,char delimeter);
/**
 * @brief Get the Hash value
 * 
 * @param str 
 * @return unsigned long long int 
 */
unsigned long long int getHash(std::string str);
/**
 * @brief Get the Cookie count as return and all the cookies present 
 * in the requestHeader in the cookie-set.
 * 
 * @param reqh 
 * @param cookieSet 
 * @return int 
 */
int  getCookies(struct reqHeader reqh, struct requestArg * cookieSet);
/**
 * @brief loads IP Port Number Websiter Folder for the server.
 * 
 * @return int 
 */
int loadConfigFile();

#endif