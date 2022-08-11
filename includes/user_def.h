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

/**
 * @brief do define custom structures to be used for variables in dynamic rendering as follows

struct <any-name-of-choice>{
 *  <var1-type> <var1-name>; 
 *  <var2-type> <var2-name>; 
 *  ....
 *  ...
 *  ...
 *  ...
 * 
 *  REFLECT(
 *      ("<var1-name>" , <var1-name>)
 *      ("<var2-name>" , <var2-name>)
 *      ("<var3-name>" , <var3-name>)
 *      ("<var4-name>" , <var4-name>)
 * )
 }
 */

/**
 * @brief User details structure to store the details required 
 * by the testWebsite. 
 * 
 */
struct UserDetails{
    std::string userName;
    REFLECT(
        ("userName",userName)
    )
};

#endif