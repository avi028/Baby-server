#include "includes.h"
#include "def.h"
#include "externs.h"
#include "utils.h"
#include "parsers.h"

/* Decodes the URL .
    Input : URL std::string 
    Ouput : urlDecode structure
 */
struct urlDecode urlParser(std::string url){


    struct urlDecode ud;
    int start =0 ;
    std::string charSet = ".?=&";
    std::string copy = "";
    bool  argsEnabled = false;
    ud.valid = true;
    ud.service = "";
    ud.ext = "";
    ud.numberOfArg=0;

    // first _name of file 
    std::string * s = &ud.service;
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

/* Get the content type from the given filename
    Input : filename std::string
    Output : contentType int
 */
int parseContentType(std::string filename){
    
    //   ico png jpeg json js css html none
    
    int  type ; 

    if(filename == ""){
        type =  NONE;
    }
    else{
    
        std::vector<std::string> tokens = str_tok(filename,'.');
    
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

/* parses the client header , decodes it and fills the request header structure
    Input : clinet-request-header-data std::string
    Output : Request-header-decoded struct reqHeader
 */
struct reqHeader parseReqHeader(std::string header){
    
    struct reqHeader r;
    std::string temp = "";
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
        std::string * tmp;
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
        std::string * tmp;
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

template<typename T>
int parseTemplate(std::string htmlTemplate, T& dataStruct){
    std::ifstream f(htmlTemplate);
    if(f.good()){
        std::string htmlTemplateData = std::string(std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>());
        print_debug(htmlTemplateData);
        // std::vector<std::string> str_set = str_tok()
    }

    return -1;
}

