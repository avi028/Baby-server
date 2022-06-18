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

using namespace std;

#define MAX_WORKER_THERAD_COUNT 100
#define MAX_IMAGE_SIZE_BYTES 10000000

// Content Types
#define NONE    1
#define ICO  2
#define JS  3
#define PNG 4
#define JPEG    5
#define JSON    6//"json"
#define CSS     7//"css"
#define HTML    8//"html"

struct params{
    int client_socket;
    sockaddr_in commSock;
    socklen_t commlen;
};

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

int parseContentType(string filename){
    //   ico png jpeg json js css html none
    vector<string> tokens = str_tok(filename,'.');

    if (tokens.size()>1){
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
        else if(tokens[1]=="html")
            return HTML;
    }
    
    return NONE;
}

int sendData(int socket,string filename,string header){

    int fd = open(filename.c_str(),O_RDONLY);
    if(fd == -1){
        cerr<<filename<<endl;
        cerr<<"file not found\n";
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

void * workingThread (void *param){
    struct params * p = (struct params *) param;
    
    char buffer[8096];

    int num_byte_recvd = recv(p->client_socket,buffer,8096,0);
    string requestMsg = string(buffer,0,num_byte_recvd);
//    cout<<requestMsg;

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

    if(parsed.size()>=3 && parsed[0]=="GET"){
        
        string requestedFile = parsed[1];
        struct stat fst;

        // get content type request
        int type = parseContentType(requestedFile);

        if(type ==NONE){
            // default output of website 
            requestedFile = "/index.html";
            type = HTML;
        }
        requestedFile = string("testWebsite")+requestedFile;
        // check if content is available
        if(stat(requestedFile.c_str(),&fst)==0){

            httpErrorCode = 200;
            // set header as per the content
            header = string("HTTP/1.1 ")    + to_string(httpErrorCode) +string(" ok\r\n") \
                                               + string("Cache-Control: no-cache, private\r\n");
                                                        
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

            // send file with updated header
            sendData(p->client_socket,requestedFile,header);    
        }
        else{
            data = header + data ;
            send(p->client_socket,data.c_str(),data.size(),0);
        }
    }
    close(p->client_socket);
    return 0;   
}

int main(int argc,char** argcv){
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
    host_addr.sin_port = htons(8081);

    // add address to the sockaddr_in while converting string to ipv4 address
    inet_pton(AF_INET,"127.0.0.1",&host_addr.sin_addr);

    const int enable = 1;
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        cerr<<"setsockopt(SO_REUSEADDR) failed";    

    // bind a socket 
    if(bind(listening_sock,(sockaddr*)&host_addr,sizeof(sockaddr_in))==-1){
        cerr<<"Bind unsuccessful : Unable to bond to given socket"<<endl;
    }

    // listen on the socket
    listen(listening_sock,SOMAXCONN);


    int * wt_tid = (int *) malloc(sizeof(int));
    pthread_t  * wt_pth = (pthread_t*) malloc(sizeof(int));

    int wt_itr =0;

    while(1){
        struct params * commPrm = (struct params *)malloc(sizeof(struct params));
        commPrm->commlen = sizeof(sockaddr_in);
        commPrm->client_socket = accept(listening_sock,(sockaddr*)&commPrm->commSock,&commPrm->commlen);
        if (commPrm->client_socket !=-1){

            cout<<"Connected to client on port "<<commPrm->client_socket<<endl; 
            
            //*******************creating  thread for each request *****************
            
            wt_tid[wt_itr]=pthread_create(&wt_pth[wt_itr],0,workingThread,(void*)commPrm);

            // ********* trying froking for each request  ***************************

            // int pid = fork();
            // if(pid==0){
            //     workingThread((void *)commPrm);
            //     cout<<"------------child process starts----------------------"<<endl;
            //     close(commPrm->client_socket);
            //     cout<<"------------child process ends----------------------"<<endl;
            //     _Exit(1);
            // }
            // else{
            //     cout<<"------------parent process----------------------"<<endl;
            //     close(commPrm->client_socket);
            // }
        }       
        else{
            cerr<<"Connection not established";
        }
    } 

    for (int i;i<MAX_WORKER_THERAD_COUNT;i++)   
        pthread_join(wt_pth[i],0);

    // close listening socket
    close (listening_sock);
    return 0;
}