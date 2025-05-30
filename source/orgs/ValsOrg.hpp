/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
 *
 *  @file ValsOrg.hpp
 *  @brief An organism consisting of a fixed-size series of values of type double.
 *  @note Status: ALPHA
 */

#ifndef MABE_VALS_ORGANISM_H
#define MABE_VALS_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/datastructs/span_utils.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/polyfill/span.hpp"

namespace mabe {

  class ValsOrg : public OrganismTemplate<ValsOrg> {
  protected:
    // How do we enforce limits on values?
    enum BoundType {
      LIMIT_NONE=0,  // No boundary limit.  (e.g., in a 0 to 100 range, 103 would stay 103)
      LIMIT_CLAMP,   // Hard stop at boundary and stay there.    (e.g., 103 would go to 100)
      LIMIT_WRAP,    // Loop around through opposite boundary?   (e.g., 103 would go to 3)
      LIMIT_REBOUND, // Reflect back by amount limit was crossed (e.g., 103 would go to 97)
      LIMIT_ERROR    // Invalid limit type.
    };

    void CalculateTotal(const std::span<double> & vals) {
      double total = 0.0;
      for (double x : vals) total += x;
      SetTrait<double>(SharedData().total_name, total);
    }

  public:
    struct ManagerData : public Organism::ManagerData {
      emp::String genome_name = "vals";  ///< Name of trait that should be used to access values.
      emp::String total_name = "total";  ///< Name of trait that indicate total of all values.
      size_t num_vals = 100;             ///< Number of values in this genome.
      double mut_prob = 0.01;            ///< Probability of position mutating on reproduction.
      double mut_size = 1.0;             ///< Standard deviation of mutations.
      double min_value = 0.0;            ///< Smallest that values are allowed to be.
      double max_value = 100.0;          ///< Largest that values are allowed to be.
      BoundType upper_bound = LIMIT_REBOUND;
      BoundType lower_bound = LIMIT_REBOUND;

      // Helper member variables.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites. 
      bool init_random = true;           ///< Should we randomize ancestor?  (false = all 0.0)

      // Helper functions.
      inline void ApplyBounds(double & value);              ///< Put a single value back in range.
      inline void ApplyBounds(std::span<double> & vals);  ///< Put all values back in range.
    };

    ValsOrg(OrganismManager<ValsOrg> & _manager)
      : OrganismTemplate<ValsOrg>(_manager) { }
    ValsOrg(const ValsOrg &) = default;
    ValsOrg(ValsOrg &&) = default;
    ~ValsOrg() { ; }

    emp::String ToString() const override {
      std::span<const double> vals = GetTrait<double>(SharedData().genome_name, SharedData().num_vals);
      return emp::MakeString(vals);
    }
    
    void GenomeFromString(const std::string & new_genome) override {
      std::span<double> vals = GetTrait<double>(SharedData().genome_name, SharedData().num_vals);
      emp::vector<std::string> sliced_vec = emp::slice(new_genome, ' ');
      emp_assert(sliced_vec.size() == vals.size());
      for(size_t idx = 0; idx < SharedData().num_vals; ++idx){
        vals[idx] = std::stod(sliced_vec[idx]);
      }
      SharedData().ApplyBounds(vals); // Make sure the values are in the allowed range.
    }

    size_t Mutate(emp::Random & random) override {
      // Identify number of and positions for mutations.
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);
      emp::BitVector & mut_sites = SharedData().mut_sites;
      mut_sites.ChooseRandom(random, num_muts);
      std::span<double> vals = GetTrait<double>(SharedData().genome_name, SharedData().num_vals);
      double & total = GetTrait<double>(SharedData().total_name);

      // Trigger mutations at the identified positions.
      int mut_pos = mut_sites.FindOne();
      while (mut_pos != -1) {
        double & cur_val = vals[mut_pos];        // Identify the next site to mutate.
        total -= cur_val;                        // Remove old value from the total.
        cur_val += random.GetNormal();           // Mutate the value at the site.
        SharedData().ApplyBounds(cur_val);       // Make sure the value stays in the allowed range.
        total += cur_val;                        // Add the update value back into the total.
        mut_pos = mut_sites.FindOne(mut_pos+1);  // Move on to the next site to mutate.
      }

      SetTrait<double>(SharedData().total_name, total);  // Store total in data map.
      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      std::span<double> vals = GetTrait<double>(SharedData().genome_name, SharedData().num_vals);
      double total = 0.0;
      for (double & x : vals) {
        x = random.GetDouble(SharedData().min_value, SharedData().max_value);
        total += x;
      }
      SetTrait<double>(SharedData().total_name, total);  // Store total in data map.
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random);
      else { 
        std::span<double> vals = GetTrait<double>(SharedData().genome_name, SharedData().num_vals);
        double total = 0.0;
        for (double & x : vals) x = 0.0;
        SetTrait<double>(SharedData().total_name, total);  // Store total in data map.
      }
    }


    /// Put the values in the correct output positions.
    void GenerateOutput() override {
      /// Output is already stored in the DataMap.
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().num_vals, "N", "Number of values in organism");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each value mutating on reproduction.");
      GetManager().LinkVar(SharedData().mut_size, "mut_size",
                      "Standard deviation on size of mutations.");
      GetManager().LinkVar(SharedData().min_value, "min_value",
                      "Lower limit for value fields.");
      GetManager().LinkVar(SharedData().max_value, "max_value",
                      "Upper limit for value fields.");
      GetManager().LinkMenu(
        SharedData().lower_bound, "lower_bound", "How should the lower limit be enforced?",
        LIMIT_NONE, "no_limit", "Allow values to be arbitrarily low.",
        LIMIT_CLAMP, "clamp", "Reduce too-low values to min_value.",
        LIMIT_WRAP, "wrap", "Make low values loop around to maximum.",
        LIMIT_REBOUND, "rebound", "Make low values 'bounce' back up." );
      GetManager().LinkMenu(
        SharedData().upper_bound, "upper_bound", "How should the upper limit be enforced?",
        LIMIT_NONE, "no_limit", "Allow values to be arbitrarily high.",
        LIMIT_CLAMP, "clamp", "Reduce too-high values to max_value.",
        LIMIT_WRAP, "wrap", "Make high values loop around to minimum.",
        LIMIT_REBOUND, "rebound", "Make high values 'bounce' back down." );
      GetManager().LinkVar(SharedData().genome_name, "genome_name",
                      "Name of variable to contain set of values.");
      GetManager().LinkVar(SharedData().total_name, "total_name",
                      "Name of variable to contain total of all values.");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = all 0.0)");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, SharedData().num_vals);

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(SharedData().num_vals);

      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().genome_name,
                                  "Value array output from organism.",
                                  0.0,
                                  SharedData().num_vals);
      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().total_name,
                                  "Total of all organism outputs.",
                                  0.0);
    }
  };

  ///////////////////////////////////////////////////////////////////////////////////////////
  //  Helper functions....

  void ValsOrg::ManagerData::ApplyBounds(double & value) {
    if (value > max_value) {
      switch (upper_bound) {
        case LIMIT_NONE:    break;
        case LIMIT_CLAMP:   value = max_value; break;
        case LIMIT_WRAP:    value -= (max_value - min_value); break;
        case LIMIT_REBOUND: value = 2 * max_value - value; break;
        case LIMIT_ERROR:   break;  // For now; perhaps do something with error?
      }
    }
    else if (value < min_value) {
      switch (lower_bound) {
        case LIMIT_NONE:    break;
        case LIMIT_CLAMP:   value = min_value; break;
        case LIMIT_WRAP:    value += (max_value - min_value); break;
        case LIMIT_REBOUND: value = 2 * min_value - value; break;
        case LIMIT_ERROR:   break;  // For now; perhaps do something with error?
      }
    }
  }

  void ValsOrg::ManagerData::ApplyBounds(std::span<double> & vals) {
    const size_t range_size = max_value - min_value;

    switch (upper_bound) {
    case LIMIT_NONE: break;
    case LIMIT_CLAMP:
      for (double & value : vals) {
        if (value > max_value) value = max_value;
        else if (value < min_value) value = min_value;        
      }
      break;
    case LIMIT_WRAP:
      for (double & value : vals) {
        if (value > max_value) value -= range_size;
        else if (value < min_value) value += range_size;        
      }
      break;
    case LIMIT_REBOUND:
      for (double & value : vals) {
        if (value > max_value) value = 2 * max_value - value;
        else if (value < min_value) value = 2 * min_value - value;
      }
      break;
    default:
      break; // Should probably throw error.
    }
  }


  MABE_REGISTER_ORG_TYPE(ValsOrg, "Organism consisting of a series of N floating-point values.");
}

#endif
