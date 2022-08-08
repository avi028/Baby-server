#include "includes.h"
#include "externs.h"
#include "utils.h"
#include "user_def.h"
#include "reflection.h"
#include "dynamicHTMLRenderer.h"

#define vp_str std::vector< std::pair<std::string,int> >
/**
 * @brief Set the Attributes in the file
 * 
 * @param data 
 * @return std::string 
 */
std::string setAttributes(std::string data){
    std::string rData = NULL_str;
    int dataSize=data.size();
    std::string attr=NULL_str;

    for (size_t i = 0; i < dataSize; i++)
    {
        if(data[i]=='{' && (i+1)<dataSize && data[i+1]=='%'){
            while(data[i++]==' ' && i<dataSize);
            while(data[i]!=' ' && data[i]!='%' && i<dataSize){
                attr+=data[i++];
            }
            if(i<dataSize && data[i]=='%' && (i+1)<dataSize && data[i+1]=='}'){
                // attribute is valid use for  info
            }
        }
    }
    
    return rData;
}


/**
 * @brief 
 * @param Object_Instance contains details of the user
 * @param htmlFileName the file which can contian attributes
 * 
 * @returns filename of the new changed file
 */

template<typename T>
std::string dyHTMLRender (T& objInstance , std::string htmlfile){

    /*
    * Temprary folder for storing the rendered webpages.
    */
    std::string tmpFolder= websiteFolder + "/tmp" ; 
    struct stat folderst;
    if(stat(tmpFolder,&folderst)==-1){
        mkdir(tmpFolder,0700);

    }
    std::ifstream f(htmlfile);
    if(f.good()){
        std::string data  ((std::istreambuf_iterator<char>(f)) ,(std::istreambuf_iterator<char>())) ;    
        data = setAttributes(data);
        // call get Attributes 
    }
    else
        return NULL_str;

}