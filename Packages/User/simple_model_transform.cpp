/*
 simple_model_transform.cpp

  (c) 2011 Tod D. Romo, Grossfield Lab
           Department of Biochemistry
           University of Rochster School of Medicine and Dentistry


  C++ template for writing a tool that transforms a model, writing it
  out as a PDB to stdout
*/


/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2011 Tod D. Romo
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



#include <loos.hpp>

using namespace std;
using namespace loos;



int main(int argc, char *argv[]) {
  
  // How the tool was invoked, for logging purposes...
  string header = invocationHeader(argc, argv);

  // **EDIT***
  // Verify correct number of command-line arguments
  if (argc != 3) {
    cerr << "Usage- simple_model_transform model-name selection\n";
  }

  // Handle command-line arguments
  int arg_index = 1;

  // Read in the model
  AtomicGroup model = createSystem(argv[arg_index++]);

  // Select a subset of it
  AtomicGroup subset = selectAtoms(model, argv[arg_index++]);

  // Prune the bonds-list (if present)
  subset.pruneBonds();

  // ***EDIT***
  // Iterate over all atoms in the subset, performing some computation
  for (AtomicGroup::iterator atom = subset.begin(); atom != subset.end(); ++atom) {
    // Do something
    // performTransformation(*atom);
  }

  // Convert the subset into a PDB
  PDB pdb = PDB::fromAtomicGroup(subset);

  // Insert the logging information into the remarks at the start of the PDB
  pdb.remarks().add(header);

  // Write the PDB to stdout
  cout << pdb;
}
