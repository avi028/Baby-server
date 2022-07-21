#include <iostream>
#include <string>

#define REFLECT(x) template<typename R> void reflect(R& r) { r x; }
#define NULL_str ""

struct user {
    std::string name1;
    std::string name2;
    std::string name3;
    std::string name4;
    std::string name5;
    std::string dateOfRelease ;

    REFLECT(
        ("dateOfRelease",dateOfRelease)
        ("name3",name3)
        ("name4",name4)
        ("name2",name2)
        ("name1",name1)
        ("name5",name5)
    )
};

class callOut {
    
    std::string attribute;
    std::string value;

    public:

    callOut(){
        attribute = NULL_str;
        value = NULL_str;
    }

    template<typename T>
    std::string getAttribute(T& myobj,std::string attribute) {
        this->attribute = attribute;
        value = NULL_str;
        myobj.reflect(*this);      
        return value;
    }

    template<typename T> 
    callOut& operator()(const char* tag, T& myfield) {
        if(attribute != NULL_str && tag == attribute){
            this->value= myfield;
        }
        return *this;
    }
};

int main() {
    callOut obj;
    user u;
    
    u.name1 = "John Wick 1";
    u.name2 = "John Wick 2";
    u.name3 = "John Wick 3";
    u.name4 = "John Wick 4";
    u.name5 = "John Wick 5";
    u.dateOfRelease = "01-01-2001";

    std::string attribute = "name4";
    std::string value = obj.getAttribute(u,attribute);
    if(value != NULL_str)    
        std::cout<<value<<std::endl;
}