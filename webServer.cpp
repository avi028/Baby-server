#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <netdb.h>
#include <string.h> 
#include <arpa/inet.h>
#include <string>
#include <iterator>
#include <sstream>
#include <streambuf>
#include <vector>
#include <fstream>
#include <map>
#include <stdlib.h>
#include <time.h>
using namespace std;


// HTTP CODES
#define HTTP_OK 200
#define HTTP_SEE_OTHER 303
#define HTTP_FILE_NOT_FOUND 404
#define HTTP_UNAUTHORISED_ACCESS 401

// MAX LIMITS
#define MAX_WORKER_THERAD_COUNT 100
#define MAX_IMAGE_SIZE_BYTES 10000000
#define MAX_HEADER_ARG_LIST 40
#define MAX_GET_ARGS 10
#define MAX_COOKIE_COUNT 10

// Content Types
#define NONE    1 // "Error in URL"
#define ICO  2 // "ico"
#define JS  3   // "js"
#define PNG 4   // "png"
#define JPEG    5 // "jpeg"
#define JSON    6//"json"
#define CSS     7//"css"
#define HTML    8//"html"
#define HTML_GET 9
#define DEFAULT  10// "/"

// Server Types 
#define Thread_Based_Server  1
#define Process_Based_Server  2


#define NUM 1
#define CHAR 2

//Debug Tools
#define DEBUG   true
#define print_debug(x) if(DEBUG) cout<<#x<<"\t"<<x<<endl

// Global variables 
string websiteFolder;
string IP;
int PORT;
int serverType;

// required Structures
struct params{
    int client_socket;
    sockaddr_in commSock;
    socklen_t commlen;
};

/********************************************

    /index.html ?   name=john   &   pass=doe

_name
_ext
_numberOfArg
_arg1Name
_arg1Value
_arg2Name
_arg2Value
_arg3Name
_arg3Value
....
...
..
.
*********************************************/


/* Structure to store the arguments received in the client request
*/
struct requestArg{
    string name;
    string value;
};

/* URL structure to store the different sections of URL 
*/
struct urlDecode{
    string service;
    string ext;
    int numberOfArg;
    struct requestArg  cg[MAX_GET_ARGS];
    bool valid;
};


/* Type to content received in the POST method 
*/
#define form_urlencoded "application/x-www-form-urlencoded\r"
#define multipart_form_data  "multipart/form-data\r"

/* Request header structure to decode and store the different parts of the Header section of the client Request 
 */
struct reqHeader{
    string  typeOfReq;
    string  url;
    string  httpVersion;
    int reqArgCnt;
    string inputType;
    string multipartBoundary ;
    struct requestArg inputArgs[MAX_GET_ARGS];
    int inputArgsCount;
    struct requestArg headerArgList[MAX_HEADER_ARG_LIST];
    bool valid;
};

/* To Response to the client Request below mentioned things are required 
 */
struct serviceResponse{
    string reqHeader;
    string response;
    int responseType;
    int responseCode;
};

// Temp volatile databse just of checking 
map<string,string> db_table;

// Session data storage
map<string,pair<string,string> > sessionKeyTable; 

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
vector<string> str_tok(string str,char delimeter){
    vector<string> tokens;
    int start =0 ;
    string copy = str;
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

unsigned long long int getHash(string str){
    unsigned long long int hash=0;
    for (int i=0;i<str.size();i++){
        hash = hash*10+(int)(str[i])%10;
    }
    return hash;
}


/* Decodes the URL .
    Input : URL string 
    Ouput : urlDecode structure
 */
struct urlDecode  urlParser(string url){
    
    struct urlDecode ud;
    int start =0 ;
    string charSet = ".?=&";
    string copy = "";
    bool  argsEnabled = false;
    ud.valid = true;
    ud.service = "";
    ud.ext = "";
    ud.numberOfArg=0;

    // first _name of file 
    string * s = &ud.service;
    int i=0;
    while(isNumChar(url[i]) || url[i]=='/')
        *s+=url[i++];

    for (;i<url.size();i++){

        if( isNumChar(url[i]) || url[i]=='/'){
            *s+=url[i];
        }
        //.
        else if(url[i]==charSet[0]){
            ud.ext="";
            s = &ud.ext;
        }
        // ?
        else if(url[i]==charSet[1]){
            ud.numberOfArg=1;
            ud.cg[ud.numberOfArg-1].name="";
            s  = &(ud.cg[ud.numberOfArg-1].name) ;
        }
        //=
        else if(url[i]==charSet[2] && ud.cg[ud.numberOfArg-1].name!=""){
            ud.cg[ud.numberOfArg-1].value="";
            s = &(ud.cg[ud.numberOfArg-1].value);
        }
        // &
        else if(url[i]==charSet[3] && ud.cg[ud.numberOfArg-1].value!=""){
            ud.numberOfArg+=1;
            ud.cg[ud.numberOfArg-1].name="";
            s  = &(ud.cg[ud.numberOfArg-1].name) ;
        }
        else{
            ud.valid = false;
            break;
        }
    }

    print_debug(ud.service);
    print_debug(ud.ext);
    print_debug(ud.numberOfArg);
    return ud;
}


/* Returns the session key from the table 
    input : user string , key string 
    output : sessionKey string 
 */
string getsessionKey(string user,string pass){
    string sessionKey = to_string(getHash(user)+getHash(pass)+(unsigned long long int )rand()%10000);    
    sessionKeyTable.insert({sessionKey,make_pair(user,pass)});

    cout<<"----------------------------After Insertion";
    cout<<sessionKey<<" "<<sessionKeyTable[sessionKey].first<<" "<<sessionKeyTable[sessionKey].second<<endl;

    return sessionKey;
}

/* Removes the session key from the table 
    Input : sessionKey string
 */
bool removeSessionKey(string sessionKey){
    if (sessionKeyTable.erase(sessionKey)>0)
        return true;
    return false;
}


/* Finds the user in the table using the session key 
    Input : sessionKey string
    Output : username string
 */
string getUserForSessionKey(string sessionKey){    
    string user="";
    map<string,pair<string,string> >::iterator it = sessionKeyTable.find(sessionKey);
    if(it!=sessionKeyTable.end()){
        // print_debug("----------------------------After Search");
        // print_debug(sessionKey);
        // print_debug(sessionKeyTable[sessionKey].first);
        // print_debug(sessionKeyTable[sessionKey].second);
        user=  it->second.first;
    }
    return user;
}


/* Get the content type from the given filename
    Input : filename string
    Output : contentType int
 */
int parseContentType(string filename){
    
    //   ico png jpeg json js css html none
    
    int  type ; 

    if(filename == ""){
        type =  NONE;
    }
    else{
    
        vector<string> tokens = str_tok(filename,'.');
    
        if(tokens.size()>1){
            if(tokens[1]=="ico")
                type = ICO;
            else if(tokens[1]=="png")
                type = PNG;
            else if (tokens[1]=="jpeg")
                type = JPEG;
            else if(tokens[1]=="json")
                type = JSON;
            else if(tokens[1]=="js")
                type = JS;
            else if(tokens[1]=="css")
                type = CSS;
            else if(tokens[1]=="html")
                type = HTML;          
        }      
        else{
            type = NONE;
        }
        }
    return type;
}


/* Parses the resuest header from the client and pushes the cookies in the cookie set
    Input : Request-header struct reqHeader , cookie-set struct rewuestArg
    Output : status int 
 */
int  getCookies(struct reqHeader reqh, struct requestArg * cookieSet){    

    string data = "";    
    for (int i=0;i<MAX_HEADER_ARG_LIST;i++){
        if(reqh.headerArgList[i].name == "Cookie"){
            data = reqh.headerArgList[i].value;
            break;
        }
    }

    if(data == ""){
        return -1;
    }
    
    string * tmp ;
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

/* parses the client header , decodes it and fills the request header structure
    Input : clinet-request-header-data string
    Output : Request-header-decoded struct reqHeader
 */
struct reqHeader parseReqHeader(string header){
    
    struct reqHeader r;
    string temp = "";
    int s_itr=0;
    r.valid = true;

    // type of req
    while(header[s_itr]!=' '){
        r.typeOfReq+=header[s_itr++];
    }
    s_itr++;
    if(s_itr > header.size()){
        r.valid = false;
        return r;
    }
    // url 
    while(header[s_itr]!=' '){
        r.url+=header[s_itr++];
    }
    s_itr++;
    if(s_itr > header.size()){
        r.valid = false;
        return r;
    }

    // HTTP VERSION
    while(header[s_itr]!='\n'){
        r.httpVersion+=header[s_itr++];
    }
    s_itr++;
    if(s_itr > header.size()){
        r.valid = false;
        return r;
    }

    if(r.typeOfReq=="GET"){

        r.reqArgCnt=0;
        string * tmp;
        r.headerArgList[r.reqArgCnt].name="";
        tmp = &(r.headerArgList[r.reqArgCnt].name);
        r.headerArgList[r.reqArgCnt].value="";
        
        for(int i=s_itr;i<header.size();i++){
            if(header[i]==':' && r.headerArgList[r.reqArgCnt].value==""){
                i++;
                if(header.size()>i){
                    r.headerArgList[r.reqArgCnt].value="";
                    tmp = &(r.headerArgList[r.reqArgCnt].value);
                }            
            }
            if(header[i]=='\n'){
                r.reqArgCnt++;
                r.headerArgList[r.reqArgCnt].name="";
                tmp = &(r.headerArgList[r.reqArgCnt].name);
            }
            else{
                *tmp+=header[i];
            }
        }

        print_debug(r.typeOfReq);
        print_debug(r.url);
        print_debug(r.reqArgCnt);
        print_debug(r.headerArgList[r.reqArgCnt-2].name);
        print_debug(r.headerArgList[r.reqArgCnt-2].value);

    }
    else if (r.typeOfReq == "POST")
    {
        r.reqArgCnt=0;
        string * tmp;
        r.headerArgList[r.reqArgCnt].name="";
        tmp = &(r.headerArgList[r.reqArgCnt].name);
        r.headerArgList[r.reqArgCnt].value="";
        
        int i=s_itr;
        for( ;i<header.size();i++){
            if(header[i]==':' && r.headerArgList[r.reqArgCnt].value==""){
                i++;
                if(header.size()>i){
                    r.headerArgList[r.reqArgCnt].value="";
                    tmp = &(r.headerArgList[r.reqArgCnt].value);
                }            
            }
            else if(header[i]=='\n' && header[i+1]=='\r'){
                i++; //\r
                i++; //\n
                i++;
                break;                
            }
            else if(header[i]=='\n'){
                if(r.headerArgList[r.reqArgCnt].name == "Content-Type"){
                    r.inputType = r.headerArgList[r.reqArgCnt].value;
                }
                
                r.reqArgCnt++;
                r.headerArgList[r.reqArgCnt].name="";
                tmp = &(r.headerArgList[r.reqArgCnt].name);
            }
            else{
                *tmp+=header[i];
            }
        }
        r.inputArgsCount=0;
        while(header.size()>i){
            r.inputArgsCount+=1;
            tmp = &(r.inputArgs[r.inputArgsCount-1].name);
            while(header[i]!='='){
                *tmp+=header[i++];
            }
            i++;
            tmp = &(r.inputArgs[r.inputArgsCount-1].value);
            while(header[i]!='\r' && header[i]!='\n' && header[i]!='&' && header.size()>i){
                *tmp+=header[i++];
            }
            i++;
        }
    }
    return r;
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
    ifstream f("webServer.config");
    if(f.good()){
        char buffer[40000];
        f.read(buffer,40000);
        vector <string>tokens = str_tok(string(buffer),'\n');        

        for (int i=0;i<tokens.size();i++){
            if(tokens[i].find('#')==0){
                continue;
            }
            else{
                vector<string> subtokens = str_tok(tokens[i],':');
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
    }
    else{
        return 0;
    }
    return 1;
}

/* ------------------------------------------------MAIN Functions ------------------------------------------------- */

/* Sends data to the client
    Input : client-socket int , file-to-be-sent string , Response-header string
    Output : status int
 */
int sendData(int socket,string filename,string header){

    int fd = open(filename.c_str(),O_RDONLY);
    if(fd == -1){
        cerr<<filename<<"file not found\n";
        close(fd);
        return 0;
    }

    struct stat fst;
    fstat(fd,&fst);

    int fsize = fst.st_size;
    int bsize = fst.st_blksize;

    // cerr<<fsize<<endl;
    // cerr<<bsize<<endl;   
    
    // add the content size to header
    header = header +string("\r\nContent-Length: ")+ to_string(fsize) + string("\r\n\r\n");
    
    // send HEADER  to the client
    send(socket,header.c_str(),header.size(),0);

    // send content 
    while (fsize > bsize){
        sendfile(socket,fd,NULL,bsize);
        fsize-=bsize; 
    }
    sendfile(socket,fd,NULL,fsize);
    
    // close the file descriptor
    close(fd);
    return 1;
}

/*  Resolve the clinet request to the service and return the appropriate response 
    Here you can edit the code as per your website If you want a dynamic website else no need 

    Input : client-decoded-url structc urlDecode , client-Request-Header struct reqHeader 
    Output : Response-to-client struct serviceResponse
 */
struct serviceResponse resolveRequest(struct urlDecode ud,struct reqHeader requestHeader){

    // response structure to sent to the client
    struct serviceResponse sr; 

    // to collect the cookies in the request header
    struct requestArg cookieSet[MAX_COOKIE_COUNT];


    // default initialzation of the  response structure
    sr.response= "";
    sr.reqHeader = "" ; 
    sr.responseCode = HTTP_OK;

    // check if its static asset
    if(ud.ext == "js" || ud.ext == "ico" || ud.ext == "png" || ud.ext == "jpeg" || ud.ext == "css" || ud.ext == "jpg"){
        sr.response = ud.service+"."+ud.ext;
        return sr;
    }
    
    /* Test website is a simple 
        website to test session management 
        with basic login and a dashboard to 
        display your detail

        Below Code shows how the various page requests are being handled
    */


    // Check if logged in or not 
    bool loggedIn = false;
    
    int cookieCount=0;
    string user = "";
    string sessionKey = "";


    cookieCount = getCookies(requestHeader,cookieSet);
    
    /* For TestWebiste we set the session as "sessionId" and here we check for that cookie 
        in the cookies we received from the browser 
    */

    if(cookieCount >0){
        int itr =0 ;
        while(itr<MAX_COOKIE_COUNT){
            if(cookieSet[itr].name=="sessionId"){
                sessionKey  = cookieSet[itr].value;
                user = getUserForSessionKey(sessionKey);
                if(user == ""){
                    loggedIn = false;
                    break;
                }   
                else{
                    loggedIn = true;
                    break;
                }
            }
            itr++;
        }
    }

    if(loggedIn){
        if(ud.service== "/" || ud.service=="/index" || ud.service =="/register" || ud.service == "/dashboard"){
            sr.response = "/dashboard.html";
        }
        else if(ud.service == "/logout"){
            removeSessionKey(sessionKey);
            sr.response = "";
            sr.reqHeader = "Set-Cookie: sessionId="+getsessionKey(ud.cg[0].value,ud.cg[1].value) \
                            +string("\r\n") +string("Location: /index.html");
            sr.responseCode = HTTP_SEE_OTHER;
        }
        else{
            sr.responseCode = HTTP_FILE_NOT_FOUND;
        }
        return sr;
    }
    else{
        //default
        if(ud.service == "/" || ud.service == "/index"){
            sr.response = "/index.html";
        }

        // login page request
        else if(ud.service == "/login" ){

            if(ud.numberOfArg > 0){
            
                // check the args are as per the service 
                if(ud.cg[0].name=="name" && ud.cg[1].name=="pass" ){

                    // do the service 
                    if(db_table.size()>0 ){
                        print_debug(ud.cg[0].value);
                        print_debug(ud.cg[1].value);
                        map<string,string>::iterator itr = db_table.find(ud.cg[0].value);
                        if(db_table.end()!=itr){
                            if(itr->second == ud.cg[1].value){
                                sr.response =  "";
                                sr.reqHeader = "Set-Cookie: sessionId="+getsessionKey(ud.cg[0].value,ud.cg[1].value) \
                                                +string("\r\n") + string("Location: /dashboard.html") ;
                                sr.responseCode = HTTP_SEE_OTHER;

                            }
                        }
                        else{
                            sr.responseCode = HTTP_UNAUTHORISED_ACCESS;
                        }
                    }
                    else{
                        sr.responseCode = HTTP_UNAUTHORISED_ACCESS;
                    }
                }
                else{
                    sr.responseCode = HTTP_FILE_NOT_FOUND;
                }
            }
            else{
                sr.response = "/login.html";    
            }
        }
        // register page request
        else if(ud.service == "/register"){

            if(ud.numberOfArg > 0){
                // check the args are as per the service 
                if(ud.cg[0].name=="name" && ud.cg[1].name=="pass"){
                        db_table[ud.cg[0].value] = ud.cg[1].value;
                        sr.response =  "";
                        sr.reqHeader = string("Location: /login.html") ;
                        sr.responseCode = HTTP_SEE_OTHER;
                }
                else{
                    sr.responseCode = HTTP_FILE_NOT_FOUND;
                }
            }
            else{
                sr.response = "/register.html";
            }
        }
        // dashboard page request
        else if(ud.service == "/dashboard"){
            sr.responseCode = HTTP_UNAUTHORISED_ACCESS;
        }
        // If any other page request 
        else{
            sr.response = ud.service + "." + ud.ext;
        }
    }
    
    // return the response 
    return sr;
}


/* Thread handling each of the client requests
 */
void * workingThread (void *param){

    struct params * p = (struct params *) param;    
    char buffer[8096];
    string header = "" ;
    string data ="";

    int num_byte_recvd = recv(p->client_socket,buffer,8096,0);
    
    print_debug("------------------------------");

    print_debug(string(buffer,0,num_byte_recvd));
    struct reqHeader requestHeader =  parseReqHeader(string(buffer,0,num_byte_recvd));
    
    //default error code
        int httpResponseCode = HTTP_FILE_NOT_FOUND;
    
    if(requestHeader.valid){
        
        string url = requestHeader.url;
        struct stat fst;
        string requestedFile = "";
        struct serviceResponse sr;

        // parse the URL        
        struct urlDecode url_decoded= urlParser(url);

        if(requestHeader.typeOfReq=="POST" && requestHeader.inputType==form_urlencoded){
            url_decoded.numberOfArg = requestHeader.inputArgsCount;
            for (size_t i = 0; i < requestHeader.inputArgsCount; i++)
            {
                url_decoded.cg[i].name = requestHeader.inputArgs[i].name;
                url_decoded.cg[i].value = requestHeader.inputArgs[i].value;
            }
            // requestHeader.typeOfReq="GET";
        }

        // reslove the service
        if(url_decoded.valid){
            sr.reqHeader = "";
            sr = resolveRequest(url_decoded,requestHeader);
            if(sr.response != "")
                requestedFile = websiteFolder +  sr.response;
            else
                requestedFile = "";

            httpResponseCode = sr.responseCode;
        }

        print_debug(requestedFile);
 
        // check for redirect
        if(httpResponseCode == HTTP_SEE_OTHER){
            requestedFile = "";
        }

        // check if authorized access
        else if(httpResponseCode == HTTP_UNAUTHORISED_ACCESS && requestedFile==""){
            requestedFile = "Error401.html";
        }

        // check if content is available
        else if(stat(requestedFile.c_str(),&fst)!=0 || httpResponseCode == HTTP_FILE_NOT_FOUND){
            httpResponseCode =  HTTP_FILE_NOT_FOUND;
            requestedFile = "Error404.html";
        }
        // set the header as per the response 
        header = string("HTTP/1.1 ") + to_string(httpResponseCode) +string(" ok\r\n");
        int type =  parseContentType(requestedFile);    
                                           
        switch (type)
        {
            case HTML:
                header = header + string("Content-Type: text/html");
                break;
                            
            case ICO:
                header = header + string("Content-Type: image/vnd.microsoft.icon");
                break;
            
            
            case PNG:
                header = header + string("Content-Type: image/png");
                break;
            
            
            case CSS:
                header = header + string("Content-Type: text/css");
                break;
            
            
            case JPEG:
                header = header + string("Content-Type: image/jpeg");
                break;
            
            
            case JSON:
                header = header + string("Content-Type: text/json");
                break;
            
            case JS:
                header = header + string("Content-Type: text/javescript");
                break;
            
            case NONE:
                header = header + string("Content-Type: text/plain");
                break;

            default:
                header = header + string("Content-Type: text/plain");
                break;            
        }
        
        // if any extra header requirement then added
        if(sr.reqHeader !=""){
            header+= string("\r\n") + sr.reqHeader + string("\r\n");
        }

        print_debug(header);
            
        // send data to clinet
        if(httpResponseCode == HTTP_SEE_OTHER){
            header += string("Content-Length: 0") + string("\r\n\r\n");
            send(p->client_socket,header.c_str(),header.size(),0);
        }
        else{
            sendData(p->client_socket,requestedFile,header);    
        }
    }
    close(p->client_socket);
    return 0;   
}

/* Sets up the initial connection
    1. Creates the listening socket
    2. binds it with the IP address
    3. Puts the socket to listening  mode.
 */
int setupInitConnection(){

    // create a socket
    int listening_sock = socket(AF_INET,SOCK_STREAM,0);

    // port check 
    if(listening_sock==-1){
        cerr<<"Unable to open socket"<<endl;
        return -1;
    }

    sockaddr_in host_addr ;
    host_addr.sin_family = AF_INET;

    // reverse the bits of the port fron host number to netwrok number 
    host_addr.sin_port = htons(PORT);

    // add address to the sockaddr_in while converting string to ipv4 address
    inet_pton(AF_INET,IP.c_str(),&host_addr.sin_addr);

    // adding reusable tag to the port
    const int enable = 1;
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        cerr<<"setsockopt(SO_REUSEADDR) failed";    
        return -1;
    }

    // bind a socket 
    if(bind(listening_sock,(sockaddr*)&host_addr,sizeof(sockaddr_in))==-1){
        cerr<<"Bind unsuccessful : Unable to bond to given socket"<<endl;
        return -1;
    }

    listen(listening_sock,SOMAXCONN);
    return listening_sock;
}

/* Takes the requests from teh clients first hand 
    1. accepts the connection 
    2. creates a new thread for the request to be handled
*/
void loadBalancer(int listening_sock){
    
    // threads for processes
    pthread_t  * wt_pth = (pthread_t*) malloc(sizeof(int));
    while(1){
        struct params * commPrm = (struct params *)malloc(sizeof(struct params));
        commPrm->commlen = sizeof(sockaddr_in);
        commPrm->client_socket = accept(listening_sock,(sockaddr*)&commPrm->commSock,&commPrm->commlen);
        if (commPrm->client_socket !=-1){

            
            cout<<"Connected to client on port "<<commPrm->client_socket<<endl; 
            
            //*******************creating  thread for each request *****************
            
            if(serverType == Thread_Based_Server){
                if(pthread_create(&wt_pth[0],0,workingThread,(void*)commPrm)==0){
                    continue;
                }
                else{
                    break;
                }
            }

            // *************** trying froking for each request  ***************************
            else if (serverType == Process_Based_Server){

                int pid = fork();
                if(pid==0){
                    workingThread((void *)commPrm);
                    // cout<<"------------child process starts----------------------"<<endl;
                    close(commPrm->client_socket);
                    // cout<<"------------child process ends----------------------"<<endl;
                    _Exit(1);
                }
                else{
                    // cout<<"------------parent process----------------------"<<endl;
                    close(commPrm->client_socket);
                }
            }
        }       
        else{
            cerr<<"Connection not established";
        }
    } 
}

/* Main Code */
int main(int argc,char** argcv){

    // random initialization for hashMap
    srand(time(NULL));

    // lOAD Config data from file
    loadConfigFile();

    // listen on the socket
    int listening_sock = setupInitConnection();

    // load balancer call
    loadBalancer(listening_sock);

    // close listening socket
    close (listening_sock);
    return 0;
}