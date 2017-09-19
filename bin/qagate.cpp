/**
 * @file ./bin/qagate.cpp
 *
 * @brief  Generate an animation for MD simulation
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2016 Jordan Bieder
 *
 * This file is part of AbiOut
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include <csignal>
#include <fstream>
#include <sstream>
#ifdef HAVE_QT
#include <QApplication>
#include "qtgui/glwidget.hpp"
#endif
#include "io/parser.hpp"
#include "io/configparser.hpp"
#include "hist/histdatanc.hpp"
#include "qtgui/glwidget.hpp"
#include "base/utils.hpp"
#include "qtgui/winqt.hpp"
#include <clocale>
#ifdef HAVE_FFTW3_THREADS
#include "fftw3.h"
#endif

#if defined(HAVE_SPGLIB) && defined(HAVE_SPGLIB_VERSION)
#  ifdef __cplusplus
extern "C"{
#  endif
#  include "spglib/spglib.h"
#  ifdef __cplusplus
}
#  endif
#endif


WinQt* ptrwin = nullptr; ///< Pointer to the window if created

/**
 * Simple function to display the name and version of the package and what window manager we use.
 */
void Version(){
  std::cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << std::endl;
  utils::dumpConfig(std::clog);
#ifdef HAVE_QT
  std::clog << "Using Qt version " << qVersion() << std::endl;
#endif
#if defined(HAVE_SPGLIB) && defined(HAVE_SPGLIB_VERSION)
  std::clog << "Using spglib version " << spg_get_major_version() << "." 
    << spg_get_minor_version() << "." 
    << spg_get_micro_version() << std::endl;
#endif
  
}

void initInput(GLWidget* glwidget, int argc, const char** argv) {
  std::vector<std::string> filename;
  for ( int i = 0 ; i < argc ; ++i ) {
    std::ifstream file(argv[i],std::ios::in);

    if ( file.good() && (i == 0 || (i > 0 && strcmp(argv[i-1],"-c") != 0 && strcmp(argv[i-1],"--config")) != 0) ) {
      filename.push_back(std::string(argv[i]));
    }

    file.close();
  }
  try {
    if ( filename.size() > 0 && glwidget != nullptr ) {
      try {
        //canvas->openFile(filename[0]);
        glwidget->processCommand(":open "+filename[0]);
      }
      catch ( Exception &e ) {
        if ( e.getReturnValue() == ERRDIV || e.getReturnValue() == ERRABT  )
          throw e;
      }

      for ( unsigned file = 1; file < filename.size() ; ++file ) {
        try {
          glwidget->processCommand(":append "+filename[file],false); // Do not pop inputChar
        }
        catch ( Exception &e ) {
          e.ADD("Ignoring file "+filename[file], ERRWAR);
          std::clog << e.fullWhat() << std::endl;
        }
      }

    }

  }
  catch (Exception& e) {
    e.ADD("Updating canvas failed",ERRDIV);
    std::clog << e.fullWhat() << std::endl;
  }
}


/** 
 * Handle signals
 * @param para signal received
 */
void handle_signal (int para) {
  if ( ptrwin == nullptr ) {
    std::cerr << "No window created.\nExiting." << std::endl;
    exit(1);
  }
  switch(para) {
    case SIGABRT :
      std::cerr << "Abord signal received." << std::endl;
      break;
    case SIGFPE :
      std::cerr << "Floating point exception." << std::endl;
      break;
    case SIGILL :
      std::cerr << "Illegal instruction exception." << std::endl;
      break;
    case SIGSEGV :
      std::cerr << "Segmentation fault occured." << std::endl;
      break;
    case SIGTERM :
    case SIGINT :
#ifndef _WIN32
    case SIGQUIT :
    case SIGKILL :
#endif
      std::cerr << "Killing process." << std::endl;
      break;
    default : 
      std::cerr << "Unknown signal received." << std::endl;
      break;
  }
  //ptrwin->exit();
  std::cerr << "Window has been asked to close." << std::endl;
  exit(-1);
};



/**
 * Main Program
 * @param argc Number of argument on the command line.
 * @param argv Array of strings for each argument.
 * @return 0 if succeeded, =/0 otherwise.
 */
int main(int argc, char** argv) {
  int rvalue = 0;
  QApplication app(argc,argv);
  QGLFormat glf = QGLFormat::defaultFormat(); 
  glf.setSampleBuffers(true); 
  glf.setSamples(8); 
  QGLFormat::setDefaultFormat(glf); 
  //QResource::registerResource("icons.rcc");
  Q_INIT_RESOURCE(icons);

  Parser parser(argc,argv);
  parser.setOption("config",'c',"","Configuration file to configure the animation.");
  parser.setOption("font",'f',"","Font to use for displaying information on the screen.");
  parser.setOption("version",'v',"Print the version number");
  parser.setOption("help",'h',"Print this message");
  parser.setOption("verbosity",'V',"2","0 : nothing\n1 : write to file\n2 : write to screen");

  std::streambuf* bufstderr = std::cerr.rdbuf();
  std::streambuf* bufstdlog = std::clog.rdbuf();
  std::ofstream fstdlog;
  std::ofstream fstderr;

#ifdef HAVE_FFTW3_THREADS
  fftw_init_threads();
#endif

  try {
    parser.parse();
    switch ( parser.getOption<unsigned>("verbosity") ) {
      case 0 : {
#ifndef _WIN32
                 fstdlog.open("/dev/null",std::ios::out);
                 fstderr.open("/dev/null",std::ios::out);
#else
                 fstdlog.open("NUL",std::ios::out);
                 fstderr.open("NUL",std::ios::out);
#endif
                 std::clog.rdbuf(fstdlog.rdbuf());
                 std::cerr.rdbuf(fstderr.rdbuf());
                 break;
               }
      case 1 : {
                 std::string errorfile=PACKAGE;
                 errorfile += ".error";
                 fstderr.open(errorfile.c_str(),std::ios::out);
                 std::cerr.rdbuf(fstderr.rdbuf());
                 std::string logfile=PACKAGE;
                 logfile += ".log";
                 fstdlog.open(logfile.c_str(),std::ios::out);
                 std::clog.rdbuf(fstdlog.rdbuf());
                 break;
               }
      case 2 : {
                 break;
               }
      default : {
                  Exception e = EXCEPTION("Bad value for line option --verbosity (-V).\nShould be (0|1|2).\nSee -h for help",ERRWAR);
                  std::clog << e.what() << std::endl;
                  break;
                }
    }
    Version();

    if ( parser.getOption<bool>("version") ) {
      throw EXCEPTION("",10);
    }
    if ( parser.getOption<bool>("help") ) {
      throw EXCEPTION("",0);
    }

    std::string config;
    if ( parser.isSetOption("config") ) {
      config = parser.getOption<std::string>("config");
    }

    ptrwin = new WinQt;

    signal(SIGABRT,handle_signal);
    signal(SIGFPE,handle_signal);
    signal(SIGILL,handle_signal);
    signal(SIGINT,handle_signal);
    signal(SIGSEGV,handle_signal);
    signal(SIGTERM,handle_signal);
#ifndef _WIN32
    signal(SIGKILL,handle_signal);
    signal(SIGQUIT,handle_signal);
#endif



    setlocale(LC_ALL, "C");
    {
      initInput(ptrwin->current(),argc-1, (const char**) argv+1);
      try{
        if ( parser.isSetOption("font") )
          ptrwin->setFont(parser.getOption<std::string>("font"));
      }
      catch (Exception& e) {
        if ( e.getReturnValue() != ConfigParser::ERFOUND ) {
          e.ADD("Something bad happened",ERRDIV);
          throw e;
        }
      }
    }

    ptrwin->setParameters(config);
  

  }
  catch ( Exception& e ) {
    rvalue = e.getReturnValue();
    if ( rvalue == ERRDIV ) {
      std::cerr << e.fullWhat() << std::endl;
      if ( rvalue == Parser::ERARG || rvalue == Parser::EROPT ) {
        std::cerr << parser;
        rvalue = 0;
      }
#if defined(WIN32) || defined(_WIN32)
      system("Pause");
#endif
    }
    else if ( rvalue == ERRWAR || rvalue == ERRCOM ) {
      std::clog << e.fullWhat() << std::endl;
#if defined(WIN32) || defined(_WIN32)
      system("Pause");
#endif
    }
    else if ( rvalue == 10 ) { // ask for version number
      rvalue = 0;
    }
    else {
      std::cout << parser;
      Window::help();
    }
  }

  if ( ptrwin != nullptr ) {
    ptrwin->show();
    rvalue = app.exec();
  }

#ifdef HAVE_FFTW3_THREADS
  fftw_cleanup_threads();
#endif

  std::cerr.rdbuf(bufstderr);
  if ( fstderr ) fstderr.close();

  std::clog.rdbuf(bufstdlog);
  if ( fstdlog ) fstdlog.close();

  return rvalue;
}
