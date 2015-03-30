/*
 *
 *    Copyright (c) 2014-2015
 *      SMASH Team
 *
 *    GNU General Public License (GPLv3 or later)
 *
 */

#ifndef SRC_INCLUDE_POTENTIALS_H_
#define SRC_INCLUDE_POTENTIALS_H_

#include <vector>

#include "configuration.h"
#include "experimentparameters.h"
#include "forwarddeclarations.h"
#include "particledata.h"
#include "threevector.h"

#ifdef BUILD_TESTS
#define VIRTUAL_FOR_TESTS virtual
#else
#define VIRTUAL_FOR_TESTS
#endif

namespace Smash {

/**
 * A class that stores parameters of potentials, calculates
 * potentials and their gradients. Potentials are responsible
 * for long-range interactions and stand in the left part of
 * Boltzmann equation. Short-range interactions are taken into
 * account in the right part of it - in the collision term.
 */
class Potentials {
 public:
  Potentials(Configuration conf, const ExperimentParameters &parameters);
  ~Potentials();
  /** Evaluates potential at point r. Potential is always taken in the local
   * Eckart rest frame, but point r is in the computational frame.
   *
   * \param[in] r Arbitrary space point where potential is calculated
   * \param[in] plist List of all particles to be used in \f$j^{\mu}\f$
   *            calculation. If the distance between particle and calculation
   *            point r, \f$ |r-r_i| > 6 \sigma \f$ then particle input
   *            to density will be ignored.
   * \param[in] acts_on pdg code of particle on which potential is going to act
   *
   * \fpPrecision Why \c double?
   **/
  VIRTUAL_FOR_TESTS
  double potential(const ThreeVector &r, const ParticleList &plist,
                                         const PdgCode acts_on) const;

  /** Evaluates potential gradient at point r. Potential is always taken in
   * the local Eckart rest frame, but point r is in the computational frame.
   *
   * \param[in] r Arbitrary space point where potential gradient is calculated
   * \param[in] plist List of all particles to be used in \f$j^{\mu}\f$
   *            calculation. If the distance between particle and calculation
   *            point r, \f$ |r-r_i| > 6 \sigma \f$ then particle input
   *            to density will be ignored.
   * \param[in] acts_on pdg code of particle on which potential is going to act
   **/
  VIRTUAL_FOR_TESTS
  ThreeVector potential_gradient(const ThreeVector &r,
                                 const ParticleList &plist,
                                 const PdgCode acts_on) const;

 private:
  // Skyrme potential on/off
  bool use_skyrme_;

  // Symmetry potential on/off
  bool use_symmetry_;

  // Number of test particles
  int ntest_;

  /** sigma of gaussian smearing
   * \fpPrecision Why \c double?
   */
  double sigma_;

  /** Parameters of skyrme potentials
   * \fpPrecision Why \c double?
   */
  double skyrme_a_, skyrme_b_, skyrme_tau_;

  /** Parameters of symmetry potential
   * \fpPrecision Why \c double?
   */
  double symmetry_s_;
};

}  // namespace Smash

#endif  // SRC_INCLUDE_POTENTIALS_H_
