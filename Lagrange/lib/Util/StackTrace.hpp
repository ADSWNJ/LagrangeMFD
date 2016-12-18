/*
Modified BSD License

Standard C++ stack trace functionality
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2012, Szymon "Enjo" Ender
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL SZYMON ENDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STACKTRACE_H_INCLUDED
#define STACKTRACE_H_INCLUDED

#include <string>
#include <sstream>

#ifdef __GNUC__ // GCC has __PRETTY_FUNCTION__
    #define FUNCTION_DESCR __PRETTY_FUNCTION__
#elif defined _MSC_VER  // MSVC++ has __FUNCTION__
    #define FUNCTION_DESCR __FUNCTION__
#else           // do it the standard way
    #define FUNCTION_DESCR __FILE__
#endif

#define STRACE_PREPARE \
    std::string(FUNCTION_DESCR) + ":" + \
    static_cast<ostringstream*>( &(ostringstream() << __LINE__) )->str()

/// Throws an exception with error message
/**
    \param message - what happened?
*/
#define STRACE_THROW(msg) \
    STRACE_PREPARE + ( std::string(msg).empty() ? "" : (std::string("\n : ") + msg) )  + "\n"

/// Throws another exception, propagating the error message from the passed exception
/**
    \param exception - source exception, whose error message is read and concatenated
    \param message - what happened? (optional)
*/
#define STRACE_RETHROW(exception, msg) \
    STRACE_THROW(msg) + exception.what()

/// Provides stack trace capabilities
/**
    Allows to create an exception with an error message, and propagate the message to higher levels.
    By using the STRACE_PREPARE macro, you will also indicate in which place the problem occured. Example:
    \code{.cpp}
    class Test {
    public:
        bool SourceOfAllProblems() {
            int numberOfCats = 3 - 10;
            if ( numberOfCats < 0 ) {
                ostringstream oss;
                oss << "Incorrect number of cats = " << numberOfCats;
                throw invalid_argument( STRACE_THROW(oss.str()) );
            } else
                return true; // enforce return warnings checks
        }
    };

    bool Rethrower() {
        try {
            if ( Test().SourceOfAllProblems() )
                return true;
            else
                return false;
        } catch (invalid_argument & e) {
            throw invalid_argument( STRACE_RETHROW(e, "") );
        } catch (logic_error & e) {
            throw logic_error( STRACE_RETHROW(e, "") );
        } catch (exception & e) {
            throw runtime_error( STRACE_RETHROW(e, "") );
        }
    }

    void HighLevelHandler() {
        try {
            Rethrower();
        } catch (invalid_argument & e) {
            throw invalid_argument( STRACE_RETHROW(e, "It must be an an invalid argument") );
        } catch (logic_error & e) {
            throw logic_error( STRACE_RETHROW(e, "I think it's an error in the logic") );
        } catch (exception & e) {
            throw length_error( STRACE_RETHROW(e, "I dunno") );
        }

    }

    int main() {
        try {
            HighLevelHandler();
        } catch (std::exception & e) {
            cout << e.what() << endl;
            return 1;
        }
        return 0;
    }
    \endcode
    Produces:
    \code
    void HighLevelHandler():100
     : It must be an an invalid argument
    bool Rethrower():88
     :
    bool Test::SourceOfAllProblems():75
     : Incorrect number of cats = -7
    \endcode
*/

#endif // STACKTRACE_H_INCLUDED
