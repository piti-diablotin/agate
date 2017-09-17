#!/bin/bash

function header {
cat > $1 << EOF
/**
 * @file $1
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2017 Jordan Bieder
 *
 * This file is part of AbiOut.
 *
 * AbiOut is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AbiOut is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AbiOut.  If not, see <http://www.gnu.org/licenses/>.
 */

EOF
}

[ $# -lt 2 ] && echo "Need more arguments" && exit 1

if [ $1 == "class" ]
then
  folder=$(dirname $2)
  name=$(basename $2)
  class=${name^}
  echo "Creating class " $class
  hppname=include/$folder/${class,,}.hpp
  cppname=src/$folder/${class,,}.cpp

  header $hppname
  header $cppname

  cat >> $hppname << EOF

#ifndef ${class^^}_HPP
#define ${class^^}_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

/** 
 *
 */
class $class {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    $class();

    /**
     * Destructor.
     */
    virtual ~$class();
};

#endif  // ${class^^}_HPP
EOF

  cat >> $cppname << EOF

#include "$folder/${class,,}.hpp"

//
$class::$class() {
  ;
}

//
$class::~$class() {
  ;
}

EOF

elif [ $1 == "namespace" ]
then
  ns=${2,,}
  echo "Creating namespace " $ns
  hppname=include/${ns}.hpp
  cppname=src/${ns}.cpp

  header $hppname
  header $cppname

  cat >> $hppname << EOF

#ifndef ${ns^^}_HPP
#define ${ns^^}_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

namespace $ns {

}

#endif  // ${ns^^}_HPP
EOF

  cat >> $cppname << EOF

#include "${ns}.hpp"

namespace $ns {

}

EOF
elif [ $1 == "header" ]
then
  file=${2,,}
  hppname=include/${file}.hpp
  echo "Creating file" $hppname

  header $hppname
  cat >> $hppname << EOF

#ifndef ${file^^}_HPP
#define ${file^^}_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif


#endif  // ${file^^}_HPP
EOF

else 
  echo "Unknow option $1"
  exit 1
fi
