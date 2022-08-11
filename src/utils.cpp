#include "../includes/utils.h"
#include "../includes/def.h"
#include "../includes/structDef.h"
#include "../includes/externs.h"

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

/* Parses the resuest header from the client and pushes the cookies in the cookie set
    Input : Request-header struct reqHeader , cookie-set struct rewuestArg
    Output : status int 
 */
int  getCookies(struct reqHeader reqh, struct requestArg * cookieSet){    

    std::string data = "";    
    for (int i=0;i<MAX_HEADER_ARG_LIST;i++){
        if(reqh.headerArgList[i].name == "Cookie"){
            data = reqh.headerArgList[i].value;
            break;
        }
    }

    if(data == ""){
        return -1;
    }
    
    std::string * tmp ;
    int argCount=0;
    
    cookieSet[argCount].name="";
    tmp = &(cookieSet[argCount].name);
    int i=0;
    while((data[i]==' ')) i++;

    for(;i<data.size();i++){
        if(data[i]==';' ){
            i++;
            while((data[i]==' ')) i++;
            i--;
            argCount++;
            cookieSet[argCount].name="";
            tmp = &(cookieSet[argCount].name);
        }
        else if(data[i]=='='){
            while((data[i]==' ')) i++;
            cookieSet[argCount].value="";
            tmp = &(cookieSet[argCount].value);
        }
        else if(data[i]=='\r'){
            argCount++;
        }
        else{
            *tmp+=data[i];
        }
    }

    return argCount;
}


/* Loads the configuration from the config file in the main folder 
*/
int loadConfigFile(){
    
    // default set 
    websiteFolder="testWebsite";
    IP="127.0.0.1";
    PORT=8080;
    serverType=Thread_Based_Server;

    // if config file is available set the vairables as per the file
    std::ifstream f("server_config.mk");
    if(f.good()){
        char buffer[40000];
        f.read(buffer,40000);
        std::vector <std::string>tokens = str_tok(std::string(buffer),'\n');        

        for (int i=0;i<tokens.size();i++){
            if(tokens[i].find('#')==0){
                continue;
            }
            else{
                std::vector<std::string> subtokens = str_tok(tokens[i],'=');
                if(subtokens[0]=="WebSiteFolderName")
                    websiteFolder = subtokens[1];
                else if(subtokens[0]=="IP")
                    IP=subtokens[1];
                else if(subtokens[0]=="PORT")
                    PORT=atoi(subtokens[1].c_str());
                else if(subtokens[0]=="Server_Type")
                    serverType=atoi(subtokens[1].c_str());
            }
        }
        std::cout<<websiteFolder<<" is running at "<<IP<<":"<<PORT<<std::endl;
        std::cout<<"You can press Ctrl+C to exit the server"<<std::endl;
    }
    else{
        return 0;
    }
    return 1;
}

