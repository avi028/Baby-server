#include "includes.h"
#include "def.h"

#ifndef USER_DEF_H
#define USER_DEF_H

#ifdef REFLECTION_CHECK

struct dummy_user {
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

#endif

struct UserDetails{
    std::string userName;
    REFLECT(
        ("userName",userName)
    )
};

#endif