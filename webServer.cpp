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


#define MAX_WORKER_THERAD_COUNT 100
#define MAX_IMAGE_SIZE_BYTES 10000000

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


#define MAX_GET_ARGS 10
#define NUM 1
#define CHAR 2

#define DEBUG   true
#define print_debug(x) if(DEBUG) cout<<#x<<"\t"<<x<<endl

// Global variables 
string websiteFolder;
string IP;
int PORT;
int serverType;


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

struct requestArg{
    string name;
    string value;
};

struct urlDecode{
string service;
string ext;
int numberOfArg;
struct requestArg  cg[MAX_GET_ARGS];
bool valid;
};

struct serviceResponse{
    string reqHeader;
    string response;
    int responseType;
};

map<string,string> db_table;

int isNumChar(char c){
    if(48 <= c && c<=57)
        return NUM;
    if((65 <= c && c<=90) || (97<=c && c<=122))
        return CHAR;
    return 0;
}

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
    print_debug(ud.cg[0].name);
    print_debug(ud.cg[0].value);
    print_debug(ud.cg[1].name);
    print_debug(ud.cg[1].value);
    print_debug(ud.numberOfArg);
    return ud;
}


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

map<string,pair<string,string> > sessionKeyTable; 

unsigned long long int getHash(string str){
    unsigned long long int hash=0;
    for (int i=0;i<str.size();i++){
        hash = hash*10+(int)(str[i])%10;
    }
    return hash;
}

string getsessionKey(string user,string pass){
    string sessionKey = to_string(getHash(user)+getHash(pass)+(unsigned long long int )rand()%10000);    
    sessionKeyTable.insert({sessionKey,make_pair(user,pass)});
    return sessionKey;
}

string getUserForSessionKey(string sessionKey){    
    map<string,pair<string,string> >::iterator it = sessionKeyTable.find(sessionKey);
    if(it!=sessionKeyTable.end()){
        return it->second.first;
    }
    return NULL;
}


int parseContentType(string filename){
    //   ico png jpeg json js css html none
    vector<string> tokens = str_tok(filename,'.');

    if(tokens[1]=="ico")
        return ICO;
    else if(tokens[1]=="png")
        return PNG;
    else if (tokens[1]=="jpeg")
        return JPEG;
    else if(tokens[1]=="json")
        return JSON;
    else if(tokens[1]=="js")
        return JS;
    else if(tokens[1]=="css")
        return CSS;
    else if(tokens[1]=="html"){
        return HTML;                
    }
}

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

#define MAX_COOKIE_COUNT 10

int  getCookies(struct reqHeader reqh, struct requestArg * cookieSet){    

    string data = "";    
    for (int i=0;i<reqh.reqArgCnt;i++){
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
    
    for(int i=0;i<data.size();i++){
        if(data[i]==';'){
            i++;
            argCount++;
            cookieSet[argCount].name="";
        }
        else if(data[i]=='='){
            cookieSet[argCount].value="";
            tmp = &(cookieSet[argCount].value);
        }
        else{
            *tmp+=data[i];
        }
    }
    return argCount;
}

// resolve the clinet request to the service and return the appropriate response 
struct serviceResponse resolverequestArg(string serviceName , struct requestArg * args , int argCount,struct reqHeader requestHeader){
    struct serviceResponse sr; 
    struct requestArg cookieSet[MAX_COOKIE_COUNT];

    // check if the service exists 
    if(serviceName == "/" && argCount ==2){

        // check the args are as per the service 
        if((args[0].name=="name" || serviceName == "/index") && args[1].name=="pass" ){
            // do the service 
            int cookieCount=0;
            
            if((cookieCount = getCookies(requestHeader,&(cookieSet[0]))) >0){
                if(cookieSet[0].name=="sessionId"){
                    // if(sessionKeyTable.find(cookieSet[0].value)){
                    //     // perform certain process 
                    // }
                }
            }

            if(db_table.size()>0 && (db_table[args[0].value]==args[1].value)){
                sr.response =  "/dashboard.html";
                sr.reqHeader = "Set-Cookie: sessionId="+getsessionKey(args[0].value,args[1].value) ; 
                sr.responseType = HTML;
            }
            else{
                sr.response =  "/unauthoreised.html";
                sr.reqHeader = " " ; 
                sr.responseType = HTML;
            }
        }
        else{
            sr.responseType = NONE;
        }
    }
    else if(serviceName == "/register" && argCount == 2){

        // check the args are as per the service 
        if(args[0].name=="name" && args[1].name=="pass"){
                db_table[args[0].value] = args[1].value;
                sr.response =  "/dashboard.html";
                sr.reqHeader = "Set-Cookie: sessionId="+getsessionKey(args[0].value,args[1].value) ; 
                sr.responseType = HTML;
        }
        else{
            sr.responseType = NONE;
        }
    }
    // return the response 
    return sr;
}

// GET / HTTP/1.1
// Host: 127.0.0.1:8080
// Connection: keep-alive
// sec-ch-ua: "Chromium";v="103", ".Not/A)Brand";v="99"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "Linux"
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.5060.53 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br
// Accept-Language: en-GB,en;q=0.9
// Cookie: sessionId=15137

#define MAX_HEADER_ARG_LIST 40
struct reqHeader{
    string  typeOfReq;
    string  service;
    string  httpVersion;
    int reqArgCnt;
    struct requestArg headerArgList[MAX_HEADER_ARG_LIST];
    bool valid;
};

struct reqHeader parseReqHeder(string header){
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
    // service 
    while(header[s_itr]!=' '){
        r.service+=header[s_itr++];
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
    print_debug(r.httpVersion); 
    print_debug(r.service);
    print_debug(r.reqArgCnt);
    print_debug(r.headerArgList[0].name);
    print_debug(r.headerArgList[0].value);
    print_debug(r.headerArgList[r.reqArgCnt-2].name);
    print_debug(r.headerArgList[r.reqArgCnt-2].value);
    return r;
}

void * workingThread (void *param){

    struct params * p = (struct params *) param;
    
    char buffer[8096];

    int num_byte_recvd = recv(p->client_socket,buffer,8096,0);

    struct reqHeader requestHeader =  parseReqHeder(string(buffer,0,num_byte_recvd));
    string requestMsg = string(buffer,0,num_byte_recvd);
    // cout<<requestMsg ;

    string header ;
    string data ;
    
    // get the file name to transfer 
    istringstream issRequestMsg(requestMsg);

    vector<string> parsed ((istream_iterator<string>(issRequestMsg)),istream_iterator<string>()) ;

    //default header, data and error code
    int httpErrorCode = 404;
    data = string("<head></head> <body> <h1>File Not Found </h1></body> ");
    header = string("HTTP/1.1 ") + to_string(httpErrorCode) +string(" ok\r\n") + string("Cache-Control: no-cache, private\r\n") \
                + string("Content-Type: text/html\r\n")+ string("Content-Length: ") +to_string(data.size()) + string("\r\n\r\n"); 

    if(requestHeader.valid && requestHeader.typeOfReq=="GET"){
        
        string url = requestHeader.service;
        struct stat fst;
        string requestedFile = "";
        struct serviceResponse sr;

        // parse the URL        
        struct urlDecode ud= urlParser(url);

        if(ud.valid){
            
            sr.reqHeader = "";

            if(ud.numberOfArg>0){
                sr = resolverequestArg(ud.service,ud.cg, ud.numberOfArg,requestHeader);
                requestedFile = websiteFolder +  sr.response;
            }
            else if(ud.service=="/"){
                requestedFile = websiteFolder + "/index.html";
            }
            else {
                requestedFile = websiteFolder+ud.service+"."+ud.ext;
            }
        }

        // check if content is available
        if(stat(requestedFile.c_str(),&fst)==0 && ud.valid){

            httpErrorCode = 200;
            header = string("HTTP/1.1 ") + to_string(httpErrorCode) +string(" ok\r\n");

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
                
                default:
                    header = header + string("Content-Type: text/plain");
                    break;
            }
            
            if(sr.reqHeader !=""){
                header+= string("\r\n") + sr.reqHeader;
            }  
            print_debug(header);

            sendData(p->client_socket,requestedFile,header);    
        }
        else{
            data = header + data;
            send(p->client_socket,data.c_str(),data.size(),0);
        }
    }
    close(p->client_socket);
    return 0;   
}

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

int main(int argc,char** argcv){

    // random initialization for hashMap
    srand(time(NULL));

    // lOAD Config data from file
    loadConfigFile();

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
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        cerr<<"setsockopt(SO_REUSEADDR) failed";    

    // bind a socket 
    if(bind(listening_sock,(sockaddr*)&host_addr,sizeof(sockaddr_in))==-1){
        cerr<<"Bind unsuccessful : Unable to bond to given socket"<<endl;
    }

    // listen on the socket
    listen(listening_sock,SOMAXCONN);

    pthread_t  * wt_pth = (pthread_t*) malloc(sizeof(int));

    int wt_itr =0;

    while(1){
        struct params * commPrm = (struct params *)malloc(sizeof(struct params));
        commPrm->commlen = sizeof(sockaddr_in);
        commPrm->client_socket = accept(listening_sock,(sockaddr*)&commPrm->commSock,&commPrm->commlen);
        if (commPrm->client_socket !=-1){

            
            cout<<"Connected to client on port "<<commPrm->client_socket<<endl; 
            
            //*******************creating  thread for each request *****************
            
            if(serverType == Thread_Based_Server){
                if(pthread_create(&wt_pth[wt_itr],0,workingThread,(void*)commPrm)==0){
                    wt_itr++;
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
   
    for (int i;i<wt_itr;i++)   
        pthread_join(wt_pth[i],0);

    // close listening socket
    close (listening_sock);
    return 0;
}