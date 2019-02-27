/**
 * @file src/vaspxml.cpp
 *
 * @brief Read a vasprun.xml 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
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


#include "hist/vaspxml.hpp"
#include "base/exception.hpp"
#include "base/phys.hpp"
#include "base/geometry.hpp"
#include "base/mendeleev.hpp"
#include <sstream>

#ifdef HAVE_LIBXML2
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#endif

using namespace Agate;

//
VaspXML::VaspXML() {
  ;
}

//
VaspXML::~VaspXML() {
  ;
}

//
void VaspXML::readFromFile(const std::string& filename) {
#ifndef HAVE_LIBXML2
  throw EXCEPTION("XML support is not available.\nConsider compiling the code with libXML2 support to read "+filename,ERRDIV);
#else

  _xyz = 3;
  _natom = 0;
  _ntime = 0;
  _znucl.clear();
  _typat.clear();

  LIBXML_TEST_VERSION;

  xmlDocPtr doc = nullptr;
  xmlNodePtr root, node = nullptr;
  xmlXPathContextPtr cptr = nullptr;
  xmlXPathObjectPtr xobj = nullptr;

  double dtion = 0;

  try {
    if ( (doc = xmlParseFile(filename.c_str())) == nullptr )
      throw EXCEPTION(std::string("File ")+filename+" could not be correctly parsed by libXML2",ERRDIV);


    if ( (root = xmlDocGetRootElement(doc)) == nullptr )
      throw EXCEPTION(std::string("File ")+filename+" is a vierge XML document",ERRABT);

    // Count calculation steps.
    xmlXPathInit();
    if ( (cptr = xmlXPathNewContext(doc)) == nullptr )
      throw EXCEPTION("Error while creating XPath context",ERRABT);

    if ( (xobj = xmlXPathEvalExpression( BAD_CAST "count(//structure)", cptr)) == NULL )
      throw EXCEPTION("Error while creating XPath context",ERRABT);
    ( xobj->type == XPATH_NUMBER ) ?  _ntime = xmlXPathCastToNumber(xobj)
      : throw EXCEPTION("Error while evaluating number of calculations", ERRABT);
    
    // Get dtion = POTIM
    if ( (xobj = xmlXPathEvalExpression( BAD_CAST "//incar", cptr)) == NULL )
      throw EXCEPTION("Error while creating XPath context",ERRABT);
    if ( xobj->type == XPATH_NODESET ) {
      if ( xobj->nodesetval->nodeNr == 1 ) {
        node = xobj->nodesetval->nodeTab[0]->xmlChildrenNode;
        while ( node != nullptr ) {
          if ( !xmlStrcmp( xmlGetProp(node,(const xmlChar*)"name"), (const xmlChar*)"POTIM") ) {
            xmlChar *key = xmlNodeListGetString(doc,node->xmlChildrenNode,1);
            dtion = utils::stod(reinterpret_cast<char*>(key))/phys::atu2fs;
            xmlFree(key);
          }
          node = node->next; 
        }
      }
    }

    // Get atominfo node to read natoms and other informations.
    if ( (xobj = xmlXPathEvalExpression( BAD_CAST "//atominfo", cptr)) == NULL )
      throw EXCEPTION("Error while creating XPath context",ERRABT);

    if ( xobj->type == XPATH_NODESET ) {
      size_t ntypat = 0;
      if ( xobj->nodesetval->nodeNr == 1 ) {
        node = xobj->nodesetval->nodeTab[0]->xmlChildrenNode;

        while ( node != nullptr ) {
          // atoms
          if ( !xmlStrcmp(node->name, (const xmlChar*)"atoms") ) {
            xmlChar *key = xmlNodeListGetString(doc,node->xmlChildrenNode,1);
            _natom = utils::stoi(reinterpret_cast<char*>(key));
            xmlFree(key);
          }
          else if ( !xmlStrcmp(node->name, (const xmlChar*)"types") ) {
            xmlChar *key = xmlNodeListGetString(doc,node->xmlChildrenNode,1);
            ntypat = static_cast<size_t>(utils::stoi(reinterpret_cast<char*>(key)));
            xmlFree(key);
          }
          else if ( !xmlStrcmp(node->name, (const xmlChar*)"array") ) {
            if ( !xmlStrcmp( xmlGetProp(node,(const xmlChar*)"name"), (const xmlChar*)"atoms") ) {
              //Read type of each atom
              xmlNodePtr natm = node->xmlChildrenNode;
              while ( natm != nullptr ) {
                if ( !xmlStrcmp( natm->name, (const xmlChar*)"set") ) {
                  xmlNodePtr typat = natm->xmlChildrenNode;
                  while ( typat != nullptr ) {
                    if ( !xmlStrcmp( typat->name, (const xmlChar*)"rc") ) {
                      xmlChar *key = xmlNodeListGetString(doc,typat->last->xmlChildrenNode,1);
                      _typat.push_back(utils::stoi(reinterpret_cast<char*>(key)));
                      xmlFree(key);
                    }
                    typat = typat->next;
                  }
                }
                natm = natm->next;
              }
            }
            else if ( !xmlStrcmp( xmlGetProp(node,(const xmlChar*)"name"), (const xmlChar*)"atomtypes") ) {
              xmlNodePtr nodeset = node->xmlChildrenNode;
              while ( nodeset != nullptr ) {
                if ( !xmlStrcmp( nodeset->name, (const xmlChar*)"set") ) {
                  xmlNodePtr dataptr = nodeset->xmlChildrenNode;
                  while ( dataptr != nullptr ) {
                    if ( !xmlStrcmp( dataptr->name, (const xmlChar*)"rc") ) {
                      int second_col = 0;
                      xmlNodePtr colptr = dataptr->xmlChildrenNode;
                      while( second_col < 2 && colptr != nullptr ) {
                        if ( !xmlStrcmp(colptr->name,(const xmlChar*)"c") ) ++second_col;
                        colptr = colptr->next;
                      }
                      colptr = colptr->prev;
                      xmlChar *key = xmlNodeListGetString(doc,colptr->xmlChildrenNode,1);
                      _znucl.push_back(mendeleev::znucl(std::string(reinterpret_cast<char*>(key))));
                      xmlFree(key);
                    }
                    dataptr = dataptr->next;
                  }
                }
                nodeset = nodeset->next;
              }
            }
          }
          node = node->next; 
        }
      }
      else throw EXCEPTION("Error while accessing atominfo node", ERRDIV);
      if ( ntypat > _znucl.size() ) 
        throw EXCEPTION("Inconsistency between the number of type of atoms and the declared number of type",ERRABT);
    }
    xmlXPathFreeObject(xobj);

    if ( _natom != _typat.size() ) 
      throw EXCEPTION("Inconsistency between the number of atoms and the declared number of atoms",ERRDIV);

    _xcart  .resize(_ntime*_natom*_xyz);
    _xred   .resize(_ntime*_natom*_xyz);
    _fcart  .resize(_ntime*_natom*_xyz);
    _acell  .resize(_ntime*_xyz);
    _rprimd .resize(_ntime*_xyz*_xyz);
    _time   .resize(_ntime);
    _stress .resize(_ntime*6);

    _ekin   .resize(_ntime);
    _etotal .resize(_ntime);
    _velocities.resize(_ntime*_natom*_xyz);
    _entropy .resize(_ntime);
    _temperature.resize(_ntime);
    _pressure.resize(_ntime);

    auto getStruct = [&](xmlNodePtr strptr, int itime){
      if ( xmlStrcmp(strptr->name,(const xmlChar*)"structure") )
        throw EXCEPTION("Not a structure node",ERRABT);

      xmlNodePtr node = strptr->xmlChildrenNode;
      while ( node != nullptr ) {
        if ( !xmlStrcmp( node->name, (const xmlChar*)"crystal") ) {
          xmlNodePtr basis = node->xmlChildrenNode;
          while ( basis != nullptr 
              && xmlStrcmp(basis->name,(const xmlChar*)"varray")
              && xmlStrcmp(xmlGetProp(basis,(const xmlChar*)"name"),(const xmlChar*)"basis")
              ) {
            basis = basis->next;
          }
          if ( basis == nullptr )
            throw EXCEPTION("Unable to find basis array",ERRABT);
          basis = basis->xmlChildrenNode;
          // basis now go through vectors <v>
          unsigned vec = 0;
          while ( basis != nullptr && vec < 3) {
            if ( !xmlStrcmp( basis->name, (const xmlChar*)"v") ) {
              xmlChar *key = xmlNodeListGetString(doc,basis->xmlChildrenNode,1);
              std::istringstream str;
              str.str(reinterpret_cast<char*>(key));
              xmlFree(key);
              for ( unsigned i = 0 ; i < 3 ; ++i ) {
                str >> _rprimd[itime*9+vec+i*3];
                _rprimd[itime*9+vec+i*3] /= phys::b2A;
                if ( str.fail() )
                  throw EXCEPTION("Bad reading for basis",ERRABT);
              }
              ++vec;
            }
            basis = basis->next;
          }
        }
        else if ( !xmlStrcmp( node->name, (const xmlChar*)"varray") 
            && !xmlStrcmp( xmlGetProp(node,(const xmlChar*)"name"), (const xmlChar*)"positions") ) {
          xmlNodePtr pos = node->xmlChildrenNode;
          // basis now go through vectors <v>
          unsigned vec = 0;
          while ( pos != nullptr && vec < _natom) {
            if ( !xmlStrcmp( pos->name, (const xmlChar*)"v") ) {
              xmlChar *key = xmlNodeListGetString(doc,pos->xmlChildrenNode,1);
              std::istringstream str;
              str.str(reinterpret_cast<char*>(key));
              xmlFree(key);
              for ( unsigned i = 0 ; i < 3 ; ++i ) {
                str >> _xred[itime*3*_natom+vec*3+i];
                if ( str.fail() )
                  throw EXCEPTION("Bad reading for positions",ERRABT);
              }
              ++vec;
            }
            pos = pos->next;
          }
        }
        else if ( !xmlStrcmp( node->name, (const xmlChar*)"varray") 
            && !xmlStrcmp( xmlGetProp(node,(const xmlChar*)"name"), (const xmlChar*)"velocities") ) {
          xmlNodePtr pos = node->xmlChildrenNode;
          // basis now go through vectors <v>
          unsigned vec = 0;
          while ( pos != nullptr && vec < _natom) {
            if ( !xmlStrcmp( pos->name, (const xmlChar*)"v") ) {
              xmlChar *key = xmlNodeListGetString(doc,pos->xmlChildrenNode,1);
              std::istringstream str;
              str.str(reinterpret_cast<char*>(key));
              xmlFree(key);
              for ( unsigned i = 0 ; i < 3 ; ++i ) {
                str >> _velocities[itime*3*_natom+vec*3+i];
                if ( str.fail() )
                  throw EXCEPTION("Bad reading for velocities",ERRABT);
              }
              ++vec;
            }
            pos = pos->next;
          }
        }
        node = node->next;
      }
    };

    auto getForcesStressEnergies = [&](xmlNodePtr calculation, int itime){
      const double forceConversion = phys::b2A/phys::Ha2eV;
      const double stressConversion = -1e-1/(phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21); // kbar -> Ha/bohr^3 and add - because vasp seems to store -sigma
      if ( xmlStrcmp(calculation->name,(const xmlChar*)"calculation") ) 
        return;

      xmlNodePtr node = calculation->xmlChildrenNode;
      while ( node != nullptr ) {
        if ( !xmlStrcmp( node->name, (const xmlChar*)"varray") 
            && !xmlStrcmp(xmlGetProp(node,(const xmlChar*)"name"),(const xmlChar*)"forces")
           ) {
          xmlNodePtr forces = node->xmlChildrenNode;
          unsigned vec = 0;
          while ( forces != nullptr && vec < _natom ) {
            // forces now goes through vectors <v>
            if ( !xmlStrcmp( forces->name, (const xmlChar*)"v") ) {
              xmlChar *key = xmlNodeListGetString(doc,forces->xmlChildrenNode,1);
              std::istringstream str;
              str.str(reinterpret_cast<char*>(key));
              xmlFree(key);
              for ( unsigned i = 0 ; i < 3 ; ++i ) {
                str >> _fcart[itime*3*_natom+vec*3+i];
                if ( str.fail() )
                  throw EXCEPTION("Bad reading for forces",ERRABT);
              }
              ++vec;
            }
            forces = forces->next;
          }
        }
        else if ( !xmlStrcmp( node->name, (const xmlChar*)"varray") 
            && !xmlStrcmp(xmlGetProp(node,(const xmlChar*)"name"),(const xmlChar*)"stress")
            ) {
          xmlNodePtr stress = node->xmlChildrenNode;
          unsigned vec = 0;
          double stresstmp[9] = {0.};
          while ( stress != nullptr && vec < 3 ) {
            // stress now goes through vectors <v>
            if ( !xmlStrcmp( stress->name, (const xmlChar*)"v") ) {
              xmlChar *key = xmlNodeListGetString(doc,stress->xmlChildrenNode,1);
              std::istringstream str;
              str.str(reinterpret_cast<char*>(key));
              xmlFree(key);
              for ( unsigned i = 0; i < 3 ; ++i ) {
                str >> stresstmp[vec*3+i];
                if ( str.fail() )
                  throw EXCEPTION("Bad reading for forces",ERRABT);
              }
              ++vec;
            }
            stress = stress->next;
          }
          _stress[itime*6  ] = stressConversion * stresstmp[0];
          _stress[itime*6+1] = stressConversion * stresstmp[4];
          _stress[itime*6+2] = stressConversion * stresstmp[8];
          _stress[itime*6+3] = stressConversion * 0.5 * (stresstmp[5]+stresstmp[7]);
          _stress[itime*6+4] = stressConversion * 0.5 * (stresstmp[2]+stresstmp[6]);
          _stress[itime*6+5] = stressConversion * 0.5 * (stresstmp[1]+stresstmp[3]);
        }
        else if ( !xmlStrcmp( node->name, (const xmlChar*)"energy") ) {
          xmlNodePtr energy = node->xmlChildrenNode;
          unsigned read = 0;
          double e_fr_energy = 0;
          double e_wo_entrp = 0;
          double kinetic = 0;
          while ( energy != nullptr && read < 3 ) {
            if ( !xmlStrcmp(xmlGetProp(energy,(const xmlChar*)"name"),(const xmlChar*)"e_fr_energy") ) {
              xmlChar *key = xmlNodeListGetString(doc,energy->xmlChildrenNode,1);
              e_fr_energy = utils::stod(reinterpret_cast<char*>(key));
              ++read;
              xmlFree(key);
            }
            else if ( !xmlStrcmp(xmlGetProp(energy,(const xmlChar*)"name"),(const xmlChar*)"e_wo_entrp") ) {
              xmlChar *key = xmlNodeListGetString(doc,energy->xmlChildrenNode,1);
              e_wo_entrp = utils::stod(reinterpret_cast<char*>(key));
              ++read;
              xmlFree(key);
            }
            else if ( !xmlStrcmp(xmlGetProp(energy,(const xmlChar*)"name"),(const xmlChar*)"kinetic") ) {
              xmlChar *key = xmlNodeListGetString(doc,energy->xmlChildrenNode,1);
              kinetic = utils::stod(reinterpret_cast<char*>(key));
              ++read;
              xmlFree(key);
            }
            energy = energy->next;
          }
          _ekin[itime] = kinetic/phys::Ha2eV;
          _entropy[itime] = (e_fr_energy-e_wo_entrp)/phys::Ha2eV;
          _etotal[itime] = e_fr_energy/phys::Ha2eV;
        }
        node = node->next;
      }
      for ( unsigned iatomc = 0 ; iatomc < 3*_natom ; ++iatomc ) {
        _fcart[itime*3*_natom+iatomc] *= forceConversion;
      }
    };
    // Get structure NOT FORCES NOR STRESS
    if ( (xobj = xmlXPathEvalExpression( BAD_CAST "//structure", cptr)) == NULL )
      throw EXCEPTION("Error while creating XPath context",ERRABT);

    if ( xobj->type == XPATH_NODESET ) {
      _ntimeAvail = 0;
      for ( int itime = 0 ; itime < xobj->nodesetval->nodeNr ; ++itime ) {
        xmlNodePtr structure = xobj->nodesetval->nodeTab[itime];
        getStruct(structure,itime);
        getForcesStressEnergies(structure->parent,itime);
        _acell[itime*3  ] = 1.;
        _acell[itime*3+1] = 1.;
        _acell[itime*3+2] = 1.;
        _time[itime]=itime*dtion;

          // Remove periodicity
        if ( itime > 0 ) {
          for (unsigned iatom = 0 ; iatom < _natom ; ++iatom) {
            for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
              const double diff = _xred[(itime*_natom+iatom)*3+coord]-_xred[((itime-1)*_natom+iatom)*3+coord] + 0.5;
              _xred[(itime*_natom+iatom)*3+coord] -= std::floor(diff);
            }
          }
        }

        for (unsigned iatom = 0 ; iatom < _natom ; ++iatom) {
          _xcart[itime*3*_natom+iatom*3  ] = _rprimd[itime*9+0]*_xred[itime*3*_natom+iatom*3] + _rprimd[itime*9+1]*_xred[itime*3*_natom+iatom*3+1] + _rprimd[itime*9+2]*_xred[itime*3*_natom+iatom*3+2];
          _xcart[itime*3*_natom+iatom*3+1] = _rprimd[itime*9+3]*_xred[itime*3*_natom+iatom*3] + _rprimd[itime*9+4]*_xred[itime*3*_natom+iatom*3+1] + _rprimd[itime*9+5]*_xred[itime*3*_natom+iatom*3+2];
          _xcart[itime*3*_natom+iatom*3+2] = _rprimd[itime*9+6]*_xred[itime*3*_natom+iatom*3] + _rprimd[itime*9+7]*_xred[itime*3*_natom+iatom*3+1] + _rprimd[itime*9+8]*_xred[itime*3*_natom+iatom*3+2];
        }
        this->computeVelocitiesPressureTemperature(itime,dtion);
        ++_ntimeAvail;
      }
      if ( _ntimeAvail == _ntime && _ntime > 1 ) {
        std::ostringstream thermo;
        this->printThermo(0,_ntime,thermo);
        std::cout << thermo.str() << std::endl;
      }
      //else std::cerr << "totot" << xobj->nodesetval->nodeNr << std::endl;
    }
    else
      throw EXCEPTION("Unable to read initial structure",ERRABT);

    xmlCleanupParser();
    xmlXPathFreeObject(xobj);
    xmlXPathFreeContext(cptr);
    xmlFreeDoc(doc);
    _filename = filename;
    _isPeriodic = false;
  }
  catch(Exception &e) {
    xmlCleanupParser();
    xmlXPathFreeObject(xobj);
    xmlXPathFreeContext(cptr);
    xmlFreeDoc(doc);
    e.ADD("Unable to construct HistData from XML file", e.getReturnValue());
    throw e;
  }

#endif
}
