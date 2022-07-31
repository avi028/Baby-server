#include "includes.h"
#include "def.h"
#include "externs.h"
#include "parsers.h"


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
