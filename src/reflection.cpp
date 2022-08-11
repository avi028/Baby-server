#include "../includes/includes.h"
#include "../includes/def.h"
#include "../includes/user_def.h"
#include "../includes/reflection.h"


callOut::callOut(){
    attribute = NULL_str;
    value = NULL_str;
}

template<typename T>
std::string callOut::getAttribute(T& myobj,std::string attribute) {
    this->attribute = attribute;
    value = NULL_str;
    myobj.reflect(*this);      
    return value;
}

template<typename T> 
callOut& callOut::operator()(const char* tag, T& myfield) {
    if(attribute != NULL_str && tag == attribute){
        this->value= myfield;
    }
    return *this;
}


#ifdef REFLECTION_CHECK

int main() {
    callOut obj;
    dummy_user u;
    
    u.name1 = "John Wick 1";
    u.name2 = "John Wick 2";
    u.name3 = "John Wick 3";
    u.name4 = "John Wick 4";
    u.name5 = "John Wick 5";
    u.dateOfRelease = "01-01-2001";

    std::cout<<"Enter the attribute"<<std::endl;
    std::string attribute=NULL_str;

    std::cin>>attribute;

    std::string value = obj.getAttribute(u,attribute);
    if(value != NULL_str)    
        std::cout<<value<<std::endl;
}

#endif