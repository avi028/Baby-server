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

struct params{
    int client_socket;
    sockaddr_in commSock;
    socklen_t commlen;
};


int sendData(int socket,string filename,string header){
    struct stat fst;
    int fd = open(filename.c_str(),O_RDONLY);
    if(fd == -1){
        close(fd);
        return 0;
    }

    fstat(fd,&fst);
    int fsize = fst.st_size;
    int bsize = fst.st_blksize;
    cerr<<fsize<<endl;
    cerr<<bsize<<endl;   
    header = header +to_string(fsize) + string("\r\n\r\n");
    // send HEADER  to the client
    send(socket,header.c_str(),header.size(),0);

    while (fsize > bsize){
        sendfile(socket,fd,NULL,bsize);
        fsize-=bsize;
    }
    sendfile(socket,fd,NULL,fsize);
    
    close(fd);
    return 1;
}

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
        if(parsed[1]=="/favicon.png"){
            
            cout << " asked for Favicon"<<endl;

            // look for image favicon.ico        
            ifstream f("testWebsite/favicon.png",ifstream::binary);

            //IF found send
            if(f.good()){
                f.close();
                httpErrorCode = 200;
                header = string("HTTP/1.1 ")    + to_string(httpErrorCode) +string(" ok\r\n") \
                                             //   + string("Cache-Control: no-cache, private\r\n") 
                                                + string("Content-Type: image/png\r\n")\
                                                + string("Content-Length: ");             
                sendData(p->client_socket,string("testWebsite"+parsed[1]),header);
            }
            else{
                f.close();
                cerr<<"Favicon.ico not found"<<endl;
            }
        }
        else{
            std::ifstream f ("testWebsite"+parsed[1]);
            if(f.good()){
    //            cout <<string("testWebsite")+parsed[1]<<endl;    
                httpErrorCode = 200;
                header = string("HTTP/1.1 ") + to_string(httpErrorCode) +string(" ok\r\n") + string("Cache-Control: no-cache, private\r\n") \
                            + string("Content-Type: text/html\r\n")+ string("Content-Length: "); 
                sendData(p->client_socket,string("testWebsite"+parsed[1]),header);
            }
            f.close();   
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
    host_addr.sin_port = htons(8080);

    // add address to the sockaddr_in while converting string to ipv4 address
    inet_pton(AF_INET,"127.0.0.1",&host_addr.sin_addr);

    // const int enable = 1;
    // if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    //     cerr<<"setsockopt(SO_REUSEADDR) failed";    

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
            wt_tid[wt_itr]=pthread_create(&wt_pth[wt_itr],0,workingThread,(void*)commPrm);
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