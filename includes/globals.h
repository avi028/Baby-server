#include "includes.h"
#ifndef GLOBALS_H
#define GLOBALS_H

// Global variables 
std::string websiteFolder;
std::string IP;
int PORT;
int serverType;
int listening_sock;

// Temp volatile databse just of checking 
std::map<std::string,std::string> db_table;

// Session data storage
std::map<std::string,std::pair<std::string,std::string> > sessionKeyTable; 


#endif