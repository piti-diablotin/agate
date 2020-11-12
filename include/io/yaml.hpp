/**
 * @file include/yaml.hpp
 *
 * @brief Wrapper to include yaml-cpp when using -Weffc++
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2016 Jordan Bieder
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


#ifndef YAML_HPP
#define YAML_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#ifdef HAVE_YAMLCPP
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Weffc++"
#      endif
#    endif
#    pragma GCC system_header
#  endif

#include <yaml-cpp/yaml.h>

namespace YAML {
  using geometry::vec3d;
  template<>
    struct convert<vec3d> {
      static Node encode(const vec3d& vec) {
        Node node;
        node.push_back(vec[0]);
        node.push_back(vec[1]);
        node.push_back(vec[2]);
        return node;
      }

      static bool decode(const Node& node, vec3d& vec) {
        if(!node.IsSequence() || node.size() != 3) {
          return false;
        }

        vec[0] = node[0].as<double>();
        vec[1] = node[1].as<double>();
        vec[2] = node[2].as<double>();
        return true;
      }
    };
}

#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif
#endif


#endif  // YAML_HPP
