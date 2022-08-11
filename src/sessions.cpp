#include "../includes/utils.h"
#include "../includes/def.h"
#include "../includes/externs.h"
#include "../includes/sessions.h"

/* Returns the session key from the table 
    input : user std::string , key std::string 
    output : sessionKey std::string 
 */
std::string getsessionKey(std::string user,std::string pass){
    std::string sessionKey = std::to_string(getHash(user)+getHash(pass)+(unsigned long long int )rand()%10000);    
    sessionKeyTable.insert({sessionKey,make_pair(user,pass)});

    // print_debug("----------------------------After Insertion");
    // print_debug(sessionKey);
    // print_debug(sessionKeyTable[sessionKey].first);
    // print_debug(sessionKeyTable[sessionKey].second);
    return sessionKey;
}

/* Removes the session key from the table 
    Input : sessionKey std::string
 */
bool removeSessionKey(std::string sessionKey){
    if (sessionKeyTable.erase(sessionKey)>0)
        return true;
    return false;
}


/* Finds the user in the table using the session key 
    Input : sessionKey std::string
    Output : username std::string
 */
std::string getUserForSessionKey(std::string sessionKey){    
    std::string user="";
    std::map<std::string,std::pair<std::string,std::string> >::iterator it = sessionKeyTable.find(sessionKey);
    if(it!=sessionKeyTable.end()){
        // print_debug("----------------------------After Search");
        // print_debug(sessionKey);
        // print_debug(sessionKeyTable[sessionKey].first);
        // print_debug(sessionKeyTable[sessionKey].second);
        user=  it->second.first;
    }
    return user;
}

