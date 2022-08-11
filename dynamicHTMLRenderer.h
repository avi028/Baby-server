
#include "includes.h"

#ifndef DYNAMIC_HTML_RENDERER_H
#define DYNAMIC_HTML_RENDERER_H

#include "externs.h"
#include "utils.h"
#include "user_def.h"
#include "reflection.h"

template<typename T>
std::string dyHTMLRender (T& objInstance , std::string htmlfile);

template<typename T>
std::string setAttributes(std::string data,T& dataStruct);

/**
 * @brief Set the Attributes in the file
 * 
 * @param data 
 * @return std::string 
 */
template<typename T>
std::string setAttributes(std::string data,T& dataStruct){
    callOut refObj;
    std::string rData = NULL_str;
    int dataSize=data.size();
    std::string attr=NULL_str;
    print_debug("-------------------- Dynamic Render Debug ----------------");
    for (size_t i = 0; i < dataSize; i++)
    {
        if(data[i]=='{' && (i+1)<dataSize && data[i+1]=='%'){
            i++;// for %
            i++;
            while(data[i]==' ' && i<dataSize)i++;
            while(data[i]!=' ' && data[i]!='%' && i<dataSize){
                print_debug(attr);
                attr=attr+data[i];
                i++;
            }
            print_debug(attr);
            
            while(data[i]==' ' && i<dataSize)i++;

            if(i<dataSize && data[i]=='%' && (i+1)<dataSize && data[i+1]=='}'){

                // attribute is valid use for  info
                print_debug(refObj.getAttribute(dataStruct,attr));
                rData = rData + refObj.getAttribute(dataStruct,attr) + " ";
            }
            i++;// for %
            i++;// for
            
        }
        else{
            rData = rData+data[i];
        }
    }
    
    return rData;
}


/**
 * @brief Renders dynamic webpages as per the file and the struct instance provided
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
    if(stat(tmpFolder.c_str(),&folderst)==-1){
        mkdir(tmpFolder.c_str(),0700);

    }

    std::string data = NULL_str;
    std::string outfile = websiteFolder +  htmlfile;
    std::ifstream f(websiteFolder +  htmlfile);

    if(f.good()){
        data = std::string((std::istreambuf_iterator<char>(f)) ,(std::istreambuf_iterator<char>())) ;    
        data = setAttributes(data,objInstance);
        f.close();
        std::ofstream fout(websiteFolder + "/tmp"+htmlfile);
        std::copy(data.begin() , data.end(), std::ostreambuf_iterator<char>(fout));
        outfile = "/tmp"+htmlfile;
    }
 
    return outfile;
}
#endif