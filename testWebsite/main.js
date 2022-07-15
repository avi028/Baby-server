function validateRegister(){
    var name  = document.getElementById("name").value;
    var pass = document.getElementById("pass").value;
    var cpass = document.getElementById("cpass").value;
    if(name ==""){
        alert("fill in the name");
    }
    else{
        if(pass == ""){
            alert("fill in the password");
        }
        else{
            if (cpass == ""){
                alert("fill in the confirm password");
            }
            else{
                if(pass != cpass) {
                    alert("password and confirm password donot match");
                }
                else{
                    return true;
                }
            }
        }
    }
    return false;
}


function validateLogin(){
    var name  = document.getElementById("name").value;
    var pass = document.getElementById("pass").value;
    if(name ==""){
        alert("fill in the name");
    }
    else{
        if(pass == ""){
            alert("fill in the password");
        }
        else{
            return true;
        }
    }
    return false;
}
