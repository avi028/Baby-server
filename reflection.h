#include "includes.h"
#ifndef REFLECTION_H
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

#endif