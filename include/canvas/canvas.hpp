/**
 * @file include/canvas.hpp
 *
 * @brief Prototype for a canvas that draw with OpenGL
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


#ifndef CANVAS_HPP
#define CANVAS_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <memory>
#include "hist/histdata.hpp"
#include "base/geometry.hpp"
#include "io/configparser.hpp"
#include "io/eigparser.hpp"
#include "hist/histdata.hpp"
#include "graphism/triobj.hpp"
#include "graphism/textrender.hpp"
#include "plot/graph.hpp"

/** 
 * Pure virtual class to handle different type of properties (positions, forces, ...?)
 */
class Canvas {

  protected :
    /**
     * Current status of the animation
     */
    enum Status { START, PAUSE, UPDATE };

    bool _wait;       ///< Shall we ask HistData to wait until the full file is loaded ?
    bool _light;      ///< Ligh is on ?
    bool _opengl;     ///< Do we use OpenGl ?
    int  _ntime;      ///< Number of time step for the animation (MD step)
    int  _tbegin;     ///< First time to consider
    int  _tend;       ///< last time to consider (exclude)
    int  _itime;      ///< Current time (/_ntime)
    int  _nLoop;      ///< How many times do we play the sequence ? -1 (default) is infinite
    int  _iLoop;      ///< How many times have we already play the sequence.
    int  _dir;        ///< Direction of the animation
    int _ndiv;
    Status  _status;     ///< Start/Pause/...
    int _translate[3];   ///< Translation vector to repeat the cell
    std::string _info;   ///< Basically save the name of the file we use.
    TriObj::Drawing  _objDraw; ///< The way things are rendered ( GILL or SILHOUETTE)
    std::unique_ptr<HistData> _histdata; ///< HistData saved in case need to output something

    std::unique_ptr<Graph> _gplot; ///< Handle to plot things if necessary;
    std::unique_ptr<EigParser> _eigparser; ///< Handle to plot things if necessary;

    /**
     * Alter/modify a canvas with respect to a keyword and stream containing data.
     * @param token keyword to know what to do
     * @param stream the stream with the other data to be parsed.
     */
    virtual void my_alter(std::string token, std::istringstream &stream) = 0;



    /**
     * A simple class to store atom properties and operate easily on it.
     */
    class Atom {

      private:

        int              _id;      ///< A number ID to represent the atom (usually the atom position in the initial cell
        int              _typat;   ///< Type of the atom
        std::array<float,3>  _xcart;   ///< Position of the atom (can be in cartesian or reduced... user preference

      public:

        /**
         * Build a simple atom
         * @param id Number to represent the atom
         * @param typat Type of the atom
         * @param x x coordinate in the basis.
         * @param y y coordinate in the basis.
         * @param z z coordinate in the basis.
         */
        Atom(int id, int typat, float x, float y, float z);

        /**
         * Simple destructor that does nothing
         */
        ~Atom(){;}

        /**
         * Get the x composant
         * @return the x composant of the atomc position
         */
        inline float x() const {return _xcart[0];}

        /**
         * Get the x composant
         * @return the x composant of the atomc position
         */
        inline float y() const {return _xcart[1];}

        /**
         * Get the x composant
         * @return the x composant of the atomc position
         */
        inline float z() const {return _xcart[2];}

        /**
         * Get the x composant
         * @return the x composant of the atomc position
         */
        inline int id() const {return _id;}

        /**
         * Get the type of the atom
         * @return the typ of the atom
         */
        inline int typat() const {return _typat;}

        /**
         * Fonction to be used when sorting a set of atom (<)
         * @return true if atom1 is before atom2 on the x axis
         */
        inline static bool sortX(const Atom& atom1, const Atom& atom2)
        { return atom1._xcart[0] < atom2._xcart[0]; }

        /**
         * Fonction to be used when sorting a set of atom (<)
         * @return true if atom1 is before atom2 on the y axis
         */
        inline static bool sortY(const Atom& atom1, const Atom& atom2)
        { return atom1._xcart[1] < atom2._xcart[1]; }

        /**
         * Fonction to be used when sorting a set of atom (<)
         * @return true if atom1 is before atom2 on the z axis
         */
        inline static bool sortZ(const Atom& atom1, const Atom& atom2)
        { return atom1._xcart[2] < atom2._xcart[2]; }

        /**
         * Fonction to be used when sorting a set of atom (>)
         * @return true if type of this is larger than the one of atom 
         */
        inline bool operator> (const Atom& atom) const
        { return _typat>atom._typat; }

        /**
         * Fonction to be used when sorting a set of atom (<)
         * @return true if type of this is less than the one of atom 
         */
        inline bool operator< (const Atom& atom) const
        { return _typat < atom._typat ; }
    };

  public :

    /**
     * Define a type for relative translation.
     * */
    enum TransDir { PLUS, MINUS }; 

    /**
     * Constructor.
     * @param drawing Shall we use OpenGl to draw things ?
     */
    Canvas(bool drawing);

    /**
     * Move
     */
    Canvas(Canvas&& canvas);

    /**
     * Destructor.
     */
    virtual ~Canvas();

    /**
     * Simple function to clean all the data and start from fresh
     */
    virtual void clear() = 0;

    /**
     * Set some data from a file 
     * This will actually try to build a histdata and use the next method
     * @param filename
     */
    virtual void openFile(const std::string& filename);

    /**
     * Add some data from a file 
     * This will actually try to build a histdata and append data to the existing histdata
     * @param filename
     */
    virtual void appendFile(const std::string& filename);

    /**
     * Set some data from a histdata
     * @param hist History of some parameters.
     */
    virtual void setHist(HistData& hist) = 0;

    /**
     * Update internal variable to get new data form our histdata
     */
    virtual void updateHist() = 0;

    /**
     * Alter/modify a canvas with respect to a keyword and stream containing data.
     * @param token keyword to know what to do
     * @param stream the stream with the other data to be parsed.
     */
    virtual void alter(std::string token, std::istringstream &stream);

    /**
     * Refresh what to see on the screen
     */
    virtual void refresh(const geometry::vec3d &cam, TextRender &render) = 0;

    /**
     * Switch light on/ff
     */
    void switchLight();

    /**
     * Give acces to light
     */
    inline bool light() { return _light; }

    /**
     * Get drawing information
     */
    inline TriObj::Drawing drawing() { return _objDraw; }
    
    /**
     * Get OpenGL information
     */
    inline bool opengl() const { return _opengl; }

    /**
     * Set the number of loops to do.
     * @param nloop Number of loops.
     */
    inline void nLoop(const int nloop) {
      _nLoop = ( nloop < -2 ? -1 : nloop );
    }

    /**
     * get the number of loops to do.
     */
    inline int nLoop() { return _nLoop; }

    /**
     * Get the typical dimension of the problem
     * @param reset The dimension will be multiplied by this value, multiplying by -1 will reset the value for a new evaluation. Positive value will just scale the dimension
     * @return  the typical dimension of the problem
     */
    virtual float typicalDim(float reset = 1.f) = 0;

    /**
     * Go to next time step if available
     */
    virtual void nextStep() {
      if ( ++_itime >= _tend ) --_itime;
    }

    /**
     * Go to next time step if available
     */
    virtual void step(const int istep) {
      if ( (_itime=istep) >= _tend ) _itime=_tend-1;
      if ( _itime < _tbegin ) _itime=_tbegin;
    }

    /**
     * Go to previous time step if available
     */
    virtual void previousStep() {
      if ( --_itime == -1 || _itime < _tbegin) _itime = _tbegin;
    }

    /**
     * Change status start/pause
     */
    inline void switchPause() {
      if ( _tend-_tbegin == 0 ) return;
      _status = (_status == START ? PAUSE : START);
      if ( _status == START && _itime >= _tend-1 && _iLoop >= _nLoop ) { _iLoop = 0; _itime = _tbegin;}
    }

    /**
     * Change status start/pause
     */
    inline bool isPaused() {
      if ( _status == UPDATE ) {
        _status = PAUSE;
        return false;
      }
      return _status == PAUSE;
    }

    /**
     * Set number of time step and check _tbegin/_tend
     * @param ntime The parser containing the parameters
     */
    virtual void setNTime(int ntime);

    /**
     * Go to next frame to draw
     * @param count The number of frame to go forward
     */
    virtual void nextFrame(const int count=1);

    /**
     * Get the time step
     * @return itime
     */
    virtual int itime() const { return _itime; }

    /**
     * Get the number of time step
     * @return ntime
     */
    virtual int ntime() const { return _ntime; }

    virtual int tbegin() const { return _tbegin; }

    virtual int tend() const { return _tend-1; }

    /**
     * Get the HistData pointer
     * @return the HistData pointer to write a structure to a file
     */
    const HistData* histdata() const { return _histdata.get(); }

    /**
     * Switch the way thing should be drawn
     */
    virtual void switchDrawing() {
      _objDraw = (_objDraw == TriObj::FILL ? TriObj::SILHOUETTE : TriObj::FILL );
#ifdef HAVE_GL
      ( _objDraw == TriObj::SILHOUETTE ) ? glPolygonMode(GL_FRONT_AND_BACK,GL_LINE) :
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
#endif
    }

    /**
     * Translate the cell in the first primitive vector 
     * @param trans If trans is PLUS then it adds a new repetition and
     * if trans is MINUS then it remove a repetition.
     */
    void translateX(TransDir trans);

    /**
     * Translate the cell in the second primitive vector 
     * @param trans If trans is PLUS then it adds a new repetition and
     * if trans is MINUS then it remove a repetition.
     */
    void translateY(TransDir trans);

    /**
     * Translate the cell in the third primitive vector 
     * @param trans If trans is PLUS then it adds a new repetition and
     * if trans is MINUS then it remove a repetition.
     */
    void translateZ(TransDir trans);

    /**
     * Get some information from the canvas.
     * @return The string that should be displayed;
     */
    const std::string& info() const { return _info; }

    /**
     * Get the number of divisions used for sphere and other GL objects
     */
    int ndiv() { return _ndiv; }

    /**
     * Set the tool to plot graphs
     * By default it will use gnuplot
     * @param plot a Graph pointer 
     */
    void setGraph(Graph *plot) { _gplot.release();_gplot.reset(plot); }

    /**
     * Display help message with all command that are use in this class
     * @param out the stream to write the help message
     */
    static void help(std::ostream &out);
};

/**
 * This is the type that should be use when the user wants to change dynamically the type of canvas he uses.
 * We use a unique_ptr instead of a simple pointer to be memory safe.
 */
typedef std::unique_ptr<Canvas> pCanvas;


#endif  // CANVAS_HPP
