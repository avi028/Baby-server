#include "includes.h"
#ifndef SESSIONS_H
#define SESSIONS_H

std::string getsessionKey(std::string user,std::string pass);
bool removeSessionKey(std::string sessionKey);
std::string getUserForSessionKey(std::string sessionKey);

#endif