/**
 * @file include/io/abihdr.hpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.fr>
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


#ifndef ABIHDR_HPP
#define ABIHDR_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "io/dtset.hpp"
#include "base/geometry.hpp"

/** 
 *
 */
class AbiHdr : virtual public Dtset {

  private :
    int _endHeader;

  protected :
    // <H1>
    char _codvsn[7]; ///< 7 is \0, only 6 relevant char
    int _hdrform; ///< Format of the header
    int _fform; ///< Code for data inside this file.
    // <H2>
    int _bandtot;
    int _date;
    int _intxc;
    int _ixc;
    //int _natom;
    int _ngfft[3];
    int _nkpt;
    int _nspden;
    int _nspinor;
    int _nsppol;
    int _nsym;
    int _npsp;
    //int _ntypat;
    int _occopt;
    int _pertcase;
    int _usepaw;
    double _ecut;
    double _ecutdg;
    double _ecutsm;
    double _ecut_eff;
    double _qptn[3];
    //double _rprimd[9];
    double _stmbias;
    double _tphysel;
    double _tsmear;
    int _usewvl;
    int _nshiftk_orig;
    int _nshiftk;
    int _mband;
    // <H3>
    std::vector<int> _istwfk;
    std::vector<int> _nband;
    std::vector<int> _npwarr;
    std::vector<int> _so_psp;
    std::vector<int> _symafm;
    std::vector<int> _symrel;
    //std::vector<> typat;
    std::vector<double> _kptns;
    std::vector<double> _occ3d;
    std::vector<double> _tnons;
    //std::vector<> znucltypat;
    std::vector<double> _wtk;
    // <H4>
    double _residm;
    //std::vector<double> _xred;
    double _etot;
    double _fermie;
    std::vector<double> _amu;
    // <H5>
    int _kptopt;
    int _pawcpxocc;
    int _nelect;
    double _charge;
    double _icoulomb;
    int _kptrlatt[9];
    int _kptrlatt_orig[9];
    std::vector<double> _shiftk_orig;
    std::vector<double> _shiftk;
    // <H6>
    std::vector<std::string> _title;
    std::vector<int> _znuclpsp;
    std::vector<int> _zionpsp;
    std::vector<int> _pspso;
    std::vector<int> _pspdat;
    std::vector<int> _pspcod;
    std::vector<double> _pspxc;
    std::vector<double> _lmn_size;
    std::vector<std::string> _md5_pseudos;

  public :

    /**
     * Constructor.
     */
    AbiHdr();

    /**
     * Destructor.
     */
    virtual ~AbiHdr();

    /**
     * Fill a Dtset and internal variables with abinit data
     * @param filename File of a unformatted fortran  binary file written by Abinit
     */
    virtual void readFromFile(const std::string& filename);

    const char* codvsn(){return _codvsn;}
    const int& hdrform(){return _hdrform;}
    const int& fform(){return _fform;}
    // <H2>                 
    const int& bandtot(){return _bandtot;}
    const int& date(){return _date;}
    const int& intxc(){return _intxc;}
    const int& ixc(){return _ixc;}
    //int _natom;
    const int* ngfft(){return _ngfft;}
    const int& nkpt(){return _nkpt;}
    const int& nspden(){return _nspden;}
    const int& nspinor(){return _nspinor;}
    const int& nsppol(){return _nsppol;}
    const int& nsym(){return _nsym;}
    const int& npsp(){return _npsp;}
    //int _ntypat;
    const int& occopt(){return _occopt;}
    const int& pertcase(){return _pertcase;}
    const int& usepaw(){return _usepaw;}
    const double& ecut(){return _ecut;}
    const double& ecutdg(){return _ecutdg;}
    const double& ecutsm(){return _ecutsm;}
    const double& ecut_eff(){return _ecut_eff;}
    const double* qptn(){return _qptn;}
    //double _rprimd[9];
    const double& stmbias(){return _stmbias;}
    const double& tphysel(){return _tphysel;}
    const double& tsmear(){return _tsmear;}
    const int& usewvl(){return _usewvl;}
    const int& nshiftk_orig(){return _nshiftk_orig;}
    const int& nshiftk(){return _nshiftk;}
    const int& mband(){return _mband;}
    // <H3>
    const std::vector<int>& istwfk(){return _istwfk;}
    const std::vector<int>& nband(){return _nband;}
    const std::vector<int>& npwarr(){return _npwarr;}
    const std::vector<int>& so_psp(){return _so_psp;}
    const std::vector<int>& symafm(){return _symafm;}
    const std::vector<int>& symrel(){return _symrel;}
    //std::vector<> typat;
    const std::vector<double>& kptns(){return _kptns;}
    const std::vector<double>& occ3d(){return _occ3d;}
    const std::vector<double>& tnons(){return _tnons;}
    //std::vector<> znucltypat;
    const std::vector<double>& wtk(){return _wtk;}
    // <H4>
    const double& residm(){return _residm;}
    //std::vector<double> _xred;
    const double& etot(){return _etot;}
    const double& fermie(){return _fermie;}
    const std::vector<double>& amu(){return _amu;}
    // <H5>
    const int& kptopt(){return _kptopt;}
    const int& pawcpxocc(){return _pawcpxocc;}
    const int& nelect(){return _nelect;}
    const double& charge(){return _charge;}
    const double& icoulomb(){return _icoulomb;}
    const int* kptrlatt(){return _kptrlatt;}
    const int* kptrlatt_orig(){return _kptrlatt_orig;}
    const std::vector<double>& shiftk_orig(){return _shiftk_orig;}
    const std::vector<double>& shiftk(){return _shiftk;}
    // <H6>
    const std::vector<std::string>& title(){return _title;}
    const std::vector<int>& znuclpsp(){return _znuclpsp;}
    const std::vector<int>& zionpsp(){return _zionpsp;}
    const std::vector<int>& pspso(){return _pspso;}
    const std::vector<int>& pspdat(){return _pspdat;}
    const std::vector<int>& pspcod(){return _pspcod;}
    const std::vector<double>& pspxc(){return _pspxc;}
    const std::vector<double>& lmn_size(){return _lmn_size;}
    const std::vector<std::string>& md5_pseudos(){return _md5_pseudos;}

    int endHeader(){return _endHeader;}

};

#endif  // ABIHDR_HPP
