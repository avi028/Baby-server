#include "includes.h"
#include "def.h"
#include "utils.h"
#include "structDef.h"
#include "user_def.h"
#include "externs.h"
#include "globals.h"
#include "sessions.h"
#include "parsers.h"

using namespace std;

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


/* 
 * Thread handling each of the client requests
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

    // // lOAD Config data from file
    loadConfigFile();

    // // listen on the socket
    int listening_sock = setupInitConnection();

    // // load balancer call
    loadBalancer(listening_sock);

    // // close listening socket
    // close (listening_sock);
    return 0;
}