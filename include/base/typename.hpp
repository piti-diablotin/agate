/**
 * @file typename.hpp
 *
 * @brief A tiny structure to get correspondace between type and string name.
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

#ifndef HAVE_TYPENAME_H
#define HAVE_TYPENAME_H

#include <typeinfo>
#include <string>

/**
 * Template structure to convert type into name
 */
template <typename T>
struct TypeName
{
  /**
   * Return the name of the type but can be number or something not human readable.
   * @return the type.
   */
  static const std::string get() {
    return typeid(T).name();
  }
};

/**
 * Specialize the definition to always get the same name.
 * For char type.
 */
template <>
struct TypeName<char>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "char";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For short type.
 */
template <>
struct TypeName<short>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "short";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For int type.
 */
template <>
struct TypeName<int>
{
  /**
   * Return the name of the type in human readable maner.
   */
  static const std::string get() {
    return "int";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For unsigned type.
 */
template <>
struct TypeName<unsigned>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "unsigned";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For long type.
 */
template <>
struct TypeName<long>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "long";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For unsigned long type.
 */
template <>
struct TypeName<unsigned long>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "unsigned long";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For float type.
 */
template <>
struct TypeName<float>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "float";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For double type.
 */
template <>
struct TypeName<double>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "double";
  }
};


/**
 * Specialize the definition to always get the same name.
 * For std::string type.
 */
template <>
struct TypeName<std::string>
{
  /**
   * Return the name of the type in human readable maner.
   * @return the human readable type.
   */
  static const std::string get() {
    return "std::string";
  }
};

#endif
