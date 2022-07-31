#include "utils.h"
#include "def.h"

/* ----------------------------------------------Utility Fuctions---------------------------------------------- */

/*  Check If the given character is numer or character or none
*/
int isNumChar(char c){
    if(48 <= c && c<=57)
        return NUM;
    if((65 <= c && c<=90) || (97<=c && c<=122))
        return CHAR;
    return 0;
}

/*  Takes inout as atring and splits it as per the given delimeter .
    Input : str string , char delimeter 
    Ouput : vector  <string>  
 */
std::vector<std::string> str_tok(std::string str,char delimeter){
    std::vector<std::string> tokens;
    int start =0 ;
    std::string copy = str;
    int end = copy.find(delimeter);
    while(end !=-1){
        tokens.push_back(copy.substr(start,end-start));
        start = end+1;
        end = copy.find(delimeter,start);
    }   
    tokens.push_back(copy.substr(start,end-start));
    return tokens;
}

/* Simple hash function to create a numeric hash of a given string 
    Input : string to br hashed
    Output : unsigned int
 */

unsigned long long int getHash(std::string str){
    unsigned long long int hash=0;
    for (int i=0;i<str.size();i++){
        hash = hash*10+(int)(str[i])%10;
    }
    return hash;
}
