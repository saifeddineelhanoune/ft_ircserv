#include "../include/server.hpp"

bool isStringDigits(std::string str)
{
    for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    {
        if (!isdigit(*it)) return false;
    }
    return true;
}
bool IsValidPort(long val, int& _tport)
{
    
    if ( val <= 0 || val > 65535)
    {
        std::cerr << "Error: Port is not valid" << std::endl;
        return false;
    }
        
    _tport = static_cast<int>(val);
    return true;
}

bool IsValidPass(std::string _pass, std::string& passwd)
{    
    if (_pass.length() < 8)
    {
        std::cerr << "Error: Password is too short" << std::endl;
        return false;
    }

    bool Lower = false, Upper = false, Digit = false, Special = false;
    
    for (std::string::iterator it = _pass.begin(); it != _pass.end(); ++it)
    {
        if (islower(*it)) Lower = true;
        else if (isupper(*it)) Upper = true;
        else if (isdigit(*it)) Digit = true;
        else if (ispunct(*it)) Special = true;
    }
    
    if (!(Lower && Upper && Digit && Special))
    {
        std::cerr << "Error: Password is not valid" << std::endl;
        std::cerr << "- At least 8 characters" << std::endl;
        std::cerr << "- At least 1 lowercase letter (a-z)" << std::endl;
        std::cerr << "- At least 1 uppercase letter (A-Z)" << std::endl;
        std::cerr << "- At least 1 digit (0-9)" << std::endl;
        std::cerr << "- At least 1 special character" << std::endl;
        return false;
    }
        
    passwd = _pass;
    return true;
}
