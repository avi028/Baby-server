# BABY SERVER IN C++
## Bare minimum http server created from scratch in c++.
- Supports Static and Dynamic webpages.
- To use change the `server_config.mk` file as per your need and run.
- A TestWebsite is contained that includes :
    - Both `GET` and `POST` kind of forms.
    - Basic authentication with register and login to showcase the session management using `COOKIES`.
    - A dashboard with user name appearing to show the dynamic rendering.
    - `responseHandler.cpp` contains the example code on how to respond to `GET` and `POST` requests and also how to render dynamic webpages.
- To use run the `make` file and then `make run` 
- Currently tested only on linux.
- Default IP and port is `127.0.0.1:8080`

## Further additions done \ pending : 
 1. Session management -- `DONE`
 2. POST method Implementation -- `DONE`
 3. Dynamic webpages support added using reflection mechanism -- `DONE`
 4. ### Database Connectivity Support -- `In Progress`

