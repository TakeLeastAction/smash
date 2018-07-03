/*
 *    Copyright (c) 2012-2018
 *      SMASH Team
 *
 *    GNU General Public License (GPLv3 or later)
 */
#ifndef SRC_INCLUDE_PARTICLES_H_
#define SRC_INCLUDE_PARTICLES_H_

#include <memory>
#include <type_traits>
#include <vector>

#include "macros.h"
#include "particledata.h"
#include "particletype.h"
#include "pdgcode.h"

namespace smash {

/**
 * \ingroup data
 *
 * The Particles class abstracts the storage and manipulation of particles.
 *
 * There is one Particles object per Experiment. It stores
 * the data about all existing particles in the experiment (ParticleData).
 *
 * \note
 * The Particles object cannot be copied, because it does not make sense
 * semantically. Move semantics make sense and can be implemented when needed.
 */
class Particles {
 public:
  /// Creates a new (empty) Particles object.
  Particles();

  /// Cannot be copied
  Particles(const Particles &) = delete;
  /// Cannot be copied
  Particles &operator=(const Particles &) = delete;

  /// \return a copy of all particles as a std::vector<ParticleData>.
  ParticleList copy_to_vector() const {
    if (dirty_.empty()) {
      return {&data_[0], &data_[data_size_]};
    }
    return {begin(), end()};
  }

  /**
   * Inserts the particle \p into the list of particles.
   * The argument \p will afterwards not be a valid copy of a particle of the
   * internal list. I.e.
   * \code
   * ParticleData particle(type);
   * particles.insert(particle);
   * particles.is_valid(particle);  // returns false
   * \endcode
   *
   * \param[in] p The data to be added. The id will not be copied. Instead a new
   * unique id is generated by the function.
   *
   * \return An immutable reference to the new ParticleData object in the
   * Particles list. This is a valid copy (i.e. Particles::is_valid returns
   * true).
   */
  const ParticleData &insert(const ParticleData &p);

  /**
   * Add \p n particles of the same type (\p pdg) to the list.
   *
   * \param[in] n The number of the added particles
   * \param[in] pdg PDG code of the added particles
   */
  void create(size_t n, PdgCode pdg);

  /**
   * Add one particle of the given \p pdg code
   *
   * \param[in] pdg PDG code of the added particle
   * \return a reference to the added particle
   */
  ParticleData &create(const PdgCode pdg);

  /// \return the number of particles in the list.
  size_t size() const { return data_size_ - dirty_.size(); }

  /// \return whether the list of particles is empty.
  bool is_empty() const { return data_size_ == 0; }

  /**
   *  Returns the time of the computational frame.
   *
   * \return computation time which is reduced by the start up time
   *
   * \note This function may only be called if the list of particles is not
   * empty.
   */
  double time() const {
    assert(!is_empty());
    return front().position().x0();
  }

  /**
   * Reset the state of the Particles object to an empty list and a new id
   * counter. The object is thus in the same state as right after construction.
   */
  void reset();

  /**
   * Check whether the ParticleData copy is still a valid copy of the one
   * stored in the Particles object.
   *
   * \param[in] copy ParticleData copy whose validity is going to be checked
   * \return whether ParticleData copy is still valid. If not, then the
   *         particle either never was a valid copy or it has interacted
   *         (e.g. scatter, decay) since it was copied.
   */
  bool is_valid(const ParticleData &copy) const {
    if (data_size_ <= copy.index_) {
      return false;
    }
    /* Check if the particles still exists. If it decayed
     * or scattered inelastically it is gone. */
    return data_[copy.index_].id() ==
               copy.id()
           /* If the particle has scattered
            * elastically, its id_process has
            * changed and we consider it invalid. */
           && data_[copy.index_].id_process() ==
                  copy.id_process();
  }

  /**
   * Remove the given particle \p p from the list. The argument \p p must be a
   * valid copy obtained from Particles, i.e. a call to \ref is_valid must
   * return \c true.
   *
   * \param[in] p Particle which is going to be removed
   * \note The validity of \p p is only enforced in DEBUG builds.
   */
  void remove(const ParticleData &p);

  /**
   * Replace the particles in \p to_remove with the particles in \p to_add in
   * the list of current particles. The particles in \p to_remove must be valid
   * copies obtained from Particles. The particles in \p to_add are adjusted
   * to be valid copies of the new particles in the Particles list.
   *
   * \param[in] to_remove A list of particles which are valid copies out of the
   *            Particles list. They identify the entries in Particles to be
   *            replaced.
   * \param[in] to_add A list of (invalid) ParticleData objects to be placed
   *            into the Particles list.
   *
   * \note The validity of \p to_remove is only enforced in DEBUG builds.
   */
  void replace(const ParticleList &to_remove, ParticleList &to_add);

  /**
   * Updates the particle identified by \p p with the state stored in \p
   * new_state. A reference to the resulting ParticleData object in the list is
   * subsequently returned.
   *
   * The state update copies the id_process, momentum, and position from \p
   * new_state.
   *
   * This function expects \p p to be a valid copy (i.e. is_valid returns \c
   * true) and it expects the ParticleType of \p p and \p new_state to be
   * equal. This is enforced in DEBUG builds.
   *
   * \param[in] p The particle which is going to be updated.
   * \param[in] new_state The new state of the particle.
   * \return Updated particle
   */
  const ParticleData &update_particle(const ParticleData &p,
                                      const ParticleData &new_state) {
    assert(is_valid(p));
    assert(p.type() == new_state.type());
    ParticleData &original = data_[p.index_];
    new_state.copy_to(original);
    return original;
  }

  /**
   * Updates the Particles object, replacing the particles in \p old_state with
   * the particles in \p new_state.
   *
   * The third parameter \p do_replace determines whether the particles are
   * actually replaced (so that they get new IDs etc) or if the old particles
   * are kept and just updated with new properties (e.g. in an elastic
   * collision).
   *
   * This function expects \p old_state to be a valid copy (i.e. is_valid
   * returns \c true). This is enforced in DEBUG builds.
   *
   * \param[in] old_state Paticles of old states
   * \param[in] new_state New states of these particles
   * \param[in] do_replace Whether to replace the old states by the new ones
   */
  void update(const ParticleList &old_state, ParticleList &new_state,
              bool do_replace) {
    if (do_replace) {
      replace(old_state, new_state);
    } else {
      for (std::size_t i = 0; i < old_state.size(); ++i) {
        new_state[i] = update_particle(old_state[i], new_state[i]);
      }
    }
  }

  /**
   * Returns the particle that is currently stored in this object given an old
   * copy of that particle.
   *
   * This function expects \p old_state to be a valid copy (i.e. is_valid
   * returns \c true). This is enforced in DEBUG builds.
   *
   * \param[in] old_state A copy of old state particle. We need its index to
   *            know where it's stored in the particles list.
   * \return The currrent state of the particle which is searched.
   */
  const ParticleData &lookup(const ParticleData &old_state) const {
    assert(is_valid(old_state));
    return data_[old_state.index_];
  }

  /**
   * \internal
   * Iterator type that skips over the holes in data_. It implements a standard
   * bidirectional iterator over the ParticleData objects in Particles.
   */
  template <typename T>
  class GenericIterator
      : public std::iterator<std::bidirectional_iterator_tag, T> {
    friend class Particles;

   public:
    /// remove const qualification
    using value_type = typename std::remove_const<T>::type;
    /// provide the pointer of a reference
    using pointer = typename std::add_pointer<T>::type;
    /// creates a lvalue reference
    using reference = typename std::add_lvalue_reference<T>::type;
    /// add const qualification to a pointer
    using const_pointer = typename std::add_const<pointer>::type;
    /// add const qualification to a reference
    using const_reference = typename std::add_const<reference>::type;

   private:
    /**
     * Constructs an iterator pointing to the ParticleData pointed to by \p p.
     * This constructor may only be called from the Particles class.
     *
     * \param[in] p The particle which is pointed by the iterator.
     */
    GenericIterator(pointer p) : ptr_(p) {}  // NOLINT(runtime/explicit)

    /// The entry in Particles this iterator points to.
    pointer ptr_;

   public:
    /**
     * Advance the iterator to the next valid (not a hole) entry in Particles.
     * Holes are identified by the ParticleData::hole_ member and thus the
     * internal pointer is incremented until all holes are skipped. It is
     * important that the Particles entry pointed to by Particles::end() is not
     * identified as a hole as otherwise the iterator would advance too far.
     *
     * \return the iterator to the next valid particle.
     */
    GenericIterator &operator++() {
      do {
        ++ptr_;
      } while (ptr_->hole_);
      return *this;
    }
    /**
     * Postfix variant of the above prefix increment operator.
     *
     * \return the iterator before increment.
     */
    GenericIterator operator++(int) {
      GenericIterator old = *this;
      operator++();
      return old;
    }

    /**
     * Advance the iterator to the previous valid (not a hole) entry in
     * Particles.
     * Holes are identified by the ParticleData::hole_ member and thus the
     * internal pointer is decremented until all holes are skipped. It is
     * irrelevant whether Particles::data_[0] is a hole because the iteration
     * typically ends at Particles::begin(), which points to a non-hole entry in
     * Particles::data_.
     *
     * \return the iterator to the previous valid particle.
     */
    GenericIterator &operator--() {
      do {
        --ptr_;
      } while (ptr_->hole_);
      return *this;
    }
    /**
     * Postfix variant of the above prefix decrement operator.
     *
     * \return the iterator before decrement.
     */
    GenericIterator operator--(int) {
      GenericIterator old = *this;
      operator--();
      return old;
    }

    /// \return the dereferenced iterator.
    reference operator*() { return *ptr_; }
    /// \return the dereferenced iterator.
    const_reference operator*() const { return *ptr_; }

    /// \return the dereferenced iterator.
    pointer operator->() { return ptr_; }
    /// \return the dereferenced iterator.
    const_pointer operator->() const { return ptr_; }

    /// \return whether two iterators point to the same object.
    bool operator==(const GenericIterator &rhs) const {
      return ptr_ == rhs.ptr_;
    }
    /// \return whether two iterators point to different objects.
    bool operator!=(const GenericIterator &rhs) const {
      return ptr_ != rhs.ptr_;
    }
    /// \return whether this iterator comes before the iterator \p rhs.
    bool operator<(const GenericIterator &rhs) const { return ptr_ < rhs.ptr_; }
    /// \return whether this iterator comes after the iterator \p rhs.
    bool operator>(const GenericIterator &rhs) const { return ptr_ > rhs.ptr_; }
    /**
     * \return whether this iterator comes before the iterator \p rhs or points
     * to the same object.
     */
    bool operator<=(const GenericIterator &rhs) const {
      return ptr_ <= rhs.ptr_;
    }
    /**
     * \return whether this iterator comes after the iterator \p rhs or points
     * to the same object.
     */
    bool operator>=(const GenericIterator &rhs) const {
      return ptr_ >= rhs.ptr_;
    }
  };
  /// An alias to the GenericIterator class of ParticleData
  using iterator = GenericIterator<ParticleData>;
  /// An alias to the GenericIterator class of const ParticleData
  using const_iterator = GenericIterator<const ParticleData>;

  /// \return a reference to the first particle in the list.
  ParticleData &front() { return *begin(); }
  /**
   * const overload of &front()
   *
   * \return a reference to the first particle in the list.
   */
  const ParticleData &front() const { return *begin(); }

  /// \return a reference to the last particle in the list.
  ParticleData &back() { return *(--end()); }
  /**
   * const overload of &back()
   *
   * \return a reference to the last particle in the list.
   */
  const ParticleData &back() const { return *(--end()); }

  /**
   * \return an iterator pointing to the first particle in the list. Use it to
   * iterate over all particles in the list.
   */
  iterator begin() {
    ParticleData *first = &data_[0];
    while (first->hole_) {
      ++first;
    }
    return first;
  }
  /**
   * const overload of begin()
   *
   * \return an iterator pointing to the first particle in the list.
   */
  const_iterator begin() const {
    ParticleData *first = &data_[0];
    while (first->hole_) {
      ++first;
    }
    return first;
  }

  /**
   * \return an iterator pointing behind the last particle in the list. Use it
   * to iterate over all particles in the list.
   */
  iterator end() { return &data_[data_size_]; }
  /**
   * const overload of end()
   *
   * \return an iterator pointing behind the last particle in the list.
   */
  const_iterator end() const { return &data_[data_size_]; }

  /// \return a const begin iterator.
  const_iterator cbegin() const { return begin(); }
  /// \return a const end iterator.
  const_iterator cend() const { return end(); }

  /**
   * \ingroup logging
   * Print effective mass and type name for all particles to the stream.
   */
  friend std::ostream &operator<<(std::ostream &out, const Particles &p);

 private:
  /**
   * Highest id of a given particle. The first particle added to data_ will
   * have id 0.
   */
  int id_max_ = -1;

  /**
   * \internal
   * Increases the capacity of data_ to \p new_capacity.
   *
   * \param[in] new_capacity new capacity which is expected to be larger than
   * data_capacity_. This is enforced in DEBUG builds.
   */
  void increase_capacity(unsigned new_capacity);
  /**
   * \internal
   * Ensure that the capacity of data_ is large enough to hold \p to_add more
   * entries. If the capacity does not sufficent increase_capacity is called.
   *
   * \param[in] to_add Number of particles which is going to be added to the list.
   */
  inline void ensure_capacity(unsigned to_add);
  /**
   * \internal
   * Common implementation for copying the relevant data of a ParticleData
   * object into data_. This does not implement a 1:1 copy, instead:
   * \li The particle id in data_ is set to the next id for a new particle.
   * \li The id_process, type, momentum, and position are copied from \p from.
   * \li The ParticleData::index_ members is not modified since it already has
   *     the correct value
   * \li The ParticleData::hole_ member is not modified and might need
   *     adjustment in the calling code.
   *
   * \param[out] to The copied version the particle added to the end of the
   *             to the particle list.
   * \param[in]  from the original particle that will be copied.
   */
  inline void copy_in(ParticleData &to, const ParticleData &from);

  /**
   * \internal
   * The number of elements in data_ (including holes, but excluding entries
   * behind the last valid particle)
   */
  unsigned data_size_ = 0u;
  /**
   * \internal
   * The capacity of the memory pointed to by data_.
   */
  unsigned data_capacity_ = 100u;
  /**
   * Points to a dynamically allocated array of ParticleData objects. The
   * allocated size is stored in data_capacity_ and the used range (starting
   * from index 0) is stored in data_size_.
   */
  std::unique_ptr<ParticleData[]> data_;

  /**
   * Stores the indexes in data_ that do not hold valid particle data and should
   * be reused when new particles are added.
   */
  std::vector<unsigned> dirty_;
};

}  // namespace smash

#endif  // SRC_INCLUDE_PARTICLES_H_
