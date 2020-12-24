/**
 * @file include/gnuplot.hpp
 *
 * @brief 
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


#ifndef GNUPLOT_HPP
#define GNUPLOT_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "plot/graph.hpp"
#include <cstdio>
#include <sstream>
#include <memory>

/** 
 *
 */
class Gnuplot : public Graph {

  private :
    std::unique_ptr<FILE> _gp;  ///< Descriptor to communicate with gnuplot
    std::stringstream _header; ///< Header to send to gnuplot before sending data
    std::stringstream _buffer; ///< All the string to send to gnuplot
    std::stringstream _custom;

    /**
     * Replace literal greek letters by the Symbol
     * @param input The string containing e.g. alpha
     * @return input with alpha replace by the correct replacement
     */
    static std::string translateToSymbol(const std::string &input);

  protected :

    /**
     * Add thing to gnuplot like ranges, and tics
     *
     */
    void addCustom();

  public :

    /**
     * Constructor.
     */
    Gnuplot();

    /**
     * Destructor.
     */
    virtual ~Gnuplot();

    /** 
     * Plot several quantities on the screen
     * @param x The x quantity
     * @param y A vector with several y quantites to plot
     * @param labels The labels corresponding to the y quantities.
     * @param colors a vector containing the color number for each vector y
     */
    virtual void plot(const std::vector<double> &x, const std::list<std::vector<double>> &y, const std::list<std::string> &labels, const std::vector<unsigned> &colors);
    
    /** 
     * Plot several quantities on the screen
     * @param x The x quantity
     * @param y A vector with several y quantites to plot
     * @param c A vector with colors for each x coordinate
     * @param labels The labels corresponding to the y quantities.
     */
    virtual void plot(const std::vector<double> &x, const std::list<std::vector<double>> &y, const std::list<std::vector<unsigned>> &c, const std::list<std::string> &labels);

    /** 
     * Plot several quantities on the screen
     * @param xy A list of (x,y) pairs to plot
     * @param labels The labels corresponding to the y quantities.
     */
    virtual void plot(const std::list<std::pair<std::vector<double>,std::vector<double>>> &xy, const std::list<std::string> &labels, const std::vector<unsigned> &colors);

    /**
     * Save the graph
     * @param filename Save to filename
     * */
    virtual void save(const std::string& filename);

    /**
     * Print out the commande to plot
     * @param out ostream for output
     * @param plotname is the filename for the file that would be created when the graph is created
     * Not the script file that would be executed to creat the graph
     */
    virtual void dump(std::ostream& out, const std::string& plotname) const;

};

#endif  // GNUPLOT_HPP
