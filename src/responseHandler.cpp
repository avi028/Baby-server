#include "../includes/includes.h"

#include "../includes/def.h"
#include "../includes/utils.h"
#include "../includes/structDef.h"
#include "../includes/user_def.h"
#include "../includes/externs.h"
#include "../includes/sessions.h"
#include "../includes/parsers.h"
#include "../includes/dynamicHTMLRenderer.h"

/*   */

/**
 * @brief User defined function to handle the requests from the website 
    Resolve the clinet request to the service and return the appropriate response 
    Here you can edit the code as per your website If you want a dynamic website else no need 

    Input : client-decoded-url structc urlDecode , client-Request-Header struct reqHeader 
    Output : Response-to-client struct serviceResponse

 * @param ud 
 * @param requestHeader 
 * @return struct serviceResponse 
 */

struct serviceResponse resolveRequest(struct urlDecode ud,struct reqHeader requestHeader){

    // response structure to sent to the client
    struct serviceResponse sr; 

    // to collect the cookies in the request header
    struct requestArg cookieSet[MAX_COOKIE_COUNT];


    // default initialzation of the  response structure
    sr.response= NULL_str;
    sr.responseHeader = NULL_str ; 
    sr.responseCode = HTTP_OK;

    // check if its static asset
    if(ud.ext == "js" || ud.ext == "ico" || ud.ext == "png" || ud.ext == "jpeg" || ud.ext == "css" || ud.ext == "jpg"){
        sr.response = ud.service+"."+ud.ext;
        return sr;
    }
    
    /**
     *  Test website is a simple website to test session management 
        with basic login and a dashboard to display your detail.
        Below Code shows how the various page requests are being handled.
    */

    int cookieCount=0;
    std::string  sessionKey = NULL_str;

    // Check if logged in or not 
    bool loggedIn = false;

    /**
     * @brief Details of the user for the test website is stored in the UserDetails structure defined 
     * in the 'user_def.h' file. 
     * 
     */
    struct UserDetails user;

    /**
     * @brief Handling cookies in the website 
     * getCookies returns the cookies count and the cookie-set in the cookieSet structure
       
        For TestWebiste we set the session as "sessionId" and here we check for that cookie 
        in the cookies we received from the browser 
    */
    cookieCount = getCookies(requestHeader,cookieSet);
    if(cookieCount >0){
        int itr =0 ;
        std::string  uname;
        while(itr<MAX_COOKIE_COUNT){
            if(cookieSet[itr].name=="sessionId"){
                sessionKey  = cookieSet[itr].value;
                /**
                 * @brief getUserForSessionKey is a user defiend function in 
                 * 'session.cpp' which can be modified as per the need 
                 * 
                 */
                uname = getUserForSessionKey(sessionKey);
                if(uname == NULL_str){
                    loggedIn = false;
                    break;
                }   
                else{
                    loggedIn = true;
                    user.userName = uname;
                    break;
                }
            }
            itr++;
        }
    }

    if(loggedIn){
        if(ud.service== "/" || ud.service=="/index" || ud.service =="/register" || ud.service == "/dashboard"){
            /**
             * dynamic response for dashboard    
             * @brief make response -> dynamic renderd file location .
             *  response = call dynamicHTMLrender('requested FIle',user) 
             */
            sr.response = "/dashboard.html";
            sr.response =  dyHTMLRender<UserDetails>(user,sr.response);
        }
        else if(ud.service == "/logout"){
            removeSessionKey(sessionKey);
            /**
             * @brief This is how we can set cookies my adding 
             * ' Set-cookie: <cookie-name>=<cookie-value>' as string 
             *   to the response header of srevice response structure
             */
            sr.response = NULL_str;
            sr.responseHeader = "Set-Cookie: sessionId="+getsessionKey(ud.cg[0].value,ud.cg[1].value) \
                            +std::string ("\r\n") +std::string ("Location: /index.html");
            /**
             * @brief HTTP_SEE_OTHER is the redirect code sent to the client in this case browser 
             * to ask for resourse as defined in the response header as 
             * ' Location: <resource to be asked for by the browser>
             * 
             */
            sr.responseCode = HTTP_SEE_OTHER;
        }
        else{
            sr.responseCode = HTTP_FILE_NOT_FOUND;
        }
        return sr;
    }
    else{
        /**
         * @brief index.html is set here as the default response to the testWebsite
         * 
         */
        if(ud.service == "/" || ud.service == "/index"){
            sr.response = "/index.html";
        }

        // login page request
        else if(ud.service == "/login" ){

            if(ud.numberOfArg > 0){
            
                // check the args are as per the service 
                if(ud.cg[0].name=="name" && ud.cg[1].name=="pass" ){
                    if(db_table.size()>0 ){
                        std::map<std::string ,std::string >::iterator itr = db_table.find(ud.cg[0].value);
                        if(db_table.end()!=itr){
                            if(itr->second == ud.cg[1].value){
                                sr.response =  NULL_str;
                                /**
                                 * @brief If the user is logged in redierct to the dashboard file. 
                                 *  Sample code to redirect from one page to another 
                                 *  "HTTP_SEE_OTHER" is the resposne code for redirection as per HTTP standards.
                                 */ 
                                sr.responseHeader = "Set-Cookie: sessionId="+getsessionKey(ud.cg[0].value,ud.cg[1].value) \
                                                +std::string ("\r\n") + std::string ("Location: /dashboard.html") ;

                                sr.responseCode = HTTP_SEE_OTHER;

                            }
                        }
                        else{
                            /**
                             * @brief If the user is trying to acces the page through other means 
                             * 
                             */
                            sr.responseCode = HTTP_UNAUTHORISED_ACCESS;
                        }
                    }
                    else{
                        sr.responseCode = HTTP_UNAUTHORISED_ACCESS;
                    }
                }
                else{
                    sr.responseCode = HTTP_FILE_NOT_FOUND;
                }
            }
            else{
                sr.response = "/login.html";    
            }
        }
        // register page request
        else if(ud.service == "/register"){

            if(ud.numberOfArg > 0){
                // check the args are as per the service 
                if(ud.cg[0].name=="name" && ud.cg[1].name=="pass"){
                        db_table[ud.cg[0].value] = ud.cg[1].value;
                        sr.response =  NULL_str;
                        sr.responseHeader = std::string ("Location: /login.html") ;
                        sr.responseCode = HTTP_SEE_OTHER;
                }
                else{
                    sr.responseCode = HTTP_FILE_NOT_FOUND;
                }
            }
            else{
                sr.response = "/register.html";
            }
        }
        // dashboard page request
        else if(ud.service == "/dashboard"){
            sr.responseCode = HTTP_UNAUTHORISED_ACCESS;
        }
        // If any other page request 
        else{
            sr.response = ud.service + "." + ud.ext;
        }
    }

    // return the service response structure.
    return sr;
}
