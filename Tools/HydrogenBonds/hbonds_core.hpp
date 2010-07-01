/*
  Core code for hbonds utilties

  (c) 2010 Tod D. Romo, Grossfield Lab, URMC
*/




#if !defined(HBONDS_CORE_HPP)
#define HBONDS_CORE_HPP

#include <loos.hpp>



typedef loos::Math::Matrix<int, loos::Math::RowMajor>   BondMatrix;


// Our own exception so we can provide a little more helpful
// information when we throw-up...

struct ErrorWithAtom : public std::exception {
  std::string _msg;
  
  ErrorWithAtom(const loos::pAtom& a, const std::string& msg) {
    std::stringstream ss;
    ss << msg << std::endl << *a << std::endl;
    _msg = ss.str();
  }

  const char* what() const throw() { return(_msg.c_str()); }

  ~ErrorWithAtom() throw() { }
};



// Track the atoms that may participate in a hydrogen bond, and the
// atoms they're attached to (if necessary) to compute the bond angle.
// Also encapsulates the operations for determining if an h-bond
// exists...
//
// Note that we hook into the parent group's SharedPeriodicBox so we
// always have current periodic boundary info...


class SimpleAtom {
public:
  SimpleAtom(const loos::pAtom& a) : atom(a), isHydrogen(false), usePeriodicity(false) { }
  SimpleAtom(const loos::pAtom& a, const loos::SharedPeriodicBox& b) : atom(a), isHydrogen(false), usePeriodicity(true), sbox(b) { }
  SimpleAtom(const loos::pAtom& a, const loos::SharedPeriodicBox& b, const bool c) : atom(a), isHydrogen(false), usePeriodicity(c), sbox(b) { }

  double distance2(const SimpleAtom& s) const;
  double angle(const SimpleAtom& s) const;

  static bool debuggingMode() { return(debugging); }
  static void debuggingMode(const bool b) { debugging = b; }

  static double innerRadius()  { return(sqrt(inner)); }
  static void innerRadius(const double r)  { inner = r*r; }

  static double outerRadius()  { return(sqrt(outer)); }
  static void outerRadius(const double r)  { outer = r*r; }

  static double maxDeviation()  { return(deviation); }
  static void maxDeviation(const double d)  { deviation = d; }


  bool hydrogenBond(const SimpleAtom& other) const;
  loos::AtomicGroup findHydrogenBonds(const std::vector<SimpleAtom>& group, const bool findFirstOnly = true);
  
  BondMatrix findHydrogenBondsMatrix(const std::vector<SimpleAtom>& group, loos::pTraj& traj, loos::AtomicGroup& model, const uint maxt) const;
  BondMatrix findHydrogenBondsMatrix(const std::vector<SimpleAtom>& group, loos::pTraj& traj, loos::AtomicGroup& model) const {
    return(findHydrogenBondsMatrix(group, traj, model, traj->nframes()));
  }


  static std::vector<SimpleAtom> processSelection(const std::string& selection, const loos::AtomicGroup& system, const bool use_periodicity = false);


  


  friend std::ostream& operator<<(std::ostream& os, const SimpleAtom& s) {
    os << "<SimpleAtom>\n";
    os << *(s.atom) << std::endl;
    os << "<isHydrogen " << s.isHydrogen << "/>\n";
    if (s.attached_to != 0)
      os << *(s.attached_to) << std::endl;
    os << "</SimpleAtom>";
    return(os);
  }


private:

  loos::pAtom atom;
  bool isHydrogen;
  bool usePeriodicity;

  static double inner, outer, deviation;
  static bool debugging;

  loos::SharedPeriodicBox sbox;
  loos::pAtom attached_to;

};




typedef SimpleAtom    SAtom;
typedef std::vector<SAtom> SAGroup;



extern BondMatrix findHydrogenBonds(const SAtom& donor, const SAGroup& acceptors, loos::pTraj& traj, loos::AtomicGroup& model, const uint maxt);




#endif
