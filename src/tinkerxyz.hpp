/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo, Alan Grossfield
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#if !(defined LOOS_TINKERXYZ_HPP)
#define LOOS_TINKERXYZ_HPP

#include <ios>
#include <fstream>
#include <sstream>
#include <iostream>

#include <stdexcept>

#include <loos_defs.hpp>
#include <AtomicGroup.hpp>


namespace loos {

  //! Class for reading a subset of the TinkerXYZ format
  /**
   * Notes:
   *
   The tinker ARC trajectory file format is just concatenated XYZ, so this
   code should be shared.
  */
  class TinkerXYZ : public AtomicGroup {
  public:
    TinkerXYZ() : _max_index(0) { }
    virtual ~TinkerXYZ() {}

    explicit TinkerXYZ(const std::string& fname) : _max_index(0), _filename(fname) {
      std::ifstream ifs(fname.c_str());
      if (!ifs)
	throw(FileOpenError(fname));
      read(ifs);
    }

    explicit TinkerXYZ(std::istream &ifs) : _max_index(0), _filename("stream") {
      read(ifs);
    }


    static pAtomicGroup create(const std::string& fname) {
      return(pAtomicGroup(new TinkerXYZ(fname)));
    } 

    //! Clones an object for polymorphism (see AtomicGroup::clone() for more info)
    virtual TinkerXYZ* clone(void) const;

    //! Creates a deep copy (see AtomicGroup::copy() for more info)
    TinkerXYZ copy(void) const;

    void read(std::istream& is);  


  private:

    TinkerXYZ(const AtomicGroup& grp) : AtomicGroup(grp) { }


    void parseAtomRecord(const std::string&);  

    //! parse the optional tinker box record
    /**
     *  Explicitly ignores the angles, assumes rectangular box
     */
    bool parseBoxRecord(const std::string& s);

    uint _max_index;
    std::string _filename;
  };


}

#endif
