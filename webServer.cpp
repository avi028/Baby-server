#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
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

struct params{
    int client_socket;
    sockaddr_in commSock;
    socklen_t commlen;
};

void * workingThread (void *param){
    struct params * p = (struct params *) param;
    
    char buffer[8096];

    int num_byte_recvd = recv(p->client_socket,buffer,8096,0);
    string requestMsg = string(buffer,0,num_byte_recvd);
    cout<<requestMsg;

    // send the webpage data to the client 
    // with http header info

    string header ;
    string data ;
    
    // get the file name to transfer 
    istringstream issRequestMsg(requestMsg);

    vector<string> parsed ((istream_iterator<string>(issRequestMsg)),istream_iterator<string>()) ;
    // for(int i=0;i<parsed.size();i++){
    //     cout<<parsed[i]<<endl;
    // }

    //default header, data and error code
    int httpErrorCode = 404;
    data = string("<head></head> <body> <h1>File Not Found </h1></body> ");
    header = string("HTTP/1.1 ") + to_string(httpErrorCode) +string(" ok\r\n") + string("Cache-Control: no-cache, private\r\n") \
                + string("Content-Type: text/html\r\n")+ string("Content-Length: ") +to_string(data.size()) + string("\r\n\r\n"); 

    if(parsed.size()>=3 && parsed[0]=="GET"){

        // read data from file 
        if(parsed[1]=="/favicon.ico"){
            
            cout << " asked for Favicon"<<endl;

            // look for image favicon.ico        
            ifstream f("testWebsite/favicon.ico",ifstream::binary);

            //IF found send
            if(f.good()){
                httpErrorCode = 200;
                char *imgBuffer = (char *)malloc(sizeof(char)*MAX_IMAGE_SIZE_BYTES);
            
                // read image as data 
                f.read(imgBuffer,sizeof(char)*MAX_IMAGE_SIZE_BYTES);
                int imgSize = (int)f.gcount();
                cout<<"favicon size"<<imgSize<<endl;
                // formulation of data and header 
                data = string(imgBuffer);
                header = string("HTTP/1.1 ") + to_string(httpErrorCode) +string(" ok\r\n") + string("Cache-Control: no-cache, private\r\n") \
                            + string("Content-Type: image/icon\r\n")+ string("Content-Length: ") +to_string(imgSize) + string("\r\n\r\n");             
                free(imgBuffer);
            }
            else{
                cerr<<"Favicon.ico not found"<<endl;
            }
            f.close();
        }
        else{
            std::ifstream f ("testWebsite"+parsed[1]);
            if(f.good()){
    //            cout <<string("testWebsite")+parsed[1]<<endl;    
                httpErrorCode = 200;
                data  = string(istreambuf_iterator<char>(f),istreambuf_iterator<char>());
                header = string("HTTP/1.1 ") + to_string(httpErrorCode) +string(" ok\r\n") + string("Cache-Control: no-cache, private\r\n") \
                            + string("Content-Type: text/html\r\n")+ string("Content-Length: ") +to_string(data.size()) + string("\r\n\r\n"); 
            }
            f.close();   
        }
    }
    
    // add header to data
    data  = header + data;

    // send data to the client
    send(p->client_socket,data.c_str(),data.size(),0);

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
    host_addr.sin_port = htons(8080);

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

    while(wt_itr < MAX_WORKER_THERAD_COUNT){
        struct params * commPrm = (struct params *)malloc(sizeof(struct params));
        commPrm->commlen = sizeof(sockaddr_in);
        commPrm->client_socket = accept(listening_sock,(sockaddr*)&commPrm->commSock,&commPrm->commlen);
        if (commPrm->client_socket !=-1){
            cout<<"Connected to client on port "<<commPrm->client_socket<<endl; 
            wt_tid[wt_itr]=pthread_create(&wt_pth[wt_itr],0,workingThread,(void*)commPrm);
            wt_itr++;
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