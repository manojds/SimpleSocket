/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimpleSocketExceptions.h
 * Author: Manoj De Silva
 *
 * Created on December 1, 2018, 11:19 AM
 */

#ifndef SIMPLESOCKETEXCEPTIONS_H
#define SIMPLESOCKETEXCEPTIONS_H

#include <exception>
#include <string>


namespace network_utils
{
    class SimpleSocketException : public std::exception
    {
    public:
        SimpleSocketException(SimpleSocketErrCodes error_code, const char* what_arg):
            exception(),
            error(error_code),
            err_str(what_arg)
                {}
        virtual ~SimpleSocketException(){}
        
        SimpleSocketErrCodes getErrorCode(){
            return error;
        }
        
        virtual const char* what() const  noexcept
        {
            return err_str.c_str();
        }
    private:
        SimpleSocketErrCodes error;
        std::string err_str;
    };
}

#endif /* SIMPLESOCKETEXCEPTIONS_H */

