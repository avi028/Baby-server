#include "includes.h"

#ifndef UTILS_H
#define UTILS_H

int isNumChar(char c);
std::vector<std::string> str_tok(std::string str,char delimeter);
unsigned long long int getHash(std::string str);

#endif