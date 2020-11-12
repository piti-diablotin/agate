/**
 * @file exception.hpp
 *
 * @brief Header file for the Exception class used everywhere.
 *
 * It provides several verbosity mode and can produce a traceback of the catch exception.
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of Agate.
 *
 * Agate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Agate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Agate.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif


#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/// @def EXCEPTION(text,error)
/// @brief Macro that initialize automatically a new exception with the filename, function name and the line of the exception.
///  @see Exception();
#define EXCEPTION(text,error) Exception(__FILE__, __func__, __LINE__, text, error)

/// @def ADD(text,error)
/// @brief Macro that adds automatically an trace to an exception object.
/// @see add();
#define ADD(text,error) add(__FILE__, __func__, __LINE__, text, error)

/// @def VDEBUG
/// @brief Provides a full debug mode for the exception messages.
/// @see setVerbosity()
/// @see what()
/// @see fullWhat()
#define VDEBUG 0

/// @def VCOMM
/// @brief Provides a simple verbose mode for the exception messages : only the messages.
/// @see setVerbosity()
/// @see what()
/// @see fullWhat()
#define VCOMM 4

/// @def ERRMEM
/// @brief Problem of memory (allocation/free/copy/access)
#define ERRMEM -10

/// @def ERRWAR
/// @brief This is just a warning, no need to stop
#define ERRWAR -20

/// @def ERRCOM
/// @brief This is just a comment.
#define ERRCOM -30

/// @def ERRDIV
/// @brief This is an error but not memory
#define ERRDIV -40

/// @def ERRABT
/// @brief This is an error with an abort process
#define ERRABT -50

/// The Exception class. Used everywhere to handle c++ exceptions.
class Exception {

  private :

    std::vector<std::string> _file; ///< Filename where the last exception occurred.
    std::vector<std::string> _line; ///< Line number where the last exception occurred.
    std::vector<std::string> _func; ///< Function where the last exception occurred.
    std::vector<std::string> _text; ///< Error message where the last exception occurred.
    unsigned    _activeStack;       ///< What element should be displayed by what();
    int         _returnValue;       ///< Error number returned.

    static int  _verbose; ///< Verbose mode used to display the traceback.

  public :

    /// Full constructor with default value -1 for the error.
    /// @param file The file name.
    /// @param func The function name.
    /// @param line The line number.
    /// @param message The error message.
    /// @param value The returned error value.
    Exception(const std::string file, const std::string func, int line, const std::string message, int value=-1) _NOEXCEPT;

    /// Constructor without line number parameter and with default value -1 for the error.
    /// @param para The file name or the function name.
    /// @param message The error message.
    /// @param value The returned error value.
    Exception(const std::string para, const std::string message, int value=-1) _NOEXCEPT;

    /// Minimal constructor with default value -1 for the error.
    /// @param message The error message.
    /// @param value The returned error value.
    Exception(const std::string message, int value=-1) _NOEXCEPT;

    /// Minimal constructor does nothing;
    Exception() _NOEXCEPT;

    /// Copy constructor.
    /// @param e The copied exception.
    Exception(const Exception& e) _NOEXCEPT;
    
    /// Destructor of an exception.
    virtual ~Exception();

    /// Method to add a full new trace message before throwing.
    /// @param file The file name.
    /// @param func The function name.
    /// @param line The line number.
    /// @param message The error message.
    /// @param value The returned error value.
    virtual void add(const std::string file, const std::string func, int line, const std::string message, int value=-1) _NOEXCEPT;

    /// Method to add a reduced new trace message before throwing.
    /// @param para The file name or the function name.
    /// @param message The error message.
    /// @param value The returned error value.

    virtual void add(const std::string para, const std::string message, int value=-1) _NOEXCEPT;
    /// Method to add a reduced new trace message before throwing.
    /// @param message The error message.
    /// @param value The returned error value.
    virtual void add(const std::string message, int value=-1) _NOEXCEPT;

    /// Method that format a message according to the verbosity mode.
    /// @return a formated string.
    /// @see fullWhat
    /// @see setVerbosity
    virtual std::string what(std::string prefix="",bool textonly=false) const _NOEXCEPT;

    /// Method that format the whole trace back message.
    /// @return a formated string.
    /// @see what
    /// @see setVerbosity
    virtual std::string fullWhat() _NOEXCEPT;

    /// Simply return the last error value.
    /// @return the last error value.
    virtual int getReturnValue() const _NOEXCEPT;

    /// Change the way trace back is displayed.
    /// @param verbose Integer that change the verbosity mode. Can 0(VDEBUG), 1, 2, 3, 4(VCOMM)
    static void setVerbosity(int verbose) _NOEXCEPT;

    /// Add two exceptions
    /// Actually only the traceback is copied.
    /// @param e The exception to add to the current one
    /// @return The current Exception with the new traceback.
    Exception& operator += (Exception& e);
};
#endif // EXCEPTION_HPP
