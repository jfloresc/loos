/*
  dcdinfo.cpp

  Dumps information about a DCD
*/



/*

  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo
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

#include <iostream>
#include <fstream>
#include <iterator>
#include <loos.hpp>
#include <boost/format.hpp>



using namespace std;
using namespace loos;

uint countFrames(DCD& dcd) {
  uint c = 0;
  for (dcd.rewind(); dcd.readFrame(); ++c) ;
  return(c);
}


void analyzeBoxes(DCD& dcd) {
  dcd.rewind();
  GCoord max(0,0,0), min(1e38, 1e38, 1e38), avg(0,0,0);
  uint n = 0;

  while (dcd.readFrame()) {
    GCoord b = dcd.periodicBox();

    avg += b;
    ++n;

    if (b.length2() > max.length2())
      max = b;
    if (b.length2() < min.length2())
      min = b;
  }

  avg /= n;

  cout << "*   Average box size is " << avg << ", min is " << min << ", and max is " << max << endl;

}



string fullHelpMessage(void) {
  string msg =
    "\n"
    "SYNOPSIS\n"
    "\tDisplay information about a DCD file\n"
    "\n"
    "DESCRIPTION\n"
    "\n"
    "\tThis tool displays the header data for a DCD file and optionally scans the periodic\n"
    "box size data.\n"
    "\n"
    "EXAMPLES\n"
    "\n"
    "\tdcdinfo -s trajectory.dcd\n"
    "This example displays the header data from trajectory.dcd along with statistics on the\n"
    "periodic box.\n"
    "\n"
    "SEE ALSO\n"
    "\ttrajinfo\n";

  return(msg);
}




int main(int argc, char *argv[]) {
  
  if (argc < 2 || argc > 3) {
    cerr << "Usage - dcdinfo [-s] trajectory.dcd\n";
    cerr << "    -s  scan the DCD for box information\n";
    cerr << fullHelpMessage();
    exit(-1);
  }

  int opt = 1;
  bool scan = false;
  if (strcmp(argv[opt], "-s") == 0) {
    scan = true;
    ++opt;
  }

  DCD::setSuppression(true);
  DCD dcd(argv[opt]);

  if (!dcd.nativeFormat())
    cout << "The DCD is not in a native binary format.\n";

  cout << boost::format("* DCD has %u atoms in %u frames with a timestep of %f.\n") % dcd.natoms() % dcd.nframes() % dcd.timestep();
  uint n = countFrames(dcd);
  if (n != dcd.nframes())
      cout << "WARNING- Trajectory actually has " << n << " rather than what is given in the header!\n"
           << "         You can fix this using the fixdcd tool on the trajectory.\n";
  
  if (dcd.hasCrystalParams()) {
    cout << "* DCD HAS box/crystal information.\n";
    dcd.readFrame();
    vector<double> xtal = dcd.crystalParams();
    cout << "* DCD Crystal params (first frame): ";
    copy(xtal.begin(), xtal.end(), ostream_iterator<double>(cout, " "));
    cout << endl;
    if (scan) {
      cout << "Scanning trajectory for box information...\n";
      analyzeBoxes(dcd);
    }
  } else
    cout << "* DCD has no box/crystal information.\n";

  vector<string> titles = dcd.titles();
  if (titles.empty())
    cout << "* DCD has no titles\n";
  else {
    cout << "* DCD Titles (" << titles.size() << "):\n";
    for (uint i=0; i<titles.size(); ++i)
      cout << "[" << i << "] : " << titles[i] << endl;
  }

  cout << "icntrl dump:\n";
  for (int i=0; i<20; ++i)
    cout << boost::format("\ticntrl[%d]\t= %d\n") % i % dcd.icntrl(i);

}
