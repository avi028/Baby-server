#include <csignal>
#include <stdlib.h>
#include "../includes/includes.h"

#include "../includes/def.h"
#include "../includes/utils.h"
#include "../includes/structDef.h"
#include "../includes/externs.h"
#include "../includes/globals.h"
#include "../includes/parsers.h"

/**
 * @brief definend by user
 * 
 * @param ud 
 * @param requestHeader 
 * @return struct serviceResponse 
 */

struct serviceResponse resolveRequest(struct urlDecode ud,struct reqHeader requestHeader);


/* ------------------------------------------------MAIN Functions ------------------------------------------------- */

/* Sends data to the client
    Input : client-socket int , file-to-be-sent std::string  , Response-header std::string 
    Output : status int
 */
int sendData(int socket,std::string  filename,std::string  header){

    int fd = open(filename.c_str(),O_RDONLY);
    if(fd == -1){
        std::cerr<<filename<<"file not found\n";
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
    header = header +std::string ("\r\nContent-Length: ")+std:: to_string(fsize) + std::string ("\r\n\r\n");
    
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

/* 
 * Thread handling each of the client requests
 */
void * requestProcessor (void *param){

    struct params * p = (struct params *) param;    
    char buffer[8096];
    std::string  header = "" ;
    std::string  data ="";

    int num_byte_recvd = recv(p->client_socket,buffer,8096,0);
    
    print_debug("------------------------------");

    print_debug(std::string (buffer,0,num_byte_recvd));
    struct reqHeader requestHeader =  parseReqHeader(std::string (buffer,0,num_byte_recvd));
    
    //default error code
        int httpResponseCode = HTTP_FILE_NOT_FOUND;
    
    if(requestHeader.valid){
        
        std::string  url = requestHeader.url;
        struct stat fst;
        std::string  requestedFile = "";
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
            sr.responseHeader = "";
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
            requestedFile = "errorPages/Error401.html";
        }

        // check if content is available
        else if(stat(requestedFile.c_str(),&fst)!=0 || httpResponseCode == HTTP_FILE_NOT_FOUND){
            httpResponseCode =  HTTP_FILE_NOT_FOUND;
            requestedFile = "errorPages/Error404.html";
        }
        // set the header as per the response 
        header = std::string ("HTTP/1.1 ") + std::to_string(httpResponseCode) +std::string (" ok\r\n");
        int type =  parseContentType(requestedFile);    
                                           
        switch (type)
        {
            case HTML:
                header = header + std::string ("Content-Type: text/html");
                break;
                            
            case ICO:
                header = header + std::string ("Content-Type: image/vnd.microsoft.icon");
                break;
            
            
            case PNG:
                header = header + std::string ("Content-Type: image/png");
                break;
            
            
            case CSS:
                header = header + std::string ("Content-Type: text/css");
                break;
            
            
            case JPEG:
                header = header + std::string ("Content-Type: image/jpeg");
                break;
            
            
            case JSON:
                header = header + std::string ("Content-Type: text/json");
                break;
            
            case JS:
                header = header + std::string ("Content-Type: text/javescript");
                break;
            
            case NONE:
                header = header + std::string ("Content-Type: text/plain");
                break;

            default:
                header = header + std::string ("Content-Type: text/plain");
                break;            
        }
        
        // if any extra header requirement then added
        if(sr.responseHeader !=""){
            header+= std::string ("\r\n") + sr.responseHeader + std::string ("\r\n");
        }

        print_debug(header);
            
        // send data to clinet
        if(httpResponseCode == HTTP_SEE_OTHER){
            header += std::string ("Content-Length: 0") + std::string ("\r\n\r\n");
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
        std::cerr<<"Unable to open socket"<<std::endl;
        return -1;
    }

    sockaddr_in host_addr ;
    host_addr.sin_family = AF_INET;

    // reverse the bits of the port fron host number to netwrok number 
    host_addr.sin_port = htons(PORT);

    // add address to the sockaddr_in while converting std::string  to ipv4 address
    inet_pton(AF_INET,IP.c_str(),&host_addr.sin_addr);

    // adding reusable tag to the port
    const int enable = 1;
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        std::cerr<<"setsockopt(SO_REUSEADDR) failed";    
        return -1;
    }

    // bind a socket 
    if(bind(listening_sock,(sockaddr*)&host_addr,sizeof(sockaddr_in))==-1){
        std::cerr<<"Bind unsuccessful : Unable to bond to given socket"<<std::endl;
        return -1;
    }

    listen(listening_sock,SOMAXCONN);
    return listening_sock;
}

/* Takes the requests from teh clients first hand 
    1. accepts the connection 
    2. creates a new thread for the request to be handled
*/
void requestHander(int listening_sock){
    
    // threads for processes
    pthread_t  * wt_pth = (pthread_t*) malloc(sizeof(int));
    while(1){
        struct params * commPrm = (struct params *)malloc(sizeof(struct params));
        commPrm->commlen = sizeof(sockaddr_in);
        commPrm->client_socket = accept(listening_sock,(sockaddr*)&commPrm->commSock,&commPrm->commlen);
        if (commPrm->client_socket !=-1){

            
            std::cout<<"Connected to client on port "<<commPrm->client_socket<<std::endl; 
            
            //*******************creating  thread for each request *****************
            
            if(serverType == Thread_Based_Server){
                if(pthread_create(&wt_pth[0],0,requestProcessor,(void*)commPrm)==0){
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
                    requestProcessor((void *)commPrm);
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
            std::cerr<<"Connection not established";
        }
    } 
}

/**
 * @brief Delete temporary files and closing connection on exit.
 * 
 */
void handle_exit(){
    struct stat folderst;
    std::string tmpFolder= websiteFolder + "/tmp" ; 
    if(stat(tmpFolder.c_str(),&folderst)!=-1){
        std::cout<<"Deleting tmp files...."<<std::endl;
        std::string  tempDir = "rm  -r " + tmpFolder;
        std::
        system(tempDir.c_str());
        std::cout<<"files deleted"<<std::endl;
    }
    std::cout<<"CLosing connection ...."<<std::endl;
    close(listening_sock);
    std::cout<<"connection closed"<<std::endl;
}

/**
 * @brief Handles the exit signal . 
 * 
 * @param signal_num 
 */
void signal_handler( int signal_num ) {
    handle_exit();
    exit(signal_num);  
}

/* Main Code */
int main(int argc,char** argcv){
    // handling exit signal 
    signal(SIGINT, signal_handler);  

    // random initialization for hashMap
    srand(time(NULL));

    // // lOAD Config data from file
    loadConfigFile();

    // // listen on the socket
    listening_sock = setupInitConnection();

    // // load balancer call
    requestHander(listening_sock);

    // // close listening socket
    // close (listening_sock);
    return 0;
}