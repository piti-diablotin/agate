/**
 * @file include/conducti/conducti.hpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#ifndef CONDUCTI_HPP
#define CONDUCTI_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "conducti/abiopt.hpp"
#include "io/configparser.hpp"
#include "base/phys.hpp"
#include "plot/graph.hpp"

/** 
 *
 */
class Conducti {

  private :
    enum rangeSelection { NONE, ENERGY, BAND };

    int _nsppol;
    int _nomega;
    double _omegaMin;
    double _omegaMax;
    double _smearing;
    Units::Energy _eunit;
    double _sunit;

    int _bandSelection[4];
    double _energySelection[4];
    double _histDelta;
    double _histBorder[2];
    rangeSelection _selection;
    std::vector<double> _omega;
    std::vector<double> _sigma;
    std::vector<double> _histogramI;
    std::vector<double> _histogramJ;

  protected :

    void buildOmega();
    void buildHistogram(double min, double max, int npoints);

  public :
    enum TensorIndex { XX, YY, ZZ, YZ, XZ, XY };

    /**
     * Constructor.
     */
    Conducti();

    /**
     * Destructor.
     */
    virtual ~Conducti();

    void setNOmega(int nomega);

    void setSmearing(double smearing);

    void setOmegaRange(double omin, double omax);

    void setRange(double eMin1, double eMax1, double eMin2, double eMax2);

    void setRange(int bandMin1, int bandMax1, int bandMin2, int bandMax2);

    void setUnits(const std::string &eunit, const std::string &sunit);

    const std::vector<double>& omega() const { return _omega; }

    void fullTensor(const AbiOpt &abiopt);

    void diagonalTensor(const AbiOpt &abiopt);

    void traceTensor(const AbiOpt &abiopt);

    void printSigma(std::ostream& out);
    void printHistogram(std::ostream& out);

    void setParameters(ConfigParser &parser);

    void getResultSigma(Graph::Config &config);

    void getResultHistogram(Graph::Config &config);

    static double getOmegaMax(const AbiOpt &abiopt);

};

#endif  // CONDUCTI_HPP
