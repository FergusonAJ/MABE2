/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  ANNBrain.hpp
 *  @brief Quick implmentation of feed-forward sigmoid-only ANNs
 *  @note Status: ALPHA
 */

#ifndef MABE_ANN_BRAIN_H
#define MABE_ANN_BRAIN_H

#include "../../core/MABE.hpp"
#include "../../core/Organism.hpp"
#include "../../core/OrganismManager.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/math/MVector.hpp"
#include "emp/math/MMatrix.hpp"

namespace mabe {

  class ANNBrain : public OrganismTemplate<ANNBrain> {
  protected:
    emp::BitVector bits;
    size_t input_nodes;
    size_t output_nodes;
    size_t num_layers;
    emp::vector<emp::MVector> layer_vec;
    emp::vector<emp::MMatrix> weight_vec;
    emp::vector<emp::MVector> bias_vec;
    emp::BitVector genome;

  public:
    ANNBrain(OrganismManager<ANNBrain> & _manager)
      : OrganismTemplate<ANNBrain>(_manager), genome() { }
    //ANNBrain(const ANNBrain &) = default;
    //ANNBrain(ANNBrain &&) = default;
    //ANNBrain(const emp::BitVector & _genome, OrganismManager<ANNBrain> & _manager)
    //  : OrganismTemplate<ANNBrain>(_manager), genome(_genome) { }
    //ANNBrain(size_t N, OrganismManager<ANNBrain> & _manager)
    //  : OrganismTemplate<ANNBrain>(_manager), bits(N) { }
    ~ANNBrain() { ; }

    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;             ///< Probability of each bit mutating on reproduction.
      std::string input_name = "brain_inputs";    ///< Name of trait to use as inputs to ANN
      std::string output_name = "brain_outputs";  ///< Name of trait to use for ANN outputs
      emp::Binomial mut_dist;             ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;           ///< A pre-allocated vector for mutation sites. 
      std::string nodes_per_layer_str;    ///< Number of nodes in each hidden layer (comma separated)
      emp::vector<size_t> nodes_per_layer_vec;  ///< Extracted values from string above
      size_t num_inputs = 3;              ///< Number of input nodes in the ANN
      size_t num_outputs = 2;             ///< Number of output nodes in the ANN
    };

    /// Use "to_string" to convert.
    std::string ToString() const override { return emp::to_string(genome); }

    size_t Mutate(emp::Random & random) override {
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);

      if (num_muts == 0) return 0;
      if (num_muts == 1) {
        const size_t pos = random.GetUInt(genome.size());
        genome.Toggle(pos);
        return 1;
      }

      // Only remaining option is num_muts > 1.
      auto & mut_sites = SharedData().mut_sites;
      mut_sites.Clear();
      for (size_t i = 0; i < num_muts; i++) {
        const size_t pos = random.GetUInt(genome.size());
        if (mut_sites[pos]) { --i; continue; }  // Duplicate position; try again.
        mut_sites.Set(pos);
      }
      genome ^= mut_sites;
      GenerateLayersFromGenome();

      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      emp::RandomizeBitVector(genome, random, 0.5);
      GenerateLayersFromGenome();
    }
    
    void Initialize(emp::Random & random) override {
      Randomize(random);
    }

    void SpliceStringIntoVec(const std::string & s, 
        emp::vector<size_t> & vec, const unsigned char sep = ','){
      vec.clear();
      std::stringstream stream;
      size_t num_chars = 0;
      for(size_t idx = 0; idx < s.size(); idx++){
        if(s[idx] == sep){
          if(num_chars > 0){
            vec.push_back(std::stod(stream.str()));
            stream.str("");
            num_chars = 0;
          }
        }
        else {
          stream << s[idx];
          ++num_chars;
        }
      } 
      if(num_chars > 0){
        vec.push_back(std::stoull(stream.str()));
        stream.str("");
      }
    }

    size_t CalculateGenomeSize(){
      size_t res = 0;
      // For each hidden layer, add biases and weights connecting to previous layer
      for(size_t layer_idx = 0; layer_idx < SharedData().nodes_per_layer_vec.size(); ++layer_idx){
        res += SharedData().nodes_per_layer_vec[layer_idx]; // Biases
        if(layer_idx != 0){ // Weights between this layer and previous
          res += SharedData().nodes_per_layer_vec[layer_idx] * 
              SharedData().nodes_per_layer_vec[layer_idx-1];
        } 
      }
      //res += SharedData().num_inputs; // Input node biases
      res += SharedData().num_outputs; // Output node biases
      if(SharedData().nodes_per_layer_vec.size() > 0){ // If we have at least one layer
        // Add weights from input nodes to first hidden layer
        res += SharedData().num_inputs * SharedData().nodes_per_layer_vec[0];
        // Add weights from last hidden layer to output nodes
        res += SharedData().num_outputs * 
            SharedData().nodes_per_layer_vec[SharedData().nodes_per_layer_vec.size() - 1];
      } 
      else{ // No hidden layers, add weights from input to output
        res += SharedData().num_inputs * SharedData().num_outputs; 
      }
      return res;
    }

    void GenerateLayersFromGenome(){
      const emp::vector<size_t> & npl_vec = SharedData().nodes_per_layer_vec;
      size_t genome_idx = 0;
      if(npl_vec.size() == 0){ // No hidden layers
        // Create the layers
        layer_vec.resize(2); 
        layer_vec[0] = emp::MVector(SharedData().num_inputs); 
        layer_vec[1] = emp::MVector(SharedData().num_outputs); 
        // Create the weights
        weight_vec.resize(1, emp::MMatrix(SharedData().num_inputs, SharedData().num_outputs));
        for(size_t input_idx = 0; input_idx < SharedData().num_inputs; ++input_idx){
          for(size_t output_idx = 0; output_idx < SharedData().num_outputs; ++output_idx){
            weight_vec[0].Set(input_idx, output_idx, genome[genome_idx++]);
          }
        }
        // Create output layer biases
        bias_vec.resize(1);
        bias_vec[0] = emp::MVector(SharedData().num_outputs);
        for(size_t output_idx = 0; output_idx < SharedData().num_outputs; ++output_idx){
          bias_vec[0][output_idx] = genome[genome_idx++];
        }
      }
      else{ // Hidden layers present
        // Create the layers
        layer_vec.resize(npl_vec.size() + 2); 
        size_t cur_idx = 0;
        layer_vec[cur_idx++] = emp::MVector(SharedData().num_inputs); 
        for(size_t layer_idx = 0; layer_idx < npl_vec.size(); ++layer_idx){
          layer_vec[cur_idx++] = emp::MVector(npl_vec[layer_idx]); 
        }
        layer_vec[cur_idx++] = emp::MVector(SharedData().num_outputs); 
        // Create the first weight matrix
        weight_vec.resize(1, emp::MMatrix(SharedData().num_inputs, npl_vec[0]));
        for(size_t input_idx = 0; input_idx < SharedData().num_inputs; ++input_idx){
          for(size_t hidden_idx = 0; hidden_idx < npl_vec[0]; ++hidden_idx){
            weight_vec[0].Set(input_idx, hidden_idx, genome[genome_idx++]);
          }
        }
        // Create hidden layer weight matrices
        for(size_t layer_a_idx = 0; layer_a_idx < npl_vec.size() - 1; ++layer_a_idx){
          emp::MMatrix M(npl_vec[layer_a_idx], npl_vec[layer_a_idx + 1]);
          for(size_t a_idx = 0; a_idx < npl_vec[layer_a_idx]; ++a_idx){
            for(size_t b_idx = 0; b_idx < npl_vec[layer_a_idx + 1]; ++b_idx){
              M.Set(a_idx, b_idx, genome[genome_idx++]);
            }
          }
          weight_vec.push_back(M);
        }
        // Create final weight matrix
        emp::MMatrix final_M(npl_vec[npl_vec.size() - 1], SharedData().num_outputs);
        for(size_t hidden_idx = 0; hidden_idx < npl_vec[npl_vec.size() - 1]; ++hidden_idx){
          for(size_t output_idx = 0; output_idx < SharedData().num_outputs; ++output_idx){
            final_M.Set(hidden_idx, output_idx, genome[genome_idx++]);
          }
        }
        weight_vec.push_back(final_M);
        // Create bias vectors
        bias_vec.resize(npl_vec.size() + 1);
        for(size_t layer_idx = 0; layer_idx < npl_vec.size(); ++layer_idx){
          bias_vec[layer_idx] = emp::MVector(npl_vec[layer_idx]);
          for(size_t node_idx = 0; node_idx < npl_vec[layer_idx]; ++node_idx)
            bias_vec[layer_idx][node_idx] = genome[genome_idx++];
        }
        bias_vec[npl_vec.size()] = emp::MVector(SharedData().num_outputs);
        for(size_t output_idx = 0; output_idx < SharedData().num_outputs; ++output_idx){
          bias_vec[npl_vec.size()][output_idx] = genome[genome_idx++];
        }
      }
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      //GetManager().LinkFuns<size_t>([this](){ return bits.size(); },
      //                 [this](const size_t & N){ return bits.Resize(N); },
      //                 "N", "Number of bits in organism");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each bit mutating on reproduction.");
      GetManager().LinkVar(SharedData().input_name, "input_name",
                      "Name of variable that contains ANN input data.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain bit sequence.");
      GetManager().LinkVar(SharedData().num_inputs, "num_inputs",
                      "Number of input nodes in the ANN");
      GetManager().LinkVar(SharedData().num_outputs, "num_outputs",
                      "Number of output nodes in the ANN");
      GetManager().LinkVar(SharedData().nodes_per_layer_str, "nodes_per_layer", 
                      "Comma-separated string denoting the number of nodes in each hidden layer");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      SpliceStringIntoVec(SharedData().nodes_per_layer_str, SharedData().nodes_per_layer_vec);
      size_t genome_size = CalculateGenomeSize();
      genome.resize(genome_size);
      GenerateLayersFromGenome();
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, genome.size());

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(genome.size());

      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().input_name,
                                  "Input to ANN.",
                                  emp::vector<double>());
      // Setup the output trait.
      GetManager().AddOwnedTrait(SharedData().output_name,
                                  "Output of ANN",
                                  emp::BitVector(SharedData().num_outputs));
    }
    
    /// Put the bits in the correct output position.
    void GenerateOutput() override {
      emp::vector<double> input_vec = GetVar<emp::vector<double>>(SharedData().input_name);
      emp_assert(input_vec.size() == layer_vec[0].cardinality(), 
          "Expected inputs: ", layer_vec[0].cardinality() , " Received inputs: ", input_vec.size());
      emp::BitVector output_vec(SharedData().num_outputs);

      if(SharedData().nodes_per_layer_vec.size() == 0){ // No hidden layers
        layer_vec[1] = layer_vec[0] * weight_vec[0] + bias_vec[0];
        for(size_t bit_idx = 0; bit_idx < output_vec.size(); ++bit_idx){
          output_vec[bit_idx] = layer_vec[1][bit_idx] > 0.5 ? 1 : 0; 
        } 
      }
      else{ // Hidden layers
        emp::MVector cur_vec = layer_vec[0];
        for(size_t layer_idx = 0; layer_idx < layer_vec.size() - 1; ++layer_idx){
          layer_vec[layer_idx + 1] = 
              layer_vec[layer_idx] * weight_vec[layer_idx] + bias_vec[layer_idx];      
        } 
        for(size_t bit_idx = 0; bit_idx < output_vec.size(); ++bit_idx){
          output_vec[bit_idx] = layer_vec[layer_vec.size() - 1][bit_idx] > 0.5 ? 1 : 0; 
        } 
         
      }
      SetVar<emp::BitVector>(SharedData().output_name, output_vec);
    }
  };

  MABE_REGISTER_ORG_TYPE(ANNBrain, "Simple artificial neural network");
}

#endif