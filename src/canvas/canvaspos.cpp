/**
 * @file src/canvaspos.cpp
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


#include <cstring>
#include <algorithm>
#include <typeinfo>
#include <iomanip>

#ifdef HAVE_CPPTHREAD
#include <thread>
#endif

#ifdef HAVE_GL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#include "canvas/canvaspos.hpp"
#include "base/exception.hpp"
#include "base/mendeleev.hpp"
#include "io/dtset.hpp"
#include "io/poscar.hpp"
#include "phonons/supercell.hpp"
#include "plot/gnuplot.hpp"
#include "io/eigparser.hpp"
#include "io/configparser.hpp"
#include "bind/tdep.hpp"
#include "hist/histdatadtset.hpp"

//
CanvasPos::CanvasPos(bool drawing) : Canvas(drawing),
  _natom(0),
  _ntypat(0),
  _typat(),
  _znucl(),
  _onBorders(),
  _xcartBorders(),
  _octahedra_z(-1),
  _octahedra(),
  _hasTranslations(false),
  _display(DISP_BORDER),
  _bond(2.00), 
  _bondRadius(0.15), 
  _sphere(_opengl),
  _cylinder(_opengl),
  _arrow(_opengl),
  _up(),
  _down(),
  _octacolor(),
  _octaDrawAtoms(true),
  _drawSpins(),
  _maxDim(1.1)
{
  _up[0] = 1.0;
  _up[1] = 0.5;
  _up[2] = 0.0;
  _down[0] = 0.0;
  _down[1] = 1.0;
  _down[2] = 0.5;
  _octacolor[0] = 0.5f;
  _octacolor[1] = 0.5f;
  _octacolor[2] = 0.5f;
  _octacolor[3] = 0.8f;
  _drawSpins[0]=true;
  _drawSpins[1]=true;
  _drawSpins[2]=true;
}

//
CanvasPos::CanvasPos(CanvasPos &&canvas) : Canvas(std::move(canvas)),
  _natom(canvas._natom),
  _ntypat(canvas._ntypat),
  _typat(std::move(canvas._typat)),
  _znucl(std::move(canvas._znucl)),
  _onBorders(std::move(canvas._onBorders)),
  _xcartBorders(std::move(canvas._xcartBorders)),
  _octahedra_z(canvas._octahedra_z),
  _octahedra(std::move(canvas._octahedra)),
  _hasTranslations(canvas._hasTranslations),
  _display(DISP_BORDER),
  _bond(canvas._bond), 
  _bondRadius(canvas._bondRadius), 
  _sphere(std::move(canvas._sphere)),
  _cylinder(std::move(canvas._cylinder)),
  _arrow(std::move(canvas._arrow)),
  _up(),
  _down(),
  _octacolor(),
  _octaDrawAtoms(canvas._octaDrawAtoms),
  _drawSpins(),
  _maxDim(canvas._maxDim)
{
  _up[0] = canvas._up[0];
  _up[1] = canvas._up[1];
  _up[2] = canvas._up[2];
  _down[0] = canvas._down[0];
  _down[1] = canvas._down[1];
  _down[2] = canvas._down[2];
  _octacolor[0] = canvas._octacolor[0];
  _octacolor[1] = canvas._octacolor[1];
  _octacolor[2] = canvas._octacolor[2];
  _octacolor[3] = canvas._octacolor[3];
  _drawSpins[0] = canvas._drawSpins[0];
  _drawSpins[1] = canvas._drawSpins[1];
  _drawSpins[2] = canvas._drawSpins[2];
  
  if ( !_octahedra.empty() ) {
    for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
      Octahedra *tmpocta = new Octahedra(std::move(*dynamic_cast<Octahedra*>(_octahedra[i].get())));
      _octahedra[i].reset(tmpocta);
    }
  }

}

//
CanvasPos::~CanvasPos() {
  this->clear();
}

void CanvasPos::clear() {
  _natom = 0;
  _ntypat = 0;
  _octahedra_z = -1;
  _hasTranslations = false;
  _typat.clear();
  _znucl.clear();
  _onBorders.clear();
  _xcartBorders.clear();
  _octahedra.clear();
}

//
void CanvasPos::setHist(HistData& hist) {
  try {
    this->clear();
    _natom = hist.natom();
    while ( hist.ntimeAvail() < 1 ) {
#ifdef HAVE_CPPTHREAD_YIELD
      std::this_thread::yield();
#endif
    }
    
    unsigned ntime = hist.ntimeAvail();
    _znucl = hist.znucl();
    _ntypat = _znucl.size();
    _znucl.insert(_znucl.begin(),0);
    _typat = hist.typat();
    if ( _ntime == _tend ) _tend = -1;
    _tbegin = 0;
    Canvas::setNTime(ntime);

    std::clog << "Using file " << hist.filename() << std::endl;
    std::clog << "Number of atoms: " << _natom << std::endl;
    std::clog << "Number of ionic steps: " << hist.ntime() << std::endl;

    const double *rprimd0 = hist.getRprimd(0);

    // Find atoms on borders
    if ( geometry::det(rprimd0) > 1e-6 ) {
      _hasTranslations = true;
      _info = hist.filename();

      //*
      const double *ptrR = hist.getXred(0);
      double normvec = 0.0;
      double nx = ( (normvec = geometry::norm({{
              rprimd0[0],
              rprimd0[3],
              rprimd0[6] }})) < 1e-6 ? 0.0 : 1.2/normvec);
      double ny = ( (normvec = geometry::norm({{
              rprimd0[1],
              rprimd0[4],
              rprimd0[7] }})) < 1e-6 ? 0.0 : 1.2/normvec);
      double nz = ( (normvec = geometry::norm({{
              rprimd0[2],
              rprimd0[5],
              rprimd0[8] }})) < 1e-6 ? 0.0 : 1.2/normvec);
      for ( int iatom = 0 ; iatom < _natom ; ++iatom ) {
        geometry::vec3d shift({{0,0,0}});
        if ( ptrR[iatom*3  ] <= nx ) {
          shift[0] = 1;
          _onBorders.push_back(std::make_pair(iatom, shift));
          if ( ptrR[iatom*3+1] <= ny ) {
            shift[1] = 1;
            _onBorders.push_back(std::make_pair(iatom, shift));
            if ( ptrR[iatom*3+2] <= nz ) {
              shift[2] = 1;
              _onBorders.push_back(std::make_pair(iatom, shift));
            }
          }
          shift[1] = shift[2] = 0;
          if ( ptrR[iatom*3+2] <= nz ) {
            shift[2] = 1;
            _onBorders.push_back(std::make_pair(iatom, shift));
          }
        }
        shift = {{0,0,0}};
        if ( ptrR[iatom*3+1] <= ny ) {
          shift[1] = 1;
          _onBorders.push_back(std::make_pair(iatom, shift));
          if ( ptrR[iatom*3+2] <= nz ) {
            shift[2] = 1;
            _onBorders.push_back(std::make_pair(iatom, shift));
          }
        }
        shift = {{0,0,0}};
        if ( ptrR[iatom*3+2] <= nz ) {
          shift[2] = 1;
          _onBorders.push_back(std::make_pair(iatom, shift));
        }
      }

      for ( auto &atom : _onBorders ) {
        _typat.push_back(_typat[atom.first]);
      }
      _xcartBorders.resize(_onBorders.size()*3);
      //*/
    }
    else { // No rprimd
      _hasTranslations = false;
      std::clog << "Translations are not available." << std::endl;
      _info = hist.filename();
    }

    // Build octahedra
    _histdata.reset(&hist);
    this->updateOctahedra(_octahedra_z);

    _itime = _tbegin;
    if ( _natom > 0 )
      std::clog << "Starting at time step " << _itime << std::endl;
    typicalDim(-1.f);
  }
  catch (Exception& e) {
    e.ADD("Failed to set _HIST in canvas",ERRDIV);
    throw e;
  }
}

//
void CanvasPos::updateHist() {
  unsigned ntimeAvail = _histdata->ntimeAvail();
  if ( _tend == _ntime ) _tend = -1;
  Canvas::setNTime(ntimeAvail);
}

//
void CanvasPos::updateOctahedra(int z) {
  if ( z > 0 && (_natom+_onBorders.size()) > 6 && _hasTranslations) {
    _octahedra_z = z;
    _octahedra.clear();
    std::vector<double> xcartTotal((_natom+_onBorders.size())*3);
    const double *histXcart = _histdata->getXcart(0);
    const double *xred = _histdata->getXred(0);
    const double *rprimd = _histdata->getRprimd(0);

    this->buildBorders(0);
    std::copy(&histXcart[0],&histXcart[_natom*3],&xcartTotal[0]);
    std::copy(&_xcartBorders[0],&_xcartBorders[_onBorders.size()*3],&xcartTotal[_natom*3]);

    try {
      for ( unsigned iatom = 0 ; iatom < _natom+_onBorders.size() ; ++iatom )
        if ( _znucl[_typat[iatom]] == _octahedra_z ) {
          Octahedra *tmpocta = new Octahedra(iatom,_natom,xred,&xcartTotal[0],rprimd,_opengl);
          _octahedra.push_back(std::unique_ptr<Octahedra>(tmpocta)); //Construct octahedra based on xcart at time 0
        }
    }
    catch (Exception &e) {
      e.ADD("Abording construction of octahedra",ERRDIV);
      throw e;
    }
  }
  else if ( z == -1 || z == 0) {
    _octahedra.clear();
    _octahedra_z = -1;
  }
  else{
    throw EXCEPTION("Not enough data to build octahedra",ERRDIV);
  }
}

//
void CanvasPos::refresh(const geometry::vec3d &cam, TextRender &render) {
  if ( _natom < 1 ) return;
  this->updateHist();
  if ( ! _opengl ) return;
#ifdef HAVE_GL
  const double *rprimd = _histdata->getRprimd(_itime);
  const double *xcart = _histdata->getXcart(_itime);
  const GLfloat fx[] = {(GLfloat)rprimd[0], (GLfloat)rprimd[3], (GLfloat)rprimd[6]};
  const GLfloat fy[] = {(GLfloat)rprimd[1], (GLfloat)rprimd[4], (GLfloat)rprimd[7]};
  const GLfloat fz[] = {(GLfloat)rprimd[2], (GLfloat)rprimd[5], (GLfloat)rprimd[8]};

  glPushMatrix();
  CanvasPos::drawCell();

  this->buildBorders(_itime);

  /* Find bond here */
  std::vector< std::pair<int ,int> > bonds; // iatom/distance
  if ( _display & DISP_BOND ) {
    bonds = this->buildBonds();
  }


  _sphere.push();
  for ( int i = 0 ; i < _translate[0] ; ++i) {
    const GLfloat fi = (GLfloat) i;
    const GLfloat xtrans[] = { fi*fx[0], fi*fx[1], fi*fx[2] };
    const bool ex = (i==_translate[0]-1);
    for ( int j = 0 ; j < _translate[1] ; ++j) {
      const GLfloat fj = (GLfloat) j;
      const GLfloat ytrans[] = { fj*fy[0]+xtrans[0], fj*fy[1]+xtrans[1], fj*fy[2]+xtrans[2] };
      const bool ey = (j==_translate[1]-1); 
      for ( int k = 0 ; k < _translate[2] ; ++k ) {
        const GLfloat fk = (GLfloat) k;
        const GLfloat ztrans[] = { fk*fz[0]+ytrans[0], fk*fz[1]+ytrans[1], fk*fz[2]+ytrans[2] };
        const bool ez = (k==_translate[2]-1);
        glPushMatrix();
        glTranslatef(ztrans[0],ztrans[1],ztrans[2]);
        // Erase color 
        CanvasPos::drawAtom(-1,0.f,0.f,0.f);
        for ( int iatom = 0 ; iatom < _natom ; ++iatom ) {
          const int znucl = _znucl[_typat[iatom]];
          CanvasPos::drawAtom(znucl,
              xcart[iatom*3],
              xcart[iatom*3+1],
              xcart[iatom*3+2]);
          if ( _display & (DISP_NAME | DISP_ID | DISP_ZNUCL) ) {
            std::stringstream label;
            if ( _display & DISP_NAME ) label << mendeleev::name[znucl];
            else if ( _display & DISP_ID   ) label << iatom+1;
            else if ( _display & DISP_ZNUCL ) label << znucl;
            glDisable(GL_DEPTH_TEST);
            glRasterPos3f(
                xcart[iatom*3+0],
                xcart[iatom*3+1],
                xcart[iatom*3+2]);
            render.render(utils::trim(label.str()),true);
            glEnable(GL_DEPTH_TEST);
          }
        }
        _sphere.pop();
        this->drawSpins();
        this->drawBonds(bonds);
        _sphere.push();
        // Erase color 
        CanvasPos::drawAtom(-1,0.f,0.f,0.f);
        if ( (_display & DISP_BORDER ) && ( ex || ey || ez ) ) {
          std::vector<unsigned> drawBorder;
          if ( (ex && !ey && !ez) || (!ex && ey && !ez) || (!ex && !ey && ez) ) {
            const double bx = (ex ? 1.0 : 0.0);
            const double by = (ey ? 1.0 : 0.0);
            const double bz = (ez ? 1.0 : 0.0);
            for ( unsigned iatom = 0 ; iatom < _onBorders.size() ; ++iatom ) {
              if ( _onBorders[iatom].second[0] == bx 
                  && _onBorders[iatom].second[1] == by 
                  && _onBorders[iatom].second[2] == bz ) {
                drawBorder.push_back(iatom);
              }
            }
          }
          else if ( (ex && ey && !ez) || (ex && !ey && ez) || (!ex && ey && ez) ) {
            const double bx = (ex ? 0.0 : 1.0);
            const double by = (ey ? 0.0 : 1.0);
            const double bz = (ez ? 0.0 : 1.0);
            for ( unsigned iatom = 0 ; iatom < _onBorders.size() ; ++iatom ) {
              if ( (bx && (_onBorders[iatom].second[0]==0.0))
                  || (by && (_onBorders[iatom].second[1]==0.0))
                  || (bz && (_onBorders[iatom].second[2]==0.0)) ) {
                drawBorder.push_back(iatom);
              }
            }
          }
          else { // 3
            for ( unsigned iatom = 0 ; iatom < _onBorders.size() ; ++iatom )
              drawBorder.push_back(iatom);
          }
          for ( auto batom : drawBorder ) {
            const int znucl = _znucl[_typat[_natom+batom]];
            CanvasPos::drawAtom(znucl,
                _xcartBorders[batom*3+0],
                _xcartBorders[batom*3+1],
                _xcartBorders[batom*3+2]);
            if ( _display & (DISP_NAME | DISP_ID | DISP_ZNUCL) ) {
              std::stringstream label;
              if ( _display & DISP_NAME ) label << mendeleev::name[znucl];
              else if ( _display & DISP_ID   ) label << _onBorders[batom].first+1;
              else if ( _display & DISP_ZNUCL ) label << znucl;
              glDisable(GL_DEPTH_TEST);
              glRasterPos3f(
                  _xcartBorders[batom*3+0],
                  _xcartBorders[batom*3+1],
                  _xcartBorders[batom*3+2]);
              render.render(utils::trim(label.str()),true);
              glEnable(GL_DEPTH_TEST);
            }
          }
          _sphere.pop();
          for ( auto batom : drawBorder ) {
            this->drawSpins(batom);
          }
          _sphere.push();
        }
        glPopMatrix();
      }
    }
  }

  // Erase color
  CanvasPos::drawAtom(-1,0.f,0.f,0.f);
  _sphere.pop();

  std::vector< std::pair<unsigned,double> > drawOrder;
  for( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
    const int id = _octahedra[i]->center();
    geometry::vec3d center;
    if ( id < _natom ) {
    center = {{ cam[0]-xcart[id*3], 
      cam[1]-xcart[id*3+1], 
      cam[2]-xcart[id*3+2] }};
    }
    else {
    center = {{ cam[0]-_xcartBorders[(id-_natom)*3], 
      cam[1]-_xcartBorders[(id-_natom)*3+1], 
      cam[2]-_xcartBorders[(id-_natom)*3+2] }};
    }
    drawOrder.push_back(std::make_pair(i,geometry::dot(cam,center)));
  }
  std::sort(drawOrder.begin(), drawOrder.end(), []
      (std::pair<unsigned,double> t1, std::pair<unsigned,double>t2) {
      return t1.second > t2.second;
      }
      );

  //glPopMatrix();

  for ( int k = 0 ; k < _translate[2] ; ++k ) {
    const GLfloat fk = (GLfloat) k;
    const GLfloat ztrans[] = { (fk)*fz[0], (fk)*fz[1],(fk)*fz[2] };
    for ( int j = 0 ; j < _translate[1] ; ++j) {
      const GLfloat fj = (GLfloat) j;
      const GLfloat ytrans[] = { ztrans[0]+(fj)*fy[0], ztrans[1]+(fj)*fy[1],ztrans[2]+(fj)*fy[2] };
      for ( int i = 0 ; i < _translate[0] ; ++i) {
        const GLfloat fi = (GLfloat) i;
        const GLfloat xtrans[] = { ytrans[0]+(fi)*fx[0], ytrans[1]+(fi)*fx[1],ytrans[2]+(fi)*fx[2] };
        glPushMatrix();
        glTranslatef(xtrans[0],xtrans[1],xtrans[2]);

        Octahedra::u3f na;
        if (  k == 0 && j == 0 && i == 0 ) {
          for( auto& iocta : drawOrder )
            if ( (_display & DISP_BORDER ) || ( _octahedra[iocta.first]->center() < _natom ) )
              _octahedra[iocta.first]->build(rprimd,xcart, na);
        }
        else {
          for( auto& iocta : drawOrder )
            if ( (_display & DISP_BORDER ) || ( _octahedra[iocta.first]->center() < _natom ) )
              _octahedra[iocta.first]->build(rprimd,nullptr, na);
        }

        if ( _octaDrawAtoms && !_octahedra.empty() ) {
          _sphere.push();
          for ( auto &a : na ) {
            CanvasPos::drawAtom(_znucl[_typat[a.first]],a.second[0],a.second[1],a.second[2]);
            if ( _display & (DISP_NAME | DISP_ID | DISP_ZNUCL) ) {
              std::stringstream label;
              if ( _display & DISP_NAME ) label << mendeleev::name[_znucl[_typat[a.first]]];
              else if ( _display & DISP_ID   ) label << a.first+1;
              else if ( _display & DISP_ZNUCL ) label << _znucl[_typat[a.first]];
              glDisable(GL_DEPTH_TEST);
              glRasterPos3f(
                  a.second[0],
                  a.second[1],
                  a.second[2]);
              render.render(utils::trim(label.str()),true);
              glEnable(GL_DEPTH_TEST);
            }
          }
          CanvasPos::drawAtom(-1,0.f,0.f,0.f);
          _sphere.pop();
        }

        if ( _light ) glDisable(GL_LIGHTING);
        for( auto& iocta : drawOrder )
          if ( (_display & DISP_BORDER ) || ( _octahedra[iocta.first]->center() < _natom ) )
            _octahedra[iocta.first]->draw(_octacolor);
        if ( _light ) glEnable(GL_LIGHTING);

        glPopMatrix();
      }
    }
  }
  glPopMatrix();


  glFlush();
#else
  (void) cam;
  (void) render;
#endif
}

void CanvasPos::nextFrame(const int count) {
  Status before = _status;
  Canvas::nextFrame(count);
  if ( before == START && _histdata.get() != nullptr && _ntime < (int)_histdata->ntime() )
    _status = START;
}

//
void CanvasPos::drawAtom(const int znucl, GLfloat posX, GLfloat posY, GLfloat posZ) {
#if defined(HAVE_GL)
  static int zprev = -1;
  if ( zprev != znucl ) {
    zprev = znucl;
    if ( zprev == -1 ) return;
    glColor3f(mendeleev::color[znucl][0], mendeleev::color[znucl][1], mendeleev::color[znucl][2]);
  }
  glPushMatrix();
  glTranslatef(posX,posY,posZ);
  if ( znucl >= 0 ) 
    _sphere.draw((float)mendeleev::radius[znucl]);
  glPopMatrix();
#else
  (void) znucl;
  (void) posX;
  (void) posY;
  (void) posZ;
#endif
}

//
void CanvasPos::drawCell() {
#ifdef HAVE_GL

  const double *rprimd = _histdata->getRprimd(_itime);
  const GLfloat x[] = {(GLfloat)rprimd[0], (GLfloat)rprimd[3], (GLfloat)rprimd[6]};
  const GLfloat y[] = {(GLfloat)rprimd[1], (GLfloat)rprimd[4], (GLfloat)rprimd[7]};
  const GLfloat z[] = {(GLfloat)rprimd[2], (GLfloat)rprimd[5], (GLfloat)rprimd[8]};
  const GLfloat xy[] = {x[0]+y[0], x[1]+y[1], x[2]+y[2]};
  const GLfloat yz[] = {y[0]+z[0], y[1]+z[1], y[2]+z[2]};
  const GLfloat zx[] = {z[0]+x[0], z[1]+x[1], z[2]+x[2]};
  const GLfloat xyz[] = {x[0]+y[0]+z[0], x[1]+y[1]+z[1], x[2]+y[2]+z[2]};

  const GLfloat tx = static_cast<GLfloat>(-(_translate[0]*0.5f));
  const GLfloat ty = static_cast<GLfloat>(-(_translate[1]*0.5f));
  const GLfloat tz = static_cast<GLfloat>(-(_translate[2]*0.5f));

  if ( _hasTranslations ) {
    glTranslatef(
        (tx*x[0]+ty*y[0]+tz*z[0]),
        (tx*x[1]+ty*y[1]+tz*z[1]),
        (tx*x[2]+ty*y[2]+tz*z[2]));
    if ( _light ) glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3d(0.0,0.0,0.0);
    glVertex3d(x[0],x[1],x[2]);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3d(0.0,0.0,0.0);
    glVertex3d(y[0],y[1],y[2]);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3d(0.0,0.0,0.0);
    glVertex3d(z[0],z[1],z[2]);
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex3d(x[0],x[1],x[2]);
    glVertex3d(zx[0],zx[1],zx[2]);
    glVertex3d(y[0],y[1],y[2]);
    glVertex3d(yz[0],yz[1],yz[2]);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex3d(x[0],x[1],x[2]);
    glVertex3d(xy[0],xy[1],xy[2]);
    glVertex3d(xyz[0],xyz[1],xyz[2]);
    glVertex3d(zx[0],zx[1],zx[2]);
    glVertex3d(z[0],z[1],z[2]);
    glVertex3d(yz[0],yz[1],yz[2]);
    glVertex3d(xyz[0],xyz[1],xyz[2]);
    glVertex3d(xy[0],xy[1],xy[2]);
    glVertex3d(y[0],y[1],y[2]);
    glEnd();
    if ( _light ) glEnable(GL_LIGHTING);
  }
  else {
    double xmean = 0.0, ymean = 0.0, zmean = 0.0;
    double masstot = 0.0;
    const double *xcart = _histdata->getXcart(_itime);
    for ( int iatom = 0 ; iatom < _natom ; ++iatom ) {
      masstot += mendeleev::mass[_znucl[_typat[iatom]]];
      xmean += (xcart[3*iatom  ]*mendeleev::mass[_znucl[_typat[iatom]]]);
      ymean += (xcart[3*iatom+1]*mendeleev::mass[_znucl[_typat[iatom]]]);
      zmean += (xcart[3*iatom+2]*mendeleev::mass[_znucl[_typat[iatom]]]);
    }
    xmean /= masstot;
    ymean /= masstot;
    zmean /= masstot;
    glTranslatef((GLfloat)-xmean,(GLfloat)-ymean,(GLfloat)-zmean);

  }

#endif
}

//
float CanvasPos::typicalDim(float reset) {
  _maxDim *= reset;
  if ( _ntime > 0 && _maxDim < 0.f ) {
    float acell[3];
    const double *rprimd = _histdata->getRprimd(0);
    const double *xcart = _histdata->getXcart(0);
    acell[0] = static_cast<float>(_translate[0]*geometry::norm({{rprimd[0],rprimd[3],rprimd[6]}}));
    acell[1] = static_cast<float>(_translate[1]*geometry::norm({{rprimd[1],rprimd[4],rprimd[7]}}));
    acell[2] = static_cast<float>(_translate[2]*geometry::norm({{rprimd[2],rprimd[5],rprimd[8]}}));
    _maxDim = *std::max_element(acell,acell+3);
    if ( std::fabs(_maxDim) < 0.000001f ) _maxDim = 1.1f * *std::max_element(xcart,xcart+_natom);
    return _maxDim;
  }
  else if ( _ntime <= 0 ) return 1.f;
  else return _maxDim;
}

void CanvasPos::drawBonds(std::vector< std::pair<int,int> >& bonds) {
#ifdef HAVE_GL
  if ( !bonds.empty() ) {
    using namespace geometry;
    _cylinder.push();
    const double *xcart = _histdata->getXcart(_itime);
    for ( auto hb = bonds.begin() ; hb != bonds.end() ; ++hb ){
      const int i1 = hb->first;
      const int i2 = hb->second;
      const unsigned t1 = _typat[i1];
      const unsigned t2 = _typat[i2];
      const double r1 = mendeleev::radius[_znucl[t1]];
      const double r2 = mendeleev::radius[_znucl[t2]];
      const float *c1 = mendeleev::color[_znucl[t1]];
      const float *c2 = mendeleev::color[_znucl[t2]];
      const double prop = 1./(r1+r2);
      const double *xcart1 = ( i1 < _natom ) ? &xcart[3*i1] : &_xcartBorders[3*(i1-_natom)];
      const double *xcart2 = ( i2 < _natom ) ? &xcart[3*i2] : &_xcartBorders[3*(i2-_natom)];
      double middle[3];
      for ( unsigned i = 0 ; i < 3 ; ++i )
        middle[i] = r2*prop*xcart1[i]+r1*prop*xcart2[i];
      glColor3f(c1[0],c1[1],c1[2]);
      _cylinder.draw(xcart1,middle,_bondRadius);
      glColor3f(c2[0],c2[1],c2[2]);
      _cylinder.draw(middle,xcart2,_bondRadius);
    }
    _cylinder.pop();
  }
#else 
  (void) bonds;
#endif
}

//
void CanvasPos::drawSpins(unsigned batom) {
#ifdef HAVE_GL
  auto drawOneSpeen = [&](const double spin[3], GLfloat x, GLfloat y, GLfloat z, int znucl) {
    if ( (spin[0]*spin[0]+spin[1]*spin[1]+spin[2]*spin[2]) > 0.0099 ) {
      glPushMatrix();
      const float spinx = ( _drawSpins[0] ? static_cast<float>(spin[0]) : 0.f );
      const float spiny = ( _drawSpins[1] ? static_cast<float>(spin[1]) : 0.f );
      const float spinz = ( _drawSpins[2] ? static_cast<float>(spin[2]) : 0.f );
      const float nn = sqrt(spinx*spinx+spiny*spiny+spinz*spinz);
      const float length = 2.f*(float)mendeleev::radius[znucl]*(1.f+nn*0.5f);
      const float inv_nn = 1.f/nn;
      const float nprod = sqrt(spiny*spiny+spinx*spinx);
      const float angle = ( (spinz<0.) ? 180.f+180.f/3.14f*asin(nprod*inv_nn) : -180.f/3.14f*asin(nprod*inv_nn) );
      const float scale=0.5f*length/nn;
      const float tol = ( (std::fabs(spinx)+std::fabs(spiny)) > 0.001f ? 0.0f : 1.0f );

      glTranslatef(x-spinx*scale,y-spiny*scale,z-spinz*scale);
      glRotatef(angle,spiny+tol,-spinx+tol,0.0f); // Keep the +1e-7 to avoid a rotation around (0,0,0)
      if ( spinz > 0 ) {
        glColor3fv(&_up[0]);
      }
      else { 
        glColor3fv(&_down[0]);
      }

      _arrow.draw((float)mendeleev::radius[1]/2.,length);
      glPopMatrix();
    }
  };

  if ( _histdata->getSpinat(_itime) != nullptr  ) {
    const double *xcart = _histdata->getXcart(_itime);
    const double *spinat = _histdata->getSpinat(_itime);
    if ( batom != (unsigned)-1 ) { // Only for atoms on border
      _arrow.push();
      drawOneSpeen(&spinat[_onBorders[batom].first*3],
          _xcartBorders[3*batom+0],
          _xcartBorders[3*batom+1],
          _xcartBorders[3*batom+2],
          _znucl[_typat[_natom+batom]]
          );
      _arrow.pop();
    }
    else {
      _arrow.push();
      for ( int iatom = 0 ; iatom < _natom ; ++iatom ){
        drawOneSpeen(&spinat[iatom*3],
            static_cast<GLfloat>(xcart[3*iatom+0]),
            static_cast<GLfloat>(xcart[3*iatom+1]),
            static_cast<GLfloat>(xcart[3*iatom+2]),
            _znucl[_typat[iatom]]
            );
      }
      _arrow.pop();
    }
  }
#else
  (void) batom;
#endif
}


//
void CanvasPos::my_alter(std::string token, std::istringstream &stream) {
  std::ostringstream out;
  if ( token == "octa_z" || token == "octahedra_z" ) {
    int z = 0;
    auto pos = stream.tellg();
    stream >> z;
    if ( stream.fail() ) {
      stream.clear();
      stream.seekg(pos);
      std::string name;
      stream >> name;
      if ( !stream.fail() ) {
        z = mendeleev::znucl(name);
      }
      else throw EXCEPTION("Cannot find element "+name,ERRDIV);
    }
    this->updateOctahedra(z);
    stream >> z;
    if ( !stream.fail() && (z == 1 || z == 0) ) {
      _octaDrawAtoms = (( z == 1 ) ? true : false);
    }
    else {
      stream.clear();
    }
  }
  else if ( token == "average" ) {
    HistData* hist = nullptr;
    HistData* backup = _histdata.release();
    try {
      hist = backup->average(_tbegin,_tend);
      auto save = _octahedra_z;
      this->setHist(*hist);
      this->updateOctahedra(save);
      if ( _status == PAUSE ) _status = UPDATE;
    }
    catch (Exception& e) {
      e.ADD("Could not get the averaged HistData object",ERRDIV);
      if ( hist != nullptr ) {
        delete hist;
        hist = nullptr;
      }
      _histdata.reset(backup);
      throw e;
    }
    //_histdata.reset(hist);
    if ( _histdata->ntime() == 1 ) _status = UPDATE;
  }
  else if ( token == "plot" || token == "print" || token == "data" ) {
    Graph::GraphSave save = Graph::GraphSave::NONE;
    if ( token == "print" )
      save = Graph::GraphSave::PRINT;
    else if ( token == "data" )
      save = Graph::GraphSave::DATA;

    try {
      if ( _gplot == nullptr ) 
        _gplot.reset(new Gnuplot);

      _gplot->setWinTitle(_info);
    }
    catch ( Exception &e ) {
      e.ADD("Unable to plot with gnuplot.\nInstead, writing data.", ERRWAR);
      std::cerr << e.fullWhat() << std::endl;
      _gplot.reset(nullptr);
    }

    size_t pos = stream.tellg();
    try {
      if ( _histdata.get() != nullptr ) {
        _histdata->plot(_tbegin,_tend, stream, _gplot.get(), save);
      }
      else
        throw EXCEPTION("",ERRABT);
    }
    catch( Exception &e ) {
      if ( e.getReturnValue() == ERRABT) {
        stream.clear();
        stream.seekg(pos);
        this->plot(_tbegin, _tend, stream,save);
      }
      else throw e;
    }
  }
  else if ( token == "u" || token == "update" ) {
    try{
      if ( !_info.empty() ) {
        auto save = _octahedra_z;
        this->openFile(_info);
        this->updateOctahedra(save);
      }
    }
    catch ( Exception &e ) {
      e.ADD("Error while reading file "+_info,ERRDIV);
      throw e;
    }
  }
  else if ( token == "c" || token == "color" ){
    unsigned c[3];
    float *tomodify = nullptr;
    int z;
    auto pos = stream.tellg();
    stream >> z;
    if ( stream.fail() ) {
      stream.clear();
      stream.seekg(pos);
      std::string name;
      stream >> name;
      if ( name == "up" ) tomodify = &_up[0];
      else if ( name == "down" ) tomodify = &_down[0];
      else if ( name == "octa" ) tomodify = &_octacolor[0];
      else if ( !stream.fail() ) {
        z = mendeleev::znucl(name);
        tomodify = &mendeleev::color[z][0];
      }
      else throw EXCEPTION("No color to define for "+name,ERRDIV);
    }
    else if ( z > 0 && z < 119 ) {
      tomodify = &mendeleev::color[z][0];
    }
    else throw EXCEPTION("Bad atomic number",ERRDIV);

    stream >> c[0] >> c[1] >> c[2];
    if ( !stream.fail() && c[0] < 256 && c[1] < 256 && c[2] < 256 ) {
      tomodify[0] = (float)c[0]/255.f;
      tomodify[1] = (float)c[1]/255.f;
      tomodify[2] = (float)c[2]/255.f;
      if ( tomodify == &_octacolor[0] ) {
        stream >> c[0];
        if ( !stream.fail() && c[0] < 101 ) 
          _octacolor[3] = (float)c[0]/100.f;
        stream.clear();
      }
    }
    else throw EXCEPTION("Bad color numbers",ERRDIV);
  }
  else if ( token == "centroid" ) {
    auto myHist = _histdata.release();
    try{
      myHist->centroid();
      auto save = _octahedra_z;
      this->setHist(*myHist);
      this->updateOctahedra(save);
    }
    catch ( Exception &e ) {
      _histdata.reset(myHist);
      throw e;
    }
  }
  else if ( token == "angle") {
    int iatom1, iatom2, iatom3 = 0;
    stream >> iatom1 >> iatom2 >> iatom3;
    if ( !stream.fail() ) {
      std::stringstream str;
      str << "Angle between atom " << iatom1 
        << ", atom " << iatom2 
        << " and atom " << iatom3 << " is " 
        << _histdata->getAngle(iatom1,iatom2,iatom3,_itime) << "[degree]";
      throw EXCEPTION(str.str(),ERRCOM);
    }
    else
      throw EXCEPTION("Need three valid ids",ERRDIV);
  }
  else if ( token == "dist" || token == "distance") {
    int iatom1, iatom2 = 0;
    stream >> iatom1 >> iatom2;
    if ( !stream.fail() ) {
      std::stringstream str;
      str << "Distance between atom " << iatom1 
        << " and atom " << iatom2 << " is " 
        << _histdata->getDistance(iatom1,iatom2,_itime) << "[bohr]";
      throw EXCEPTION(str.str(),ERRCOM);
    }
    else
      throw EXCEPTION("Need two valid ids",ERRDIV);
  }
  else if ( token == "show" ){
    std::string what;
    stream >> what;
    if ( !stream.fail() ) { 
      if ( what == "border" )
        _display |= DISP_BORDER;
      else if ( what == "name" ) 
        _display = (_display&~(DISP_ZNUCL | DISP_ID))|DISP_NAME;
      else if ( what == "znucl" )
        _display = (_display&~(DISP_NAME  | DISP_ID))|DISP_ZNUCL;
      else if ( what == "id" )
        _display = (_display&~(DISP_ZNUCL | DISP_NAME))|DISP_ID;
      else if ( what == "bond" )
        _display |= DISP_BOND;
      else
        throw EXCEPTION("Options for show not known",ERRDIV);
    }
  }
  else if ( token == "hide" ){
    std::string what;
    stream >> what;
    if ( !stream.fail() ) { 
      if ( what == "border" )
        _display &= ~DISP_BORDER;
      else if ( what == "name" )
        _display &= ~DISP_NAME;
      else if ( what == "znucl" )
        _display &= ~DISP_ZNUCL;
      else if ( what == "id" )
        _display &= ~DISP_ID;
      else if ( what == "bond" )
        _display &= ~DISP_BOND;
      else
        throw EXCEPTION("Options for show not known",ERRDIV);
    }
  }
  else if ( token == "div" || token == "division" ){
    if (_opengl) {
      unsigned int div;
      stream >> div;
      if ( !stream.fail() ) { 
        _sphere.division(div);
        _sphere.genUnit();
        _cylinder.division(div);
        _cylinder.genUnit();
        _arrow.division(div);
        _arrow.genUnit();
      }
    }
  }
  else if ( token == "bond" ) {
    double h;
    stream >> h;
    if ( !stream.fail() ) _bond = ( h < 1e-1 ) ? -1.0 : 1+h;
  }
  else if ( token == "rad" || token == "radius" ){
    double *tomodify = nullptr;
    int z;
    double radius;
    auto pos = stream.tellg();
    stream >> z;
    if ( stream.fail() ) {
      stream.clear();
      stream.seekg(pos);
      std::string name;
      stream >> name;
      if ( !stream.fail() ) {
        if ( name == "bond" ) {
          tomodify = &_bondRadius;
        }
        else {
          z = mendeleev::znucl(name);
          tomodify = &mendeleev::radius[z];
        }
      }
      else throw EXCEPTION("No radius to define for "+name,ERRDIV);
    }
    else if ( z > 0 && z < 119 ) {
      tomodify = &mendeleev::radius[z];
    }
    else throw EXCEPTION("Bad atomic number",ERRDIV);

    stream >> radius;
    if ( !stream.fail() && radius > 0.0 ) {
      *tomodify = radius;
    }
    else throw EXCEPTION("Radius must be positive",ERRDIV);
  }
  else if ( token == "move" || token == "mv" ){
    double red[3]={0};
    int iatom;
    stream >> iatom;
    --iatom;
    if ( stream.fail() || iatom >= _natom )
      throw EXCEPTION("Bad atomic number",ERRDIV);

    stream >> red[0] >> red[1] >> red[2];
    if ( !stream.bad() ) {
      if ( _histdata != nullptr ) {
        try {
          _histdata->moveAtom(_itime,iatom,red[0],red[1],red[2]);
        }
        catch ( Exception &e ) {
          e.ADD("Failed to move atom",ERRDIV);
          throw e;
        }
      }
      stream.clear();
    }
    else throw EXCEPTION("Usage is :move iatom X Y Z",ERRDIV);
  }
  else if ( token == "periodic" ) {
    if ( _histdata == nullptr ) 
      throw EXCEPTION("Load a file first !",ERRDIV);
    bool toPeriodic;
    stream >> toPeriodic;
    if ( !stream.fail() ) {
      _histdata->periodicBoundaries(toPeriodic);
      this->updateOctahedra(_octahedra_z);
    }
    else
      throw EXCEPTION("Could not read line", ERRDIV);
  }
  else if ( token == "thermo" || token == "thermodynamics" ) {
    try{
      if ( _histdata == nullptr ) 
        throw EXCEPTION("No data loaded",ERRDIV);
      _histdata->printThermo(_tbegin, _tend);
    }
    catch ( Exception &e ) {
      throw e;
    }
  }
  else if ( token == "spin" ) {
    _drawSpins[0] = false;
    _drawSpins[1] = false;
    _drawSpins[2] = false;
    bool exit = false;
    while (!stream.eof() && !exit) {
      char direction;
      stream >> direction;
      switch (direction) {
        case 'x' :
          _drawSpins[0] = true;
          break;
        case 'y' :
          _drawSpins[1] = true;
          break;
        case 'z' :
          _drawSpins[2] = true;
          break;
        case '\0' :
          exit = true;
          break;
        case ' ' :
          break;
        default :
          throw EXCEPTION("Bad direction", ERRDIV);
          break;
      }
    }
    stream.clear();
  }
  /*
  else if ( token == "dec" ) {
    unsigned ntime = 100;
    unsigned step = 100;
    double mu = 0.99;
    stream >> ntime >> step >> mu;
    if ( stream.fail() ) stream.clear();
    _histdata->decorrelate(_tbegin,_tend,ntime,50,mu,step);
  }
  */
  else if ( token == "spg" || token == "spacegroup" ) {
    if ( _histdata == nullptr ) 
      throw EXCEPTION("No data loaded",ERRDIV);
    unsigned spgnum = 0;
    std::string spgname;
    double tol;
    stream >> tol; // bohr
    if ( stream.fail() ) tol = 0.01;
    try{
      if ( _histdata == nullptr ) 
        throw EXCEPTION("You need to load a file first",ERRDIV);
      spgnum = _histdata->getSpgNum(_itime, tol, &spgname);
      out << "Space group " << spgnum << ": " << spgname;
      throw EXCEPTION(out.str(), ERRCOM);
    }
    catch ( Exception &e ) {
      throw e;
    }
  }
  else if ( token == "w" || token == "write" ) {
    if ( _histdata == nullptr ) 
      throw EXCEPTION("No data loaded",ERRDIV);
    std::string format, name;
    stream >> format;
    if ( !stream.fail() ) {
      Dtset *dttrial = nullptr;
      double tolerance = 0.0010; // bohr
      if ( format == "dtset" || format == "cif" ) {
        dttrial = new Dtset(*_histdata,_itime);
        if ( format == "cif" ) {
          double toltry;
          auto pos = stream.tellg();
          stream >> toltry;
          if ( stream.fail() ) {
            stream.clear();
            stream.seekg(pos);
          }
          else {
            tolerance = toltry;
          }
        }
      }
      else if ( format == "POSCAR" || format == "poscar" ) {
        dttrial = new Poscar(*_histdata,_itime);
      }
      else {
        throw EXCEPTION("Bad tokens for \"write\": write (dtset|poscar|cif) filename.",ERRDIV);
      }
      stream >> name;
      if ( stream.fail() ) throw EXCEPTION("Missing filename",ERRDIV);
      std::string primitive;
      stream >> primitive;
      if ( !stream.fail() ) {
        if ( primitive == "primitive" ) {
          dttrial->standardizeCell(true,tolerance);
        }
        else if ( primitive == "standard" ) {
          dttrial->standardizeCell(false,tolerance);
        }
      }
      unsigned int i = _translate[0];
      unsigned int j = _translate[1];
      unsigned int k = _translate[2];
      if ( i+j+k > 3 ) {
        *dttrial = Supercell(*dttrial,i,j,k);
      }
      ( format == "cif" ) ? dttrial->cif(name,tolerance) : dttrial->dump(name);
      out << "Input file " << name << " written.";
      throw EXCEPTION(out.str(), ERRCOM);
      delete dttrial;
    }
    else {
      throw EXCEPTION("Bad tokens for \"write\": write (dtset|poscar|cif) filename.",ERRDIV);
    }
  }
  else {
    throw EXCEPTION("Unknown token "+token,ERRCOM);
  }
}

void CanvasPos::plot(unsigned tbegin, unsigned tend, std::istream &stream, Graph::GraphSave save) {
  std::string function;
  Graph::Config config;
  (void) tbegin;
  (void) tend;

  stream >> function;
  std::vector<double> &x = config.x;
  std::list<std::vector<double>> &y = config.y;
  //std::list<std::pair<std::vector<double>,std::vector<double>>> &xy = config.xy;
  std::list<std::string> &labels = config.labels;
  std::vector<short> &colors = config.colors;
  std::string &filename = config.filename;
  std::string &xlabel = config.xlabel;
  std::string &ylabel = config.ylabel;
  std::string &title = config.title;
  bool &doSumUp = config.doSumUp;

  std::string line;
  size_t pos = stream.tellg();
  std::getline(stream,line);
  stream.clear();
  stream.seekg(pos);
  ConfigParser parser;
  parser.setContent(line);

  // band
  if ( function == "band" ) {
    if ( _gplot != nullptr )
      _gplot->setWinTitle("Band Structure");
    filename = "bandStruct";
    doSumUp = false;
    title = "Band Structure";
    xlabel = "k-path";
    std::clog << std::endl << " -- Band Structure --" << std::endl;
    std::string bandfile;
    stream >> bandfile;
    if ( stream.fail() )
      throw EXCEPTION("You need to specify a filename",ERRDIV);

    EigParser* eigparser;
    try {
      eigparser = EigParser::getEigParser(bandfile);
    }
    catch (Exception &e) {
      e.ADD("Unable to get an EigParser",ERRDIV);
      throw e;
    }
    double fermi = 0;
    try {
      fermi = parser.getToken<double>("fermi");
    }
    catch (Exception &e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND )
        throw e;
    }
    try {
      std::string strUnit = utils::tolower(parser.getToken<std::string>("eunit"));
      EigParser::Unit eunit;
      if ( strUnit == "ev" )
        eunit = EigParser::eV;
      else if ( strUnit == "ha" )
        eunit = EigParser::Ha;
      else if ( strUnit == "thz" )
        eunit = EigParser::THz;
      else if ( strUnit == "cm-1" )
        eunit = EigParser::pcm;
      eigparser->setUnit(eunit);

    }
    catch (Exception &e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND )
        throw e;
    }
    unsigned ignore = 0;
    try {
      ignore = parser.getToken<double>("ignore");
      if ( ignore >= eigparser->getNband() )
        throw EXCEPTION("ignore should be smaller than the number of bands",ERRDIV);
    }
    catch (Exception &e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND )
        throw e;
    }
    std::vector<unsigned> ndiv = eigparser->getNdiv();
    std::vector<std::string> kptlabels = eigparser->getLabels();
    try {
      std::string tok1 = parser.getToken<std::string>("ndiv");
      std::vector<std::string> ndivk = utils::explode(tok1,':');
      ndiv.clear();
      for ( auto div : ndivk )
        ndiv.push_back(utils::stoi(div));
    }
    catch (Exception &e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND )
        throw e;
    }
    try {
      std::string tok2 = parser.getToken<std::string>("labels");
      kptlabels = utils::explode(tok2,':');
    }
    catch (Exception &e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND )
        throw e;
    }
    switch (eigparser->getUnit()) {
      case EigParser::Unit::eV:
        ylabel = "Energy [eV]";
        break;
      case EigParser::Unit::Ha:
        ylabel = "Energy [Ha]";
        break;
      case EigParser::Unit::THz:
        ylabel = "Energy [THz]";
        break;
      case EigParser::Unit::pcm:
        ylabel = "Energy [cm-1]";
        break;
      default:
        ylabel = "Energy";
    }
    x = eigparser->getPath();
    for ( unsigned iband = ignore ; iband < eigparser->getNband() ; ++iband ) {
      y.push_back(std::move(eigparser->getBand(iband,fermi,1)));
      colors.push_back(0);
      labels.push_back("");
    }
    if ( eigparser->isPolarized() ) {
      *labels.begin() = "Spin 1";
      labels.push_back("Spin 2");
      for ( unsigned iband = ignore ; iband < eigparser->getNband() ; ++iband ) {
        y.push_back(std::move(eigparser->getBand(iband,fermi,2)));
        colors.push_back(1);
        labels.push_back("");
      }
      labels.pop_back();
    }
    double minval = *std::min_element(y.begin()->begin(),y.begin()->end());
    double maxval = *std::max_element(y.rbegin()->begin(),y.rbegin()->end());
    double min = minval-(maxval-minval)*0.1;
    double max = maxval+(maxval-minval)*0.1;
    std::stringstream tmp;
    if ( _gplot != nullptr ) {
      _gplot->setXRange(0,eigparser->getLength());
      _gplot->setYRange(min,max);
      _gplot->addArrow(0,0,eigparser->getLength(),0,false);
      unsigned kptsize = kptlabels.size();
      if ( ndiv.size() > 0 &&  (ndiv.size() == kptsize-1 || kptsize == 0 ) ) {
        if ( kptsize > 0 ) {
          _gplot->addXTic(kptlabels[0],0);
        }
        unsigned acc = 0;
        for ( unsigned i = 0 ; i < ndiv.size()-1 ; ++i ) {
          acc += ndiv[i];
          if ( acc >= x.size() )
            throw EXCEPTION("Something is wrong in you ndiv argument",ERRDIV);
          if ( kptsize > 0 ) {
            _gplot->addXTic(kptlabels[i+1],x[acc]);
          }
          _gplot->addArrow(x[acc],min,x[acc],max,false);
        }
        if ( kptsize > 0 ) {
          _gplot->addXTic(*kptlabels.rbegin(),eigparser->getLength());
        }
      }
      else if ( ndiv.size() > 0 &&  ndiv.size() != kptlabels.size()-1 ) {
        throw EXCEPTION("Number of ndiv not compatible with number of labels",ERRDIV);
      }
    }
    if ( save == Graph::GraphSave::DATA ) { 
      eigparser->dump(filename+".dat",EigParser::PRTKPT|EigParser::PRTIKPT);
      save = Graph::GraphSave::NONE;
    }
  }
  else if ( function == "tdep" ) {
    if ( _histdata == nullptr ) 
      throw EXCEPTION("No data loaded",ERRDIV);

    std::clog << std::endl << " -- TDEP --" << std::endl;

    Tdep tdep;

    HistDataMD *histmd;
    bool has_unitcell = false;
    if ( ( histmd = dynamic_cast<HistDataMD*>(_histdata.get()) ) ) {
      try {
        tdep.supercell(histmd);
        has_unitcell = true;
      }
      catch ( Exception &e ) {
        std::cerr << e.fullWhat() << std::endl;
      }
    }
    else
      throw EXCEPTION("Not a valid Hist file",ERRDIV);

    try {
      ( parser.getToken<bool>("debug") ) ? tdep.mode(Tdep::Mode::Debug) : tdep.mode(Tdep::Mode::Normal);
    }
    catch (...) {
      tdep.mode(Tdep::Mode::Normal);
    }

    try {
      tdep.step(parser.getToken<unsigned>("step"));
    }
    catch (...) {
    }

    try {
      tdep.rcut(parser.getToken<double>("rcut"));
    }
    catch (...) {
    }

    try {
      std::string unitcell = parser.getToken<std::string>("unitcell");
      auto uc = HistData::getHist(unitcell,true);
      tdep.unitcell(uc,0);
      delete uc;
    }
    catch ( Exception &e ) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND ) {
        e.ADD("Error with the unitcell definition",ERRABT);
        throw e;
      }
      else if ( !has_unitcell && e.getReturnValue() == ConfigParser::ERFOUND ) {
        e.add("Set the unitcell with the keyword unitcell FILENAME");
        throw e;
      }
    }
    tdep.tbegin(_tbegin);
    tdep.tend(_tend);

    std::clog << std::endl << "Running TDEP, this can take some time." << std::endl;
    tdep.tdep();
    std::clog << std::endl << "OK." << std::endl;

    auto save = _info;
    HistDataDtset *uc = new HistDataDtset(tdep.unitcell());
    this->setHist(*uc);
    _info = save;
    std::stringstream info("band phonon-bands.yaml "+line);
    try {
      if ( _gplot == nullptr ) 
        _gplot.reset(new Gnuplot);
      this->plot(_tbegin, _tend,info,Graph::GraphSave::NONE);
      return;
    }
    catch ( Exception &e ) {
      e.ADD("Unable to plot with gnuplot.\nInstead, writing data.", ERRWAR);
      std::cerr << e.fullWhat() << std::endl;
      _gplot.reset(nullptr);
    }
  }

  else {
    throw EXCEPTION(std::string("Function ")+function+std::string(" not available yet or your need to load a file first"),ERRABT);
  }
  config.save = save;
  Graph::plot(config,_gplot.get());
  if ( _gplot != nullptr )
    _gplot->clearCustom();
}

void CanvasPos::buildBorders(unsigned itime) {
  // Atoms on border
  const double *xred = _histdata->getXred(itime);
  const double *rprimd = _histdata->getRprimd(itime);

  /*
  double normvec = 0.0;
  const double nx = ( (normvec = geometry::norm({{
          rprimd[0],
          rprimd[3],
          rprimd[6] }})) < 1e-6 ? 0.0 : 1.2/normvec);
  const double ny = ( (normvec = geometry::norm({{
          rprimd[1],
          rprimd[4],
          rprimd[7] }})) < 1e-6 ? 0.0 : 1.2/normvec);
  const double nz = ( (normvec = geometry::norm({{
          rprimd[2],
          rprimd[5],
          rprimd[8] }})) < 1e-6 ? 0.0 : 1.2/normvec);
  for ( int iatom = 0 ; iatom < _natom ; ++iatom ) {
    geometry::vec3d shift({{0,0,0}});
    if ( xred[iatom*3  ] <= nx ) {
      shift[0] = 1;
      _onBorders.push_back(std::make_pair(iatom, shift));
      if ( xred[iatom*3+1] <= ny ) {
        shift[1] = 1;
        _onBorders.push_back(std::make_pair(iatom, shift));
        if ( xred[iatom*3+2] <= nz ) {
          shift[2] = 1;
          _onBorders.push_back(std::make_pair(iatom, shift));
        }
      }
      shift[1] = shift[2] = 0;
      if ( xred[iatom*3+2] <= nz ) {
        shift[2] = 1;
        _onBorders.push_back(std::make_pair(iatom, shift));
      }
    }
    shift = {{0,0,0}};
    if ( xred[iatom*3+1] <= ny ) {
      shift[1] = 1;
      _onBorders.push_back(std::make_pair(iatom, shift));
      if ( xred[iatom*3+2] <= nz ) {
        shift[2] = 1;
        _onBorders.push_back(std::make_pair(iatom, shift));
      }
    }
    shift = {{0,0,0}};
    if ( xred[iatom*3+2] <= nz ) {
      shift[2] = 1;
      _onBorders.push_back(std::make_pair(iatom, shift));
    }
  }

  _typat.resize(_natom);
  for ( auto &atom : _onBorders ) {
    _typat.push_back(_typat[atom.first]);
  }
  _xcartBorders.resize(_onBorders.size()*3);
  */

  int batom = 0;
  for ( auto &atom : _onBorders ) {
    GLfloat xredAtom[3] = {
      (GLfloat) (xred[atom.first*3  ]+atom.second[0]),
      (GLfloat) (xred[atom.first*3+1]+atom.second[1]),
      (GLfloat) (xred[atom.first*3+2]+atom.second[2])
    };
    _xcartBorders[batom*3+0] = rprimd[0]*xredAtom[0]+rprimd[1]*xredAtom[1]+rprimd[2]*xredAtom[2];
    _xcartBorders[batom*3+1] = rprimd[3]*xredAtom[0]+rprimd[4]*xredAtom[1]+rprimd[5]*xredAtom[2];
    _xcartBorders[batom*3+2] = rprimd[6]*xredAtom[0]+rprimd[7]*xredAtom[1]+rprimd[8]*xredAtom[2];
    ++batom;
  }
}


std::vector<std::pair<int,int>> CanvasPos::buildBonds() {
  std::vector< std::pair<int ,int> > bonds; // iatom/distance
  const double b2 = _bond*_bond;
  const double *xcart = _histdata->getXcart(_itime);
  int nimage = (int) _histdata->nimage();
  for ( int img = 0 ; img < nimage ; ++img ) {
    int first = img*_natom/nimage;
    int last = (img+1)*_natom/nimage;
    // Inside
    for ( int iatom = first ; iatom < last ; ++iatom ) {
      const unsigned typ1 = _typat[iatom];
      const double rad1 = mendeleev::radius[_znucl[typ1]];
      geometry::vec3d pos={{ xcart[3*iatom], xcart[3*iatom+1], xcart[3*iatom+2] }};
      for ( int hatom = iatom+1 ; hatom < last ; ++hatom ) {
        unsigned typ2 = _typat[hatom];
        const double blength = rad1 + mendeleev::radius[_znucl[typ2]];
        geometry::vec3d hpos={{ xcart[3*hatom]-pos[0], xcart[3*hatom+1]-pos[1], xcart[3*hatom+2]-pos[2] }};
        double norm2 = geometry::dot(hpos,hpos);
        if ( norm2 < (blength*blength)*b2 )
          bonds.push_back(std::make_pair(iatom,hatom));
      }
      // Inside - Border
      if (_display & DISP_BORDER ) {
        for ( unsigned batom = 0 ; batom <  _onBorders.size() ; ++batom ) {
          int atomref = _onBorders[batom].first;
          if ( atomref >= first && atomref < last ) {
            unsigned typ2 = _typat[atomref];
            const double blength = rad1 + mendeleev::radius[_znucl[typ2]];
            geometry::vec3d hpos={{ _xcartBorders[3*batom]-pos[0], _xcartBorders[3*batom+1]-pos[1], _xcartBorders[3*batom+2]-pos[2] }};
            double norm2 = geometry::dot(hpos,hpos);
            if ( norm2 < (blength*blength)*b2 )
              bonds.push_back(std::make_pair(iatom,_natom+batom));
          }
        }
      }
    }
    // Border
    if (_display & DISP_BORDER) {
      for ( unsigned batom1 = 0 ; batom1 <  _onBorders.size() ; ++batom1 ) {
        int atomref1 = _onBorders[batom1].first;
        if ( atomref1 >= first && atomref1 < last ) {
          const unsigned typ1 = _typat[atomref1];
          const double rad1 = mendeleev::radius[_znucl[typ1]];
          geometry::vec3d pos={{ _xcartBorders[3*batom1], _xcartBorders[3*batom1+1], _xcartBorders[3*batom1+2] }};
          // Border - Border
          for ( unsigned batom2 = 0 ; batom2 <  _onBorders.size() ; ++batom2 ) {
            int atomref2 = _onBorders[batom2].first;
            if ( atomref2 >= first && atomref2 < last ) {
              int typ2 = _typat[atomref2];
              const double blength = rad1 + mendeleev::radius[_znucl[typ2]];
              geometry::vec3d hpos={{ _xcartBorders[3*batom2]-pos[0], _xcartBorders[3*batom2+1]-pos[1], _xcartBorders[3*batom2+2]-pos[2] }};
              double norm2 = geometry::dot(hpos,hpos);
              if ( norm2 < (blength*blength)*b2 )
                bonds.push_back(std::make_pair(_natom+batom1,_natom+batom2));
            }
          }
        }
      }
    }
  }
  return bonds;
}


//
void CanvasPos::help(std::ostream &out) {
  using std::endl;
  using std::setw;
  out << endl << "-- Here are the commands related to positions mode --" << endl;
  out <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  out << setw(40) << ":average" << setw(59) << "Compute the average structure and display it. Reload the full histdata with :update or :u)." << endl;
  out << setw(40) << ":angle id1 id2 id3" << setw(59) << "Compute the angle between atom id1, atom id2 and atom id3." << endl;
  out << setw(40) << ":centroid" << setw(59) << "Compute the centroid of each atom if several images are available." << endl;
  out << setw(40) << ":c or :color S X Y Z" << setw(59) << "Set the color in RGB for atom S (atomic number or name) or spin with S=(up|down) or octahedra S=octa." << endl;
  out << setw(40) << " " << setw(59) << "For S=octa an additional alpha parameter can be set for opacity." << endl;
  out << setw(40) << " " << setw(59) << "In rotations mode (:mode rotations) S=(plus|minus)." << endl;
  out << setw(40) << ":div or :division number" << setw(59) << "Number of division to draw spheres (>1)." << endl;
  out << setw(40) << ":dist or :distance id1 id2" << setw(59) << "Compute the distance between atom id1 and atom id2." << endl;
  out << setw(40) << ":bond factor " << setw(59) << "Factor to find the bonded atoms." << endl;
  out << setw(40) << ":hide WHAT" << setw(59) << "Hide WHAT=(border|name|znucl|id)" << endl;
  out << setw(40) << ":mv or :move iatom X Y Z" << setw(59) << "Move the atom iatom at the new REDUCED coordinate (X,Y,Z)" << endl;
  out << setw(40) << ":octa_z or :octahedra_z Z A" << setw(59) << "To draw an octahedron around the atoms Z (atomic numbers or name) A=(0|1) to draw the atoms at the tops." << endl;
  out << setw(40) << ":periodic (0|1)" << setw(59) << "Move all the atoms inside the celle (1) or make a continuous trajectory (0)" << endl;
  out << setw(40) << ":rad or :radius S R" << setw(59) << "Set the radius of atom S (atomic number or name) to R bohr." << endl;
  out << setw(40) << ":s or :speed factor" << setw(59) << "Velocity scaling factor to change the animation speed." << endl;
  out << setw(40) << ":show WHAT" << setw(59) << "Show WHAT=(border|name|znucl|id)" << endl;
  out << setw(40) << ":spin COMPONENTS" << setw(59) << "Specify what component of the spin to draw (x,y,z,xy,yz,xz,xyz)" << endl;
  out << setw(40) << ":spg or :spacegroup [tol]" << setw(59) << "Get the space group number and name. Tol is the tolerance for the symmetry finder." << endl;
  out << setw(40) << ":thermo or thermodynamics" << setw(59) << "Print total energy, volume, temperature, pressure averages (Only available for _HIST files)." << endl;
  out << setw(40) << ":w or :write (dtset|poscar|cif) filename [primitive]" << setw(59) << "Write the current structure in the desired format into filename file." << endl;
  out << setw(40) << "" << setw(59) << "Add the keyword primitive to save a primitive cell of the given current structure." << endl;
}
