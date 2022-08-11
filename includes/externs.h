#include "includes.h"
#ifndef EXTERNS_H
#define EXTERNS_H

// Global variables 
extern std::string websiteFolder;
extern std::string IP;
extern int PORT;
extern int serverType;
extern int listening_sock;

// Session data storage
extern std::map<std::string,std::pair<std::string,std::string> > sessionKeyTable; 

// Temp volatile databse just of checking 
extern std::map<std::string,std::string> db_table;



#endif