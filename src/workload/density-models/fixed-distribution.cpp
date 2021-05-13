/* Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fixed-distribution.hpp"

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

BOOST_CLASS_EXPORT(problem::FixedDistribution)

namespace problem{

FixedDistribution::FixedDistribution(){ }

FixedDistribution::FixedDistribution(const Specs& specs) : specs_(specs){
  is_specced_ = true;
}

FixedDistribution::~FixedDistribution() { }

FixedDistribution::Specs FixedDistribution::ParseSpecs(config::CompoundConfigNode density_config){

  Specs specs;
  std::string type;
  double fixed_density;
  density_config.lookupValue("distribution", type);
  density_config.lookupValue("density", fixed_density);

  specs.type = type;
  specs.fixed_density = fixed_density;

  return specs;
}

void FixedDistribution::SetDensity(const double density){
  specs_.fixed_density = density;
}

void FixedDistribution::SetWorkloadTensorSize(const std::uint64_t size ){
  // setter that allows workload tensor size at a latter stage (topology.cpp, PreEvaluationCheck)
  specs_.workload_tensor_size = size;
}

std::uint64_t FixedDistribution::GetTileOccupancyByConfidence(const std::uint64_t tile_shape,
                                                              const double confidence) const
{

  if (confidence == 0){
    return 0;
  } else {
    return ceil(tile_shape * specs_.fixed_density);
  }

}

std::uint64_t FixedDistribution::GetMaxTileOccupancyByConfidence(const tiling::CoordinateSpaceTileInfo& tile,
                                                                 const double confidence) const
{
  std::uint64_t tile_shape = tile.GetShape();
  return FixedDistribution::GetTileOccupancyByConfidence(tile_shape, confidence);
}


std::uint64_t FixedDistribution::GetMaxTileOccupancyByConfidence_LTW (const std::uint64_t tile_shape,
                                                   const double confidence) const
{
  return FixedDistribution::GetTileOccupancyByConfidence(tile_shape, confidence);
}

std::uint64_t FixedDistribution::GetWorkloadTensorSize() const{
  return specs_.workload_tensor_size;
};

std::string FixedDistribution::GetDistributionType() const{
  return specs_.type;
}

// double FixedDistribution::GetTileDensityByConfidence(const std::uint64_t tile_shape,
//                                                      const double confidence,
//                                                      const uint64_t allocated_capacity) const{
//   (void) tile_shape;
//   (void) confidence;
//   (void) allocated_capacity;
//
//   return specs_.fixed_density;
//
// }

double FixedDistribution::GetMaxTileDensityByConfidence(const tiling::CoordinateSpaceTileInfo tile,
                                                        const double confidence) const
{
  (void) tile;
  (void) confidence;

  return specs_.fixed_density;
}

double FixedDistribution::GetTileExpectedDensity( const uint64_t tile_shape ) const {

  (void) tile_shape;
  assert(is_specced_);
  return specs_.fixed_density;
}


double FixedDistribution::GetProbability(const std::uint64_t tile_shape,
                                         const std::uint64_t nnz_vals) const {

  assert(is_specced_);

  if (tile_shape == 1){
    // tile shape 1 is a special case, where we use a binomial representation to reflect the probability
    return nnz_vals == 0 ? 1 - specs_.fixed_density : specs_.fixed_density;

  } else if (ceil(tile_shape * specs_.fixed_density) != nnz_vals){
    return 0.0;

  } else {
    // fixed distribution is not stochastic
    return 1.0;
  }
}

double FixedDistribution::GetTileOccupancyProbability(const tiling::CoordinateSpaceTileInfo& tile,
                                                      const std::uint64_t occupancy) const
{
  std::uint64_t tile_shape = tile.GetShape();
  return GetProbability(tile_shape, occupancy);
}


double FixedDistribution::GetExpectedTileOccupancy (const tiling::CoordinateSpaceTileInfo tile) const
{
  return tile.GetShape() * specs_.fixed_density;
}

}