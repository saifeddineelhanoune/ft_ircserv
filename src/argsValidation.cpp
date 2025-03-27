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
    if (val <= 0 || val > 65535)
    {
        Logger::error("Port is not valid. Must be between 1 and 65535");
        return false;
    }
        
    _tport = static_cast<int>(val);
    return true;
}

bool IsValidPass(std::string _pass, std::string& passwd)
{    
    if (_pass.length() < 8)
    {
        Logger::error("Password is too short (minimum 8 characters)");
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
        Logger::error("Password does not meet complexity requirements:");
        Logger::error("- At least 8 characters");
        Logger::error("- At least 1 lowercase letter (a-z)");
        Logger::error("- At least 1 uppercase letter (A-Z)");
        Logger::error("- At least 1 digit (0-9)");
        Logger::error("- At least 1 special character");
        return false;
    }
        
    passwd = _pass;
    return true;
}
