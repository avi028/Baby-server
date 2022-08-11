#include "includes.h"
#ifndef REFLECTION_H
    #include "def.h"
    #include "user_def.h"

    #define REFLECTION_H
    /**
     * @brief Call out class is the base class to reead write the variables of structs defined in the user.def
     * 
     * 
     */
    class callOut {
        
        std::string attribute;
        std::string value;

        public:

        callOut();

        template<typename T>
        std::string getAttribute(T& myobj,std::string attribute);
        
        template<typename T> 
        callOut& operator()(const char* tag, T& myfield);
    };
    
    /**
     * @brief Construct a new call Out::call Out object
     * 
     */
    callOut::callOut(){
        attribute = NULL_str;
        value = NULL_str;
    }

    /**
     * @brief gets the required attribute from the given structure
     * 
     * @tparam T 
     * @param myobj 
     * @param attribute 
     * @return std::string 
     */
    template<typename T>
    std::string callOut::getAttribute(T& myobj,std::string attribute) {
        this->attribute = attribute;
        value = NULL_str;
        myobj.reflect(*this);      
        return value;
    }

    /**
     * @brief Handles the '()' reflect operator
     * 
     * @tparam T 
     * @param tag 
     * @param myfield 
     * @return callOut& 
     */
    template<typename T> 
    callOut& callOut::operator()(const char* tag, T& myfield) {
        if(attribute != NULL_str && tag == attribute){
            this->value= myfield;
        }
        return *this;
    }

#endif