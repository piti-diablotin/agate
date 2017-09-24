/**
 * @file src/epigenexml.cpp
 *
 * @brief 
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


#include "hist/multibinit.hpp"
#include "base/exception.hpp"
#include "base/phys.hpp"
#include "base/geometry.hpp"
#include "base/mendeleev.hpp"
#include "base/utils.hpp"
#include <cstring>
#include <sstream>


#ifdef HAVE_LIBXML2
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#endif

//
Multibinit::Multibinit() {
  ;
}

//
Multibinit::~Multibinit() {
  ;
}

//
void Multibinit::readFromFile(const std::string& filename) {
#ifndef HAVE_LIBXML2
  throw EXCEPTION("XML support is not available.\nConsider compiling the code with libXML2 support to read "+filename,ERRDIV);
#else

  _xyz = 3;
  _natom = 0;
  _ntime = 1;
  _znucl.clear();
  _typat.clear();

  LIBXML_TEST_VERSION;

  int i,iamu,present,ntypat,mu,nu;
  double gprimd[9];
  unsigned int iatom;
  xmlChar *key,*uri;
  xmlDocPtr doc = nullptr;
  xmlNodePtr root, node, node2;
  xmlXPathContextPtr cptr = nullptr;
  xmlXPathObjectPtr xobj = nullptr;

  try {
    if ( (doc = xmlParseFile(filename.c_str())) == nullptr )
      throw EXCEPTION(std::string("File ")+filename+" could not be correctly parsed by libXML2",ERRDIV);

    if ( (root = xmlDocGetRootElement(doc)) == nullptr )
      throw EXCEPTION(std::string("File ")+filename+" is a vierge XML document",ERRABT);

    //Count number of atoms
    xmlXPathInit();
    if ( (cptr = xmlXPathNewContext(doc)) == nullptr )
      throw EXCEPTION("Error while creating XPath context",ERRABT);

    if ( (xobj = xmlXPathEvalExpression( BAD_CAST "//System_definition", cptr)) == NULL )
      throw EXCEPTION("Error while creating XPath context",ERRABT);

    ntypat = 0;
    if ( xobj->type == XPATH_NODESET ) {

      if ( xobj->nodesetval->nodeNr == 1 ) {
        //Get dimension (natom,ntypat)
        node = xobj->nodesetval->nodeTab[0]->xmlChildrenNode;
        if ( (xobj = xmlXPathEvalExpression( BAD_CAST "count(//atom)", cptr)) == NULL )
          throw EXCEPTION("Error while creating XPath context",ERRABT);
        ( xobj->type == XPATH_NUMBER ) ?  _natom = xmlXPathCastToNumber(xobj)
          : throw EXCEPTION("Error while evaluating number of atoms", ERRABT);

        //Get number of type
        iatom = 0;
        std::vector<double> amu(_natom);
        for(i=0; i<int(_natom); i++){
          amu[i] = 0 ;
        }

        while (node != nullptr) {   
          if ((!xmlStrcmp(node->name, (const  xmlChar *) "atom"))) {
            uri = xmlGetProp(node, (const  xmlChar *) "mass");
            present = 0;
            for(unsigned i=0; i< _natom; i++){
              if (amu[i] == utils::stod(reinterpret_cast<char*>(uri))){
                present = 1;
                break;
              }
            }
            if(present==0){
              amu[iatom] = (utils::stod(reinterpret_cast<char*>(uri)));
              ntypat++;
            }
            xmlFree(uri);
          }
          node = node->next; 
        }

        // Resize the array typat
        _typat.resize(_natom);
        _znucl.resize(ntypat);
        _xcart  .resize(_ntime*_natom*_xyz);
        _xred   .resize(_ntime*_natom*_xyz);
        _fcart  .resize(_ntime*_natom*_xyz);
        _acell  .resize(_ntime*_xyz);
        _rprimd .resize(_ntime*_xyz*_xyz);
        _time   .resize(_ntime);
        _stress .resize(_ntime*6);

        //Fill array of the structure
        doc = xmlParseFile(filename.c_str());
        root = xmlDocGetRootElement(doc);
        xobj = xmlXPathEvalExpression( BAD_CAST "//System_definition",cptr);
        node = xobj->nodesetval->nodeTab[0]->xmlChildrenNode;
        iamu  = 0;
        iatom = 0;
        while ( node != nullptr ) {
          if ((!xmlStrcmp(node->name, (const  xmlChar *) "unit_cell"))) {
            key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);      
            std::istringstream str;
            str.str(reinterpret_cast<char*>(key));
            for(mu=0;mu<3;mu++){
              _acell[mu]= 1. ;
              for(nu=0;nu<3;nu++){
                str >> _rprimd[mu*3+nu];
                if ( str.fail() )
                  throw EXCEPTION("Bad reading for basis",ERRABT);
              }
            }
            xmlFree(key);
          }           

          if ((!xmlStrcmp(node->name, (const  xmlChar *) "atom"))) {
            uri = xmlGetProp(node, (const  xmlChar *) "mass");
            present = 0;
            //1) fill the atomic mass unit
            for(i=0;i<=ntypat;i++){
              if(amu[i]==(utils::stod(reinterpret_cast<char*>(uri)))){
                present = 1;
                break;
              }
            }
            if(present==0){
              amu[iamu]=(utils::stod(reinterpret_cast<char*>(uri)));
              _znucl[iamu] = mendeleev::znucl(amu[iamu]);
              iamu++;
            }
            // fill the typat table
            for(i=0;i<=ntypat;i++){
              if(amu[i]==(utils::stod(reinterpret_cast<char*>(uri)))){
                _typat[iatom]=i+1;
              }
            }
            xmlFree(uri);

            node2 = node->xmlChildrenNode;
            while (node2 != NULL) {
              if ( iatom <=_natom) {
                if ((!xmlStrcmp(node2->name, (const  xmlChar *) "position"))) {
                  key = xmlNodeListGetString(doc, node2->xmlChildrenNode, 1);
                  std::istringstream str;
                  str.str(reinterpret_cast<char*>(key));
                  for(mu=0;mu<3;mu++){
                    str >> _xcart[3*iatom+mu];
                    if ( str.fail() )
                      throw EXCEPTION("Bad reading for basis",ERRABT);
                  }
                  iatom++;
                }
              }
              else{
                printf(" error: The number of atom doesn't match with the XML file\n");
                exit(0);
              } 
              node2 = node2->next;
            }
          }
          node = node->next; 
        } 
      }
    }
    else throw EXCEPTION("Error while accessing atominfo node", ERRABT);


    //convert in reduced coordinates
    //First : get grimd
    double t1  = _rprimd[4] * _rprimd[8] - _rprimd[7] * _rprimd[5];
    double t2  = _rprimd[7] * _rprimd[2] - _rprimd[1] * _rprimd[8];
    double t3  = _rprimd[1] * _rprimd[5] - _rprimd[4] * _rprimd[2];
    double det = _rprimd[0] * t1 + _rprimd[3] * t2 + _rprimd[6] * t3 ;

    //Make sure matrix is not singular
    if (std::abs(det)>1.0E-16){
      double dd = 1/det;
      gprimd[0] = t1 * dd;
      gprimd[3] = t2 * dd;
      gprimd[6] = t3 * dd;
      gprimd[1] = (_rprimd[6]*_rprimd[5]-_rprimd[3]*_rprimd[8]) * dd;
      gprimd[4] = (_rprimd[0]*_rprimd[8]-_rprimd[6]*_rprimd[2]) * dd;
      gprimd[7] = (_rprimd[3]*_rprimd[2]-_rprimd[0]*_rprimd[5]) * dd;
      gprimd[2] = (_rprimd[3]*_rprimd[7]-_rprimd[6]*_rprimd[4]) * dd;
      gprimd[5] = (_rprimd[6]*_rprimd[1]-_rprimd[0]*_rprimd[7]) * dd;
      gprimd[8] = (_rprimd[0]*_rprimd[4]-_rprimd[3]*_rprimd[1]) * dd;
    }
    else{
      throw EXCEPTION("Determinant of rprim is zero",ERRABT);
    }

    for (unsigned iatom = 0 ; iatom < _natom ; ++iatom) {
      _xred[iatom*3  ] = gprimd[0]*_xcart[iatom*3] + gprimd[1]*_xcart[iatom*3+1] + gprimd[2]*_xcart[iatom*3+2];
      _xred[iatom*3+1] = gprimd[3]*_xcart[iatom*3] + gprimd[4]*_xcart[iatom*3+1] + gprimd[5]*_xcart[iatom*3+2];
      _xred[iatom*3+2] = gprimd[6]*_xcart[iatom*3] + gprimd[7]*_xcart[iatom*3+1] + gprimd[8]*_xcart[iatom*3+2];
    }

    //Do some tests
    if ( ntypat > int(_znucl.size())) 
      throw EXCEPTION("Inconsistency between the number of type of atoms and the declared number of type",ERRABT);

    if ( _natom != _typat.size() ) 
      throw EXCEPTION("Inconsistency between the number of atoms and the declared number of atoms",ERRDIV);

    xmlCleanupParser();
    xmlXPathFreeObject(xobj);
    xmlXPathFreeContext(cptr);
    xmlFreeDoc(doc);
    _filename = filename;
    _ntimeAvail = _ntime;
  }
  catch(Exception &e) {
    xmlCleanupParser();
    xmlXPathFreeObject(xobj);
    xmlXPathFreeContext(cptr);
    xmlFreeDoc(doc);
    e.ADD("Unable to construct HistData from tatata file", e.getReturnValue());
    throw e;
  }

#endif
}
