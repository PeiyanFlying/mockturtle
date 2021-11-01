/* mockturtle: C++ logic network library
 * Copyright (C) 2018-2021  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "experiments.hpp"
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/networks/buffered.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/algorithms/aqfp/buffer_insertion.hpp>
#include <mockturtle/algorithms/aqfp/buffer_verification.hpp>

#include <lorina/lorina.hpp>
#include <fmt/format.h>
#include <string>
#include <algorithm>

/* Note: Please download this repository: https://github.com/lsils/ASPDAC2021_exp 
   and copy the folder ASPDAC2021_exp/experiments/benchmarks_aqfp/ to the build path of mockturtle. */
int main()
{
  using namespace experiments;
  using namespace mockturtle;

  experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t>
    exp( "optimal_buffer_insertion", "benchmark", "#gates", "depth", "opt", "depth_JJ" );

  static const std::string benchmarks_aqfp[] = {
    /*"5xp1",*/ "c1908", "c432", "c5315", "c880", "chkn", "count", "dist", "in5", "in6", "k2",
    "m3", "max512", "misex3", "mlp4", "prom2", "sqr6", "x1dn"};

  for ( auto const& benchmark : benchmarks_aqfp )
  {
    if ( benchmark != "count" ) continue;
    fmt::print( "[i] processing {}\n", benchmark );
    mig_network mig;
    if ( lorina::read_verilog( "benchmarks_aqfp/" + benchmark + ".v", verilog_reader( mig ) ) != lorina::return_code::success )
      return -1;
    
    buffer_insertion_params ps;
    ps.optimization_effort = buffer_insertion_params::optimal;
    ps.assume.splitter_capacity = 3u;
    ps.assume.branch_pis = true;
    ps.assume.balance_pis = true;
    ps.assume.balance_pos = false;
    
    buffer_insertion aqfp( mig, ps );
    buffered_mig_network bufntk;
    uint32_t const b_OPT = aqfp.run( bufntk );
    depth_view d_buf{bufntk};
    assert( verify_aqfp_buffer( bufntk, ps.assume ) );

    depth_view d{mig};
    exp( benchmark, mig.num_gates(), d.depth(), b_OPT, d_buf.depth() );
  }

  exp.save();
  exp.table();

  return 0;
}
