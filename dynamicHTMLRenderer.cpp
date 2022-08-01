#include "includes.h"
#include "externs.h"
#include "user_def.h"
#include "reflection.h"
#include "dynamicHTMLRenderer.h"


/**
 * @brief get the attributes and their location in the read vector of strings
 * 
 */

std::vector< std::pair<std::string,int> > getAttributes(std::vector<std::string> lineset){
    std::vector< std::pair<std::string,int> > att_set;

    return att_set;
}


/**
 * @brief 
 * @param User-Object-Instance contains details of the user
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

    // call get Attributes 


}