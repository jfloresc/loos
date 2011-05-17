/*
  density-dist.cpp

  Compute the charge/mass/electron density along the z dimension of a system

*/



/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Alan Grossfield
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


#include <ctype.h>

#include <loos.hpp>
#include <boost/program_options.hpp>


using namespace std;
using namespace loos;
namespace opts = loos::OptionsFramework;

enum CalculationType { ELECTRON, CHARGE, MASS };


// @cond TOOLS_INTERNAL
class ToolOptions : public opts::OptionsPackage {
public:
  ToolOptions() :
    symmetrize(false),
    auto_all(true),
    calc_type_desc("electron")
  { }

  void addGeneric(opts::po::options_description& o) {
    o.add_options()
      ("auto", opts::po::value<bool>(&auto_all)->default_value(auto_all), "Automatically compute system density")
      ("zsymmetry", opts::po::value<bool>(&symmetrize)->default_value(symmetrize), "Symmetric with respect to Z")
      ("type", opts::po::value<string>(&calc_type_desc)->default_value(calc_type_desc), "Calculation type (mass, charge, electron)")
      ("window", opts::po::value<uint>(&window)->default_value(window), "Window size (in frames) for time series (0 = disabled)")
      ;
  }

  bool postConditions(opts::po::variables_map& map) {
    if (toupper(calc_type_desc[0]) == 'C')
      calc_type = CHARGE;
    else if (toupper(calc_type_desc[0]) == 'E')
      calc_type = ELECTRON;
    else if (toupper(calc_type_desc[0]) == 'M')
      calc_type = MASS;
    else {
      cerr << "Error- unknown calculation type '" << calc_type_desc << "' (should be either charge, mass or electron)\n";
      return(false);
    }
    return(true);
  }

  string print() const {
    ostringstream oss;
    oss << boost::format("auto=%d, zsymmetry=%d, type='%s', window=%d") 
      % auto_all
      % symmetrize
      % calc_type_desc
      % window;
    return(oss.str());
  }

  bool symmetrize, auto_all;
  uint window;
  string calc_type_desc;
  CalculationType calc_type;
};

// @endcond





int main(int argc, char *argv[]) {

  string hdr = invocationHeader(argc, argv);

  // Options handling...
  opts::BasicOptions* bopts = new opts::BasicOptions;
  opts::OutputPrefix* popts = new opts::OutputPrefix;
  opts::BasicTrajectory* tropts = new opts::BasicTrajectory;

  // The required options could have been folded into ToolOptions, but
  // using RequiredArguments saves us from having to handle the help()
  // and print() methods...
  opts::RequiredArguments* ropts = new opts::RequiredArguments;
  ropts->addArgument("minz", "min-z");
  ropts->addArgument("maxz", "max-z");
  ropts->addArgument("nbins", "number-of-bins");
  ropts->addVariableArguments("selection", "selection");
  ToolOptions* topts = new ToolOptions;

  opts::AggregateOptions options;
  options.add(bopts).add(popts).add(tropts).add(ropts).add(topts);
  if (!options.parse(argc, argv))
    exit(-1);

  double min_z = parseStringAs<double>(ropts->value("minz"));
  double max_z = parseStringAs<double>(ropts->value("maxz"));
  uint nbins = parseStringAs<uint>(ropts->value("nbins"));
  vector<string> selections = ropts->variableValues("selection");

  AtomicGroup system = tropts->model;
  pTraj traj = tropts->trajectory;

  // End of options

  cout << "# " << hdr << endl;

  // density from each selection
  vector<AtomicGroup> subsets;
  if (topts->auto_all)
    subsets.push_back(system);
  for (vector<string>::iterator i = selections.begin(); i != selections.end(); ++i)
    subsets.push_back(selectAtoms(system, *i));
  
  double bin_width = (max_z - min_z) / nbins;

  // Create and zero out the total charge_distribution
  vector< vector<double> > dists(subsets.size(), vector<double>(nbins, 0.0));
  // If we do windowed time series too, we'll need to zero out as we go, which 
  // means we'll need separate storage for the sum
  vector< vector<double> > cum_dists(subsets.size(), vector<double>(nbins, 0.0));

  // Note: the equillibration frames are already skipped by opts::BasicTrajectory
  
  // loop over the remaining frames
  uint frame = 0;
  while (traj->readFrame()) {
    // update coordinates
    traj->updateGroupCoords(system);

    // Compute the bin volume for normalization purposes
    GCoord box = system.periodicBox();
    double bin_volume = bin_width * box.x() * box.y();

    // Loop over the subsets and compute charge distribution.
    // (first set is all atoms)
    for (uint i=0; i<subsets.size(); i++) {
      double weight;
      for (AtomicGroup::iterator atom = subsets[i].begin(); 
                                 atom != subsets[i].end(); 
                                 ++atom) {
        switch(topts->calc_type) {
        case CHARGE: weight = (*atom)->charge(); break;
        case MASS: weight = (*atom)->mass(); break;
        case ELECTRON: weight = (*atom)->atomic_number() - (*atom)->charge(); break;
        default:
          cerr << "ERROR- unknown calculation type\n";
          exit(-1);
        }
        double z = (*atom)->coords().z();
        if (topts->symmetrize)
          z = abs(z);
        
        if ( (z > min_z) && (z < max_z) ) {
          uint bin = (int)( (z - min_z) / bin_width );
          dists[i][bin] += weight/bin_volume;
        }
      }
    }
    ++frame;

    // If windowed time series were requested, output them here
    if (topts->window && (frame % topts->window == 0)) {
        uint output_val = frame / topts->window;

        // build the output file name
        char piece[128];
        snprintf(piece, 128, "_%d.dat", output_val);
        string file_name = popts->prefix + string(piece);
        ofstream outfile(file_name.c_str());
        if (!outfile.is_open() ) {
            throw(runtime_error("couldn't open output file"));
            }

        // write the header
        outfile << "# Z\tAllAtoms";
        for (uint i=1; i<subsets.size(); i++) {
          outfile << " Set(" << i <<") "; 
        }
        outfile << endl;

        // Normalize by the number of frames and
        // output the average density
        for (uint i=0; i<nbins; i++) {
            double z = (i+0.5)*bin_width + min_z;
            outfile << z << "\t";
            for (unsigned int j=0; j<subsets.size(); j++) {
                double c = dists[j][i] / topts->window;
                outfile << c << "\t";
            }
            outfile << endl;
        }

        // Add the windowed density to the cumulative density and
        // zero out the distributions
        
        for (uint i=0; i<dists.size(); i++) {
            for (uint j=0; j<nbins; j++) {
                cum_dists[i][j] += dists[i][j];
            }

            // zero out the windowed dist
            dists[i].assign(dists[i].size(), 0.0);
        }
        
    }


  }

  // Normalize by the number of frames and output the average charge density
  cout << "# Z\t" << (topts->auto_all ? "AllAtoms" : "");
  for (uint i=1; i<subsets.size(); i++) {
    cout << " Set(" << i <<") "; 
  }
  cout << endl;

  // If we didn't output windowed time series, we never used cum_dists, so we need 
  // to copy it over here
  // If we did, we may have some data in dists that hasn't been added to cum_dists yet
  if (!topts->window) {
      cum_dists = dists;
  }
  else if (frame % topts->window != 0){
     for (uint i=0; i<dists.size(); i++) {
         for (uint j=0; j<nbins; j++) {
             cum_dists[i][j] += dists[i][j];
         }
     }
  }

  for (uint i=0; i<nbins; i++) {
    double z = (i+0.5)*bin_width + min_z;
    cout << z << "\t"; 
    for (uint j=0; j<subsets.size(); j++) {
      double c = cum_dists[j][i] / frame;
      
      cout << c << "\t";
    }
    cout << endl;
  }


  
}
