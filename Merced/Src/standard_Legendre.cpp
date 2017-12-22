/*
 * ******** merced: calculate the transfer matrix *********
 * $Revision: 1 $
 * $Date: 2011-01-28 (Fri, Jan 28, 2011) $
 * $Author: hedstrom $
 * $Id: standard_Legendre.cpp 1 2011-01-28 hedstrom $
 * ******** merced: calculate the transfer matrix *********
 *
 * # <<BEGIN-copyright>>
  Copyright (c) 2017, Lawrence Livermore National Security, LLC.
  Produced at the Lawrence Livermore National Laboratory.
  Written by the LLNL Nuclear Data and Theory group
          (email: mattoon1@llnl.gov)
  LLNL-CODE-725546.
  All rights reserved.
  
  This file is part of the Merced package, used to generate nuclear reaction
  transfer matrices for deterministic radiation transport.
  
  
      Please also read this link - Our Notice and Modified BSD License
  
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the disclaimer below.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the disclaimer (as noted below) in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of LLNS/LLNL nor the names of its contributors may be used
        to endorse or promote products derived from this software without specific
        prior written permission.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
  THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  
  Additional BSD Notice
  
  1. This notice is required to be provided under our contract with the U.S.
  Department of Energy (DOE). This work was produced at Lawrence Livermore
  National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.
  
  2. Neither the United States Government nor Lawrence Livermore National Security,
  LLC nor any of their employees, makes any warranty, express or implied, or assumes
  any liability or responsibility for the accuracy, completeness, or usefulness of any
  information, apparatus, product, or process disclosed, or represents that its use
  would not infringe privately-owned rights.
  
  3. Also, reference herein to any specific commercial products, process, or services
  by trade name, trademark, manufacturer or otherwise does not necessarily constitute
  or imply its endorsement, recommendation, or favoring by the United States Government
  or Lawrence Livermore National Security, LLC. The views and opinions of authors expressed
  herein do not necessarily state or reflect those of the United States Government or
  Lawrence Livermore National Security, LLC, and shall not be used for advertising or
  product endorsement purposes.
  
 * # <<END-copyright>>
*/
// implementation of the classes used to handle Legendre expansions of energy probability density

#include <cmath>
#include <cstdlib>
#include <cfloat>
#ifdef _OPENMP
 #include <omp.h>
#endif

#include "standard_Legendre.hpp"
#include "messaging.hpp"
#include "global_params.hpp"

// ************* class standard_Legendre_vector *****************
// ----------- standard_Legendre_vector::read_coef --------------
// Reads the Legendre coefficients of the energy probability density
void standard_Legendre_vector::read_coef( data_parser& infile, int num_Eout, int max_order )
{
  standard_Legendre_vector::iterator new_Eout_ptr;
  // read the data
  for( int Eout_count = 0; Eout_count < num_Eout; ++Eout_count )
  {
    // make a new set of Legendre coefficients
    new_Eout_ptr = insert( end( ), Legendre_coefs( ) );
    new_Eout_ptr->initialize( max_order );
    new_Eout_ptr->set_E_out( infile.get_next_double( ) );  // energy of outgoing particle
    int file_order = infile.get_next_int( ) - 1;  // Legendre order of input data
    int save_coefs = ( file_order > max_order ) ? max_order : file_order;
    int coef_count = 0;
    for( ; coef_count <= save_coefs; ++coef_count )
    {
      ( *new_Eout_ptr )[ coef_count ] = infile.get_next_double( );
    }
    // we may need to discard high-order coefficients
    for( coef_count = save_coefs+1; coef_count <= file_order; ++coef_count )
    {
      infile.get_next_double( );
    }
  }
  // ensure proper normalization
  renorm( );

  if( Ein_interp.qualifier == UNITBASE )
  {
    to_unit_base( );
  }
  else if( Ein_interp.qualifier == CUMULATIVE_POINTS )
  {
    form_cum_prob( );
  }
}
// ----------- standard_Legendre_vector::append_data --------------
// Appends a copy of the data to the list
void standard_Legendre_vector::append_data( double E_out, const Legendre_coefs &to_copy )
{
  // make a new set of Legendre coefficients
  standard_Legendre_vector::iterator new_Eout_ptr;
  new_Eout_ptr = insert( end( ), Legendre_coefs( ) );
  new_Eout_ptr->initialize( to_copy.order );
  new_Eout_ptr->set_E_out( E_out );
  for( int L_count = 0; L_count <= to_copy.order; ++L_count )
  {
    (*new_Eout_ptr)[ L_count ] = to_copy.value( L_count );
  }
}
// ----------- standard_Legendre_vector::extrapolate_copy --------------
// Copies a vector with extrapolation
void standard_Legendre_vector::extrapolate_copy(
     const standard_Legendre_vector& vector_from,
     double min_E, double max_E )
{
  set_E_in( vector_from.get_E_in( ) );
  Ein_interp = vector_from.Ein_interp;
  Eout_interp = vector_from.Eout_interp;

  Legendre_coefs null_entry;  // for creating entries, initially all zero
  null_entry.initialize( 0 );

  static double e_tol = Global.Value( "E_tol" );
  standard_Legendre_vector::const_iterator this_entry = vector_from.begin( );

  double E_value = this_entry->get_E_out( );
  if( min_E < ( 1.0 - e_tol )*E_value )
  {
    // extrapolate a zero head of the list
    append_data( min_E, null_entry );
    if( this_entry->value( 0 ) != 0.0 )
    {
      append_data( ( 1.0 - e_tol )*E_value, null_entry );
    }
    append_data( E_value, *this_entry );
  }
  else if( min_E < E_value )
  {
    append_data( min_E, *this_entry );
  }
  else
  {
    append_data( E_value, *this_entry );
  }
  for( ++this_entry; this_entry != vector_from.end( ); ++this_entry )
  {
    append_data( this_entry->get_E_out( ), *this_entry );
  }
  standard_Legendre_vector::iterator Lptr = end( );
  --Lptr;
  E_value = Lptr->get_E_out( );
  if( max_E > ( 1.0 + e_tol )*E_value )
  {
    // extrapolate a zero tail
    if( Lptr->value( 0 ) != 0.0 )
    {
      append_data( ( 1.0 + e_tol )*E_value, null_entry );
    }
    append_data( max_E, null_entry );
  }
  else if( max_E > E_value )
  {
    Lptr->set_E_out( max_E );
  }
}
// ----------- standard_Legendre_vector::form_cum_prob --------------
// Forms the list of cumulative probabilities
void standard_Legendre_vector::form_cum_prob( )
{
  // copy the data
  cum_prob.Eout_interp = Eout_interp;
  for( standard_Legendre_vector::const_iterator Eout_ptr = begin( );
       Eout_ptr != end( ); ++Eout_ptr )
  {
    cumulative_prob_list::iterator cum_prob_ptr = cum_prob.insert(
      cum_prob.end( ), cumulative_prob_entry( ) );
    cum_prob_ptr->E_out = Eout_ptr->get_E_out( );
    cum_prob_ptr->Prob = Eout_ptr->value( 0 );
  }
  // now form the slopes and cumulative probabilities
  if( Eout_interp == HISTOGRAM )
  {
    cum_prob.get_cum_prob_flat( );
  }
  else // lin-lin
  {
    cum_prob.get_cum_prob_linlin( );
  }
}

// *************** class standard_Legendre_param *************************
// ------------------ standard_Legendre_param::initialize ----------------
// Allocates space
void standard_Legendre_param::initialize( int Order )
{
  mid_lower_Eout.initialize( Order );
  mid_upper_Eout.initialize( Order );
  use_prev_Eout.initialize( Order );
  use_next_Eout.initialize( Order );
  Ein0_data.prev_data.initialize( Order );
  Ein0_data.next_data.initialize( Order );
  Ein1_data.prev_data.initialize( Order );
  Ein1_data.next_data.initialize( Order );
}
// ------------------ standard_Legendre_param::reset_start ----------------
// Initializes the data pointers for one incident energy range
void standard_Legendre_param::reset_start( )
{
  Ein0_data.set_E_in( this_Ein->get_E_in( ) );
  Ein1_data.set_E_in( next_Ein->get_E_in( ) );
  left_Ein = this_Ein->get_E_in( );
  right_Ein = next_Ein->get_E_in( );

  // initialize the pointers
  if( Ein_interp.qualifier == DIRECT )
  {
    if( Ein_interp.flag == LINLIN )
    {
      setup_Ein_linlin( );
    }
    else if( Ein_interp.flag == HISTOGRAM )
    {
      setup_Ein_flat( );
    }
  }
  else if( Ein_interp.qualifier == UNITBASE )
  {
    setup_Ein_ubase( );
  }
  else if( Ein_interp.qualifier == CUMULATIVE_POINTS )
  {
    setup_Ein_cum_prob( );
  }

  if( Ein_interp.qualifier != CUMULATIVE_POINTS )
  {
    // for the range of Eout values
    double lower_Eout;
    double higher_Eout;

    lower_Eout = ( left_ptr->get_E_out( ) > right_ptr->get_E_out( ) )?
        left_ptr->get_E_out( ) : right_ptr->get_E_out( );
    higher_Eout = ( next_left_ptr->get_E_out( ) < next_right_ptr->get_E_out( ) )?
        next_left_ptr->get_E_out( ) : next_right_ptr->get_E_out( );
    if( higher_Eout <= lower_Eout )
    {
      FatalError( "standard_Legendre_param::reset_start", "Check the Eout values." );
    }

    // Interpolate to the common Eout values
    common_low_Eout( lower_Eout );
    common_high_Eout( higher_Eout );
  }

  // physical outgoing energy ranges
  if( Ein_interp.qualifier == DIRECT )
  {
    Eout_0_range.x = Ein0_data.prev_data.get_E_out( );
    Eout_0_range.y = Ein0_data.next_data.get_E_out( );
    Eout_1_range.x = Ein1_data.prev_data.get_E_out( );
    Eout_1_range.y = Ein1_data.next_data.get_E_out( );
  }
  else
  {
    Eout_0_range.x = Ein0_data.ubase_map.Eout_min;
    Eout_0_range.y = Ein0_data.ubase_map.un_unit_base( Ein0_data.next_data.get_E_out( ) );
    Eout_1_range.x = Ein1_data.ubase_map.Eout_min;
    Eout_1_range.y = Ein1_data.ubase_map.un_unit_base( Ein1_data.next_data.get_E_out( ) );
  }
}
// ---------------- standard_Legendre_param::setup_Ein_ubase ------------------
// Sets up the data for unit-base interpolation in incident energy
void standard_Legendre_param::setup_Ein_ubase( )
{
  // lower incident energy
  left_ptr = this_Ein->begin( );
  next_left_ptr = left_ptr;
  ++next_left_ptr;
  last_left_ptr = this_Ein->end( );

  // higher incident energy
  right_ptr = next_Ein->begin( );
  next_right_ptr = right_ptr;
  ++next_right_ptr;
  last_right_ptr = next_Ein->end( );

  // save the unit-base interpolation
  Ein0_data.ubase_map.copy( this_Ein->ubase_map );
  Ein1_data.ubase_map.copy( next_Ein->ubase_map );
}
// ---------------- standard_Legendre_param::setup_Ein_cum_prob ------------------
// Sets up the data for cumulative points interpolation in incident energy
void standard_Legendre_param::setup_Ein_cum_prob( )
{
  // lower incident energy
  left_ptr = this_Ein->begin( );
  next_left_ptr = left_ptr;
  ++next_left_ptr;
  last_left_ptr = this_Ein->end( );
  left_cum_prob = this_Ein->cum_prob.begin( );
  next_left_cum_prob = left_cum_prob;
  ++next_left_cum_prob;

  // skip zero probability intervals
  while( ( left_cum_prob->Prob == 0.0 ) &&
         ( left_cum_prob->slope == 0.0 ) )
  {
    left_cum_prob = next_left_cum_prob;
    ++next_left_cum_prob;
    left_ptr = next_left_ptr;
    ++next_left_ptr;
  }

  // higher incident energy
  right_ptr = next_Ein->begin( );
  next_right_ptr = right_ptr;
  ++next_right_ptr;
  last_right_ptr = next_Ein->end( );
  right_cum_prob = next_Ein->cum_prob.begin( );
  next_right_cum_prob = right_cum_prob;
  ++next_right_cum_prob;

  // skip zero probability intervals
  while( ( right_cum_prob->Prob == 0.0 ) &&
         ( right_cum_prob->slope == 0.0 ) )
  {
    right_cum_prob = next_right_cum_prob;
    ++next_right_cum_prob;
    right_ptr = next_right_ptr;
    ++next_right_ptr;
  }

  // for the range of cumulative probabilities A
  //  double lower_A = 0.0;
  double higher_A = ( next_left_cum_prob->cum_prob < next_right_cum_prob->cum_prob )?
      next_left_cum_prob->cum_prob : next_right_cum_prob->cum_prob;

  // set up Ein0_data and Ein1_data
  setup_low_A( );
  setup_high_A( higher_A );
  Ein0_data.to_unit_base( );
  Ein1_data.to_unit_base( );
}
// ---------------- standard_Legendre_param::setup_Ein_linlin ------------------
// Sets up the data for direct linlin interpolation in incident energy
void standard_Legendre_param::setup_Ein_linlin( )
{
  // remove previous data
  if( !low_linlin.empty( ) )
  {
    low_linlin.erase( low_linlin.begin( ), low_linlin.end( ) );
    high_linlin.erase( high_linlin.begin( ), high_linlin.end( ) );
  }

  // get the outgoing energy range
  left_ptr = this_Ein->begin( );
  right_ptr = next_Ein->begin( );
  double Eout_min = ( left_ptr->get_E_out( ) < right_ptr->get_E_out( ) ) ?
    left_ptr->get_E_out( ) : right_ptr->get_E_out( );

  left_ptr = this_Ein->end( );
  --left_ptr;
  right_ptr = next_Ein->end( );
  --right_ptr;
  double Eout_max = ( left_ptr->get_E_out( ) > right_ptr->get_E_out( ) ) ?
    left_ptr->get_E_out( ) : right_ptr->get_E_out( );

  // extrapolate as zero
  low_linlin.extrapolate_copy( *this_Ein, Eout_min, Eout_max );
  high_linlin.extrapolate_copy( *next_Ein, Eout_min, Eout_max );

  // set pointers at lower incident energy
  left_ptr = low_linlin.begin( );
  next_left_ptr = left_ptr;
  ++next_left_ptr;
  last_left_ptr = low_linlin.end( );

  // higher incident energy
  right_ptr = high_linlin.begin( );
  next_right_ptr = right_ptr;
  ++next_right_ptr;
  last_right_ptr = high_linlin.end( );
}
// ---------------- standard_Legendre_param::setup_Ein_flat ------------------
// Sets up the data for histogram interpolation in incident energy
void standard_Legendre_param::setup_Ein_flat( )
{
  // lower incident energy
  left_ptr = this_Ein->begin( );
  next_left_ptr = left_ptr;
  ++next_left_ptr;
  last_left_ptr = this_Ein->end( );
  // we need to set up the right pointers
  right_ptr = this_Ein->begin( );
  next_right_ptr = next_left_ptr;
  last_right_ptr = this_Ein->end( );
}
// ------------------ standard_Legendre_param::get_next_Eout ----------------
// Increments the data pointers for one incident energy range
bool standard_Legendre_param::get_next_Eout( )
{
  // ignore intervals with probability less than skip_tol
  static double skip_tol = Global.Value( "abs_tol" );

  bool done = false;
  if( Ein_interp.qualifier == CUMULATIVE_POINTS )
  {
    // undo the unit-base map before copying data
    Ein0_data.un_unit_base( );
    Ein1_data.un_unit_base( );
  }

  Ein0_data.prev_data.set_E_out( Ein0_data.next_data.get_E_out( ) );
  Ein0_data.prev_data.copy_coef( Ein0_data.next_data );
  Ein1_data.prev_data.set_E_out( Ein1_data.next_data.get_E_out( ) );
  Ein1_data.prev_data.copy_coef( Ein1_data.next_data );

  // update the pointers
  if( next_left_ptr->get_E_out( ) <= Ein0_data.prev_data.get_E_out( ) )
  {
    left_ptr = next_left_ptr;
    ++next_left_ptr;
    if( next_left_ptr == last_left_ptr )
    {
      return true;
    }
    if( Ein_interp.qualifier == CUMULATIVE_POINTS )
    {
      left_cum_prob = next_left_cum_prob;
      ++next_left_cum_prob;

      bool do_skip = false;
      // skip intervals with essentially zero probability
      while( next_left_cum_prob->cum_prob - left_cum_prob->cum_prob <= skip_tol )
      {
	do_skip = true;
        left_cum_prob = next_left_cum_prob;
        ++next_left_cum_prob;
        left_ptr = next_left_ptr;
        ++next_left_ptr;
        if( next_left_ptr == last_left_ptr )
        {
          return true;
        }
      }
      if( do_skip )
      {
        Ein0_data.prev_data.set_E_out( left_ptr->get_E_out( ) );
        Ein0_data.prev_data.copy_coef( *left_ptr );
      }
    }
  }
  if( next_right_ptr->get_E_out( ) <= Ein1_data.prev_data.get_E_out( ) )
  {
    right_ptr = next_right_ptr;
    ++next_right_ptr;
    if( next_right_ptr == last_right_ptr )
    {
      return true;
    }
    if( Ein_interp.qualifier == CUMULATIVE_POINTS )
    {
      right_cum_prob = next_right_cum_prob;
      ++next_right_cum_prob;

      bool do_skip = false;
      // skip intervals with essentially zero probability
      while( next_right_cum_prob->cum_prob - right_cum_prob->cum_prob <= skip_tol )
      {
	do_skip = true;
        right_cum_prob = next_right_cum_prob;
        ++next_right_cum_prob;
        right_ptr = next_right_ptr;
        ++next_right_ptr;
        if( next_right_ptr == last_right_ptr )
        {
          return true;
        }
      }
      if( do_skip )
      {
        Ein1_data.prev_data.set_E_out( right_ptr->get_E_out( ) );
        Ein1_data.prev_data.copy_coef( *right_ptr );
      }
    }
  }

  if( Ein_interp.qualifier == CUMULATIVE_POINTS )
  {
    // Interpolate to the common higher cumulative probability
    double higher_A = ( next_left_cum_prob->cum_prob < next_right_cum_prob->cum_prob )?
        next_left_cum_prob->cum_prob : next_right_cum_prob->cum_prob;
    setup_high_A( higher_A );
    Ein0_data.to_unit_base( );
    Ein1_data.to_unit_base( );
  }
  else
  {
    // Interpolate to the common higher Eout value
    double higher_Eout;
    higher_Eout = ( next_left_ptr->get_E_out( ) < next_right_ptr->get_E_out( ) )?
        next_left_ptr->get_E_out( ) : next_right_ptr->get_E_out( );
    common_high_Eout( higher_Eout );
  }

  // Reset the physical E_out ranges
  Eout_0_range.x = Eout_0_range.y;
  Eout_1_range.x = Eout_1_range.y;
  if( ( Ein_interp.qualifier == UNITBASE ) ||
      ( Ein_interp.qualifier == CUMULATIVE_POINTS ) )
  {
    Eout_0_range.y = Ein0_data.ubase_map.un_unit_base( Ein0_data.next_data.get_E_out( ) );
    Eout_1_range.y = Ein1_data.ubase_map.un_unit_base( Ein1_data.next_data.get_E_out( ) );
    // We may have skipped an interval with zero probability
    Eout_0_range.x = Ein0_data.ubase_map.un_unit_base( Ein0_data.prev_data.get_E_out( ) );
    Eout_1_range.x = Ein1_data.ubase_map.un_unit_base( Ein1_data.prev_data.get_E_out( ) );
  }
  else if( Ein_interp.flag == LINLIN )
  {
    Eout_0_range.y = Ein0_data.next_data.get_E_out( );
    Eout_1_range.y = Ein1_data.next_data.get_E_out( );
  }
  else // Ein_interp == HISTOGRAM
  {
    Eout_0_range.y = Ein0_data.next_data.get_E_out( );
    Eout_1_range.y = Eout_0_range.y;
  }
  return done;
}
// ---------------- standard_Legendre_param::common_low_Eout ------------------
// Interpolates (Eout, probability) data to the lower common Eout value
void standard_Legendre_param::common_low_Eout( double lower_Eout )
{
  Ein0_data.prev_data.set_E_out( lower_Eout );
  Ein1_data.prev_data.set_E_out( lower_Eout );

  if( ( left_ptr->get_E_out( ) == lower_Eout ) || ( Ein0_data.Eout_interp == HISTOGRAM ) )
  {
    Ein0_data.prev_data.copy_coef( *left_ptr );
  }
  else
  {
    Ein0_data.prev_data.linlin_interp( lower_Eout, *left_ptr, *next_left_ptr );
  }

  if( ( right_ptr->get_E_out( ) == lower_Eout ) || ( Ein1_data.Eout_interp == HISTOGRAM ) )
  {
    Ein1_data.prev_data.copy_coef( *right_ptr );
  }
  else
  {
    Ein1_data.prev_data.linlin_interp( lower_Eout, *right_ptr, *next_right_ptr );
  }
}
// ---------------- standard_Legendre_param::common_high_Eout ------------------
// Interpolates (Eout, probability) data to the higher common Eout value
void standard_Legendre_param::common_high_Eout( double higher_Eout )
{
  Ein0_data.next_data.set_E_out( higher_Eout );
  Ein1_data.next_data.set_E_out( higher_Eout );

  if( next_left_ptr->get_E_out( ) == higher_Eout )
  {
    Ein0_data.next_data.copy_coef( *next_left_ptr );
  }
  else if( Ein0_data.Eout_interp == HISTOGRAM )
  {
    Ein0_data.next_data.copy_coef( *left_ptr );
  }
  else
  {
    Ein0_data.next_data.linlin_interp( higher_Eout, *left_ptr, *next_left_ptr );
  }

  if( next_right_ptr->get_E_out( ) == higher_Eout )
  {
    Ein1_data.next_data.copy_coef( *next_right_ptr );
  }
  else if( Ein1_data.Eout_interp == HISTOGRAM )
  {
    Ein1_data.next_data.copy_coef( *right_ptr );
  }
  else
  {
    Ein1_data.next_data.linlin_interp( higher_Eout, *right_ptr, *next_right_ptr );
  }
}
// ---------------- standard_Legendre_param::setup_low_A ------------------
// Sets (Eout, probability) data to the lower zero cumulative probability
void standard_Legendre_param::setup_low_A( )
{
  Ein0_data.prev_data.set_E_out( left_ptr->get_E_out( ) );
  Ein1_data.prev_data.set_E_out( right_ptr->get_E_out( ) );

  Ein0_data.prev_data.copy_coef( *left_ptr );
  Ein1_data.prev_data.copy_coef( *right_ptr );
}
// ---------------- standard_Legendre_param::setup_high_A ------------------
// Interpolates (Eout, probability) data to the higher common cumulative probability
void standard_Legendre_param::setup_high_A( double higher_A )
{
  double higher_Eout;

  if( next_left_cum_prob->cum_prob == higher_A )
  {
    Ein0_data.next_data.set_E_out( next_left_ptr->get_E_out( ) );
    Ein0_data.next_data.copy_coef( *next_left_ptr );
  }
  else
  {
    higher_Eout = left_cum_prob->get_cum_inv( higher_A );
    Ein0_data.next_data.set_E_out( higher_Eout );
    if( Ein0_data.Eout_interp == HISTOGRAM )
    {
      Ein0_data.next_data.copy_coef( *left_ptr );
    }
    else
    {
      Ein0_data.next_data.linlin_interp( higher_Eout, *left_ptr, *next_left_ptr );
    }
  }

  if( next_right_cum_prob->cum_prob == higher_A )
  {
    Ein1_data.next_data.set_E_out( next_right_ptr->get_E_out( ) );
    Ein1_data.next_data.copy_coef( *next_right_ptr );
  }
  else
  {
    higher_Eout = right_cum_prob->get_cum_inv( higher_A );
    Ein1_data.next_data.set_E_out( higher_Eout );
    if( Ein1_data.Eout_interp == HISTOGRAM )
    {
      Ein1_data.next_data.copy_coef( *right_ptr );
    }
    else
    {
      Ein1_data.next_data.linlin_interp( higher_Eout, *right_ptr, *next_right_ptr );
    }
  }
}
// ------------------ standard_Legendre_param::unitbase_interpolate ----------------
// Does unit-base interpolation between two incident energies
void standard_Legendre_param::unitbase_interpolate( double Ein )
{
  double dEin = right_Ein - left_Ein;
  if( dEin <= 0.0 )
  {
    FatalError( "standard_Legendre_param::unitbase_interpolate",
		"Incident energies out of order" );
  }
  // lin-lin interpolate the physical energy range
  double alpha = ( Ein - left_Ein )/dEin;
  Eout_range.x = ( 1.0 - alpha )*Eout_0_range.x + alpha*Eout_1_range.x;
  Eout_range.y = ( 1.0 - alpha )*Eout_0_range.y + alpha*Eout_1_range.y;
  // interpolate the unit-base map
  mid_ubase_map.interpolate( alpha, Ein0_data.ubase_map, Ein1_data.ubase_map );

  // interpolate data at the lower and upper unit-base outgoing energies
  if(  Ein_interp.flag == LINLOG )
  {
    alpha = log( Ein / left_Ein )/log( right_Ein / left_Ein );
  }
  mid_lower_Eout.unitbase_interp( Ein, alpha, Ein0_data.prev_data,
     Ein1_data.prev_data );
  mid_upper_Eout.unitbase_interp( Ein, alpha, Ein0_data.next_data,
     Ein1_data.next_data );
  double Eout_min_ubase;
  double Eout_max_ubase;
  if( Eout_interp == HISTOGRAM )
  {
    // save the unit-base outgoing energies that are to be used
    if( use_Eout_min )
    {
      Eout_min_ubase = mid_ubase_map.to_unit_base( Eout_min );
      use_prev_Eout.set_E_out( Eout_min_ubase );
    }
    else
    {
      use_prev_Eout.set_E_out( mid_lower_Eout.get_E_out( ) );
    }
    use_prev_Eout.copy_coef( mid_lower_Eout );
    if( use_Eout_max )
    {
      Eout_max_ubase = mid_ubase_map.to_unit_base( Eout_max );
      use_next_Eout.set_E_out( Eout_max_ubase );
    }
    else
    {
      use_next_Eout.set_E_out( mid_upper_Eout.get_E_out( ) );
    }
    use_next_Eout.copy_coef( mid_lower_Eout );
  }
  else
  {
    // interpolate data to the unit-base outgoing energies that are to be used
    if( use_Eout_min )
    {
      Eout_min_ubase = mid_ubase_map.to_unit_base( Eout_min );
      use_prev_Eout.linlin_interp( Eout_min_ubase, mid_lower_Eout, mid_upper_Eout );
    }
    else
    {
      use_prev_Eout.set_E_out( mid_lower_Eout.get_E_out( ) );
      use_prev_Eout.copy_coef( mid_lower_Eout );
    }
    if( use_Eout_max )
    {
      Eout_max_ubase = mid_ubase_map.to_unit_base( Eout_max );
      use_next_Eout.linlin_interp( Eout_max_ubase, mid_lower_Eout, mid_upper_Eout );
    }
    else
    {
      use_next_Eout.set_E_out( mid_upper_Eout.get_E_out( ) );
      use_next_Eout.copy_coef( mid_upper_Eout );
    }
  }
}
// ------------------ standard_Legendre_param::direct_linlin_interpolate -----------
// Does direct lin-lin interpolation between two incident energies
void standard_Legendre_param::direct_linlin_interpolate( double Ein )
{
  double dEin = right_Ein - left_Ein;
  if( dEin <= 0.0 )
  {
    FatalError( "standard_Legendre_param::linlin_interpolate",
		"Incident energies out of order" );
  }
  double alpha = ( Ein - left_Ein )/dEin;
  // interpolate data at the lower and upper unit-base outgoing energies
  mid_lower_Eout.Ein_linlin_interp( Ein, left_Ein, Ein0_data.prev_data, right_Ein,
     Ein1_data.prev_data );
  mid_upper_Eout.Ein_linlin_interp( Ein, left_Ein, Ein0_data.next_data, right_Ein,
     Ein1_data.next_data );

  double Eout_min_linlin;
  double Eout_max_linlin;
  if( Eout_interp == HISTOGRAM )
  {
    // save the outgoing energies that are to be used
    if( use_Eout_min )
    {
      Eout_min_linlin = Eout_min;
      use_prev_Eout.set_E_out( Eout_min_linlin );
    }
    else
    {
      use_prev_Eout.set_E_out( mid_lower_Eout.get_E_out( ) );
    }
    use_prev_Eout.copy_coef( mid_lower_Eout );
    if( use_Eout_max )
    {
      Eout_max_linlin = Eout_max;
      use_next_Eout.set_E_out( Eout_max_linlin );
    }
    else
    {
      use_next_Eout.set_E_out( mid_upper_Eout.get_E_out( ) );
    }
    use_next_Eout.copy_coef( mid_lower_Eout );
  }
  else
  {
    // interpolate data to the outgoing energies that are to be used
    if( use_Eout_min )
    {
      Eout_min_linlin = Eout_min;
      use_prev_Eout.linlin_interp( Eout_min_linlin, mid_lower_Eout, mid_upper_Eout );
    }
    else
    {
      use_prev_Eout.set_E_out( mid_lower_Eout.get_E_out( ) );
      use_prev_Eout.copy_coef( mid_lower_Eout );
    }
    if( use_Eout_max )
    {
      Eout_max_linlin = Eout_max;
      use_next_Eout.linlin_interp( Eout_max_linlin, mid_lower_Eout, mid_upper_Eout );
    }
    else
    {
      use_next_Eout.set_E_out( mid_upper_Eout.get_E_out( ) );
      use_next_Eout.copy_coef( mid_upper_Eout );
    }
  }
  // interpolate the physical energy range
  Eout_range.x = ( 1.0 - alpha )*Eout_0_range.x + alpha*Eout_1_range.x;
  Eout_range.y = ( 1.0 - alpha )*Eout_0_range.y + alpha*Eout_1_range.y;
}
// ------------------ standard_Legendre_param::flat_interpolate ----------------
// Does histogram interpolation between two incident energies
void standard_Legendre_param::flat_interpolate( )
{
  // copy the data at the lower incident energy
  mid_lower_Eout.set_E_out( Ein0_data.prev_data.get_E_out( ) );
  mid_lower_Eout.copy_coef( Ein0_data.prev_data );
  mid_upper_Eout.set_E_out( Ein0_data.next_data.get_E_out( ) );
  mid_upper_Eout.copy_coef( Ein0_data.next_data );

  double Eout_min_flat;
  double Eout_max_flat;
  if( Eout_interp == HISTOGRAM )
  {
    // save the outgoing energies that are to be used
    if( use_Eout_min )
    {
      Eout_min_flat = Eout_min;
      use_prev_Eout.set_E_out( Eout_min_flat );
    }
    else
    {
      use_prev_Eout.set_E_out( mid_lower_Eout.get_E_out( ) );
    }
    use_prev_Eout.copy_coef( mid_lower_Eout );
    if( use_Eout_max )
    {
      Eout_max_flat = Eout_max;
      use_next_Eout.set_E_out( Eout_max_flat );
    }
    else
    {
      use_next_Eout.set_E_out( mid_upper_Eout.get_E_out( ) );
    }
    use_next_Eout.copy_coef( mid_lower_Eout );
  }
  else
  {
    // interpolate data to the outgoing energies that are to be used
    if( use_Eout_min )
    {
      Eout_min_flat = Eout_min;
      use_prev_Eout.linlin_interp( Eout_min_flat, mid_lower_Eout, mid_upper_Eout );
    }
    else
    {
      use_prev_Eout.set_E_out( mid_lower_Eout.get_E_out( ) );
      use_prev_Eout.copy_coef( mid_lower_Eout );
    }
    if( use_Eout_max )
    {
      Eout_max_flat = Eout_max;
      use_next_Eout.linlin_interp( Eout_max_flat, mid_lower_Eout, mid_upper_Eout );
    }
    else
    {
      use_next_Eout.set_E_out( mid_upper_Eout.get_E_out( ) );
      use_next_Eout.copy_coef( mid_upper_Eout );
    }
  }
  // the physical energy range
  Eout_range.x = Eout_0_range.x;
  Eout_range.y = Eout_0_range.y;
}

// ************* class standard_Legendre *****************
// ----------- standard_Legendre::standard_Legendre ------------------
// constructor
standard_Legendre::standard_Legendre( )
{
  Ein_interp.flag = NOTSET;  // LINLIN or HISTOGRAM
  Eout_interp = NOTSET;   // LINLIN or HISTOGRAM
}
// ----------- standard_Legendre::~standard_Legendre ------------------
// destructor
standard_Legendre::~standard_Legendre( )
{
}
// ----------- standard_Legendre::read_data ------------------
// Reads the Legendre data
void standard_Legendre::read_data( data_parser& infile, int num_Ein )
{
  order = Global.Value( "outputLegendreOrder" );
  standard_Legendre::iterator new_Ein_ptr;
  for( int Ein_count = 0; Ein_count < num_Ein; ++Ein_count )
  {
    // make a new standard_Legendre_vector
    new_Ein_ptr = insert( end( ), standard_Legendre_vector( ) );
    new_Ein_ptr->set_E_in( infile.get_next_double( ) );  // energy of incident particle
    new_Ein_ptr->Ein_interp = Ein_interp;
    new_Ein_ptr->Eout_interp = Eout_interp;
    int num_Eout = infile.get_next_int( );  // how many outgoing energies
    new_Ein_ptr->read_coef( infile, num_Eout, order );
  }
}
// -----------  standard_Legendre::get_Ein_range --------------
//  Gets the range of nontrivial incident energy bins; computes first_Ein and last_Ein
// returns true if the threshold is too high for the energy bins
bool standard_Legendre::get_Ein_range( const dd_vector& sigma_, const dd_vector& mult_,
    const dd_vector& weight_,
    const Flux_List& e_flux_, const Energy_groups& Ein_groups )
{
  double E_first;
  double E_last;
  standard_Legendre_param initial_param;
  bool done = initial_param.get_Ein_range( sigma_, mult_, weight_, e_flux_,
                                         Ein_groups, &E_first, &E_last );
  if( done ) return true;

  // check the range of incident energies for the probability data
  standard_Legendre::const_iterator this_ptr = begin( );
  double E_data = this_ptr->get_E_in( );
  if( E_data > E_first )
  {
    E_first = E_data;
  }
  first_Ein = Ein_groups.first_bin_ID( E_first );

  this_ptr = end( );
  --this_ptr;
  E_data = this_ptr->get_E_in( );
  if( E_data < E_last )
  {
    E_last = E_data;
  }
  last_Ein = Ein_groups.last_bin_ID( E_last );

  return false;
}
// ----------- standard_Legendre::get_T ------------------
// Computes the transfer matrix
void standard_Legendre::get_T( const dd_vector& sigma, const dd_vector& multiple, 
  const dd_vector& weight, T_matrix& transfer )
{
  bool interp_OK = ( ( Ein_interp.qualifier == UNITBASE ) &&
		     ( ( Ein_interp.flag == LINLIN ) ||
		       ( Ein_interp.flag == LINLOG ) ) ) ||
    ( ( Ein_interp.qualifier == CUMULATIVE_POINTS ) &&
      ( ( Ein_interp.flag == LINLIN ) ||
        ( Ein_interp.flag == HISTOGRAM ) ) ) ||
    ( ( Ein_interp.qualifier == DIRECT ) &&
      ( ( Ein_interp.flag == LINLIN ) ||
        ( Ein_interp.flag == HISTOGRAM ) ) );

  if( !interp_OK )
  {
    FatalError( "standard_Legendre::get_T", "Incident interpolation type not implemented" );
  }
  interp_OK = ( Eout_interp == LINLIN ) || ( Eout_interp == HISTOGRAM );
  if( !interp_OK )
  {
    FatalError( "standard_Legendre::get_T", "Outgoing interpolation type not implemented" );
  }
  if( Ein_interp.qualifier == DIRECT )
  {
    Warning( "standard_Legend::get_T",
      "direct interpolation may violate energy conservation" );
  }

  // Is there data inside the incident energy range?
  bool done = get_Ein_range( sigma, multiple, weight, transfer.e_flux,
    transfer.in_groups );
  if( done )
  {
    transfer.zero_transfer( );
  }

  long int quad_count = 0;  // number of 3-d quadratures
  long int Ein_F_count= 0;  // number of calls to standard_Legendre_F::Ein_F

  // now do the integrals incident bin by incident bin
#pragma omp parallel for schedule( dynamic, 1 ) default( none )	\
  shared( sigma, multiple, weight, transfer ) \
  reduction( +: quad_count ) reduction( +: Ein_F_count )
  for( int Ein_bin = first_Ein; Ein_bin < last_Ein; ++Ein_bin )
  {
    standard_Legendre_param Ein_param;
    Ein_param.Ein_interp = Ein_interp;
    Ein_param.Eout_interp = Eout_interp;
    Ein_param.initialize( transfer.order );

    // set up the data range for this bin
    Ein_param.setup_bin( Ein_bin, sigma, multiple, weight, transfer.e_flux,
                         transfer.in_groups );
    setup_data( &Ein_param );
    // work on this bin
    for( ; ; )
    {
      // get the incident energy interval common to all data
      set_Ein_range( Ein_bin, Ein_param );
      E_data_ladder( transfer, &Ein_param );  // loop over the outgoing energies
      // go to the next interval
      bool Done = next_ladder( Ein_param.data_E_1, &Ein_param );
      if( Done )
      {
        break;
      }
    }
    quad_count += Ein_param.quad_count;
    Ein_F_count += Ein_param.Ein_F_count;
  } // end of parallel loop

  // print the counts of function evaluations
  cout << "2d quadratures: " << quad_count << endl;
  cout << "standard_Legendre_F::Ein_F calls: " << Ein_F_count << endl;
  cout << "average standard_Legendre_F::Ein_F calls: " << 1.0*Ein_F_count/quad_count << endl;
}
// ----------- standard_Legendre::setup_data --------------
// Initializes the quadrature parameters
void standard_Legendre::setup_data( standard_Legendre_param *Ein_param )
{
  Ein_param->Ein0_data.Ein_interp = Ein_interp;
  Ein_param->Ein0_data.Eout_interp = Eout_interp;
  Ein_param->Ein1_data.Ein_interp = Ein_interp;
  Ein_param->Ein1_data.Eout_interp = Eout_interp;

  Ein_param->this_Ein = begin( );
  Ein_param->next_Ein = Ein_param->this_Ein;
  ++Ein_param->next_Ein;

  while( Ein_param->next_Ein->get_E_in( ) <= *Ein_param->Ein_ptr )
  {
    Ein_param->this_Ein = Ein_param->next_Ein;
    ++Ein_param->next_Ein;
  }

  double first_E = Ein_param->this_Ein->get_E_in( );
  if( first_E > Ein_param->data_E_0 )
  {
    Ein_param->data_E_0 = first_E;
    bool data_bad = Ein_param->update_pointers( first_E );
    if( data_bad )
    {
      FatalError( "standard_Legendre::setup_data", "incident energies out of order" );
    }
  }
}
// -----------  standard_Legendre::set_Ein_range ------------------
// Sets the range of incident energies for this intergration
void standard_Legendre::set_Ein_range( int Ein_bin, standard_Legendre_param &Ein_param )
{
  Ein_param.set_Ein_range( );
  double this_E = Ein_param.this_Ein->get_E_in( );
  if( this_E > Ein_param.data_E_0 ) Ein_param.data_E_0 = this_E;
  this_E = Ein_param.next_Ein->get_E_in( );
  if( this_E < Ein_param.data_E_1 ) Ein_param.data_E_1 = this_E;

  if( Ein_param.data_E_1 < Ein_param.data_E_0 )
  {
    FatalError( "standard_Legendre::set_Ein_range", "check the incident energies" );
  }
  Ein_param.set_sigma_range( );
}
// ----------- standard_Legendre::E_data_ladder --------------
// Loops through the energy data
 void standard_Legendre::E_data_ladder( T_matrix& transfer,
    standard_Legendre_param *Ein_param )
{
  Ein_param->reset_start( );
  // loop through the energy data
  for( ; ; )
  {
    // the physical (E, E') values for the data
    Ein_param->lower_hits.E_Eout.first.x = Ein_param->this_Ein->get_E_in( );
    Ein_param->lower_hits.E_Eout.first.y = Ein_param->Eout_0_range.x;  // physical lower E'
    Ein_param->lower_hits.E_Eout.second.x = Ein_param->next_Ein->get_E_in( );
    Ein_param->lower_hits.E_Eout.second.y = Ein_param->Eout_1_range.x;  // physical lower E'
    Ein_param->upper_hits.E_Eout.first.x = Ein_param->this_Ein->get_E_in( );     // Ein
    Ein_param->upper_hits.E_Eout.first.y = Ein_param->Eout_0_range.y;  // physical upper E'
    Ein_param->upper_hits.E_Eout.second.x = Ein_param->next_Ein->get_E_in( );    // Ein
    Ein_param->upper_hits.E_Eout.second.y = Ein_param->Eout_1_range.y;  // physical upper E'

    Eout_ladder( transfer, Ein_param );
    bool done = Ein_param->get_next_Eout( );
    if( done ) break;
  }
}
// ----------- standard_Legendre::Eout_ladder --------------
// Loops through the outgoing energy bins
void standard_Legendre::Eout_ladder( T_matrix& transfer,
   standard_Legendre_param *Ein_param )
{
  double dummy = 0.0;
  for( int Eout_count = 0; Eout_count < transfer.num_Eout_bins;
    ++Eout_count )
  {
    vector< double >::const_iterator Eout_ptr = transfer.out_groups.begin( )
      + Eout_count;
    // how does the lowest interpolation line meet this E-E' box?
    Ein_param->lower_hits.hit_box( dummy, Eout_ptr,
      Ein_param->data_E_0, Ein_param->data_E_1 );
    if( ( Eout_count < transfer.num_Eout_bins - 1 ) &&
        ( Ein_param->lower_hits.is_above( ) ) )
    {
      // go on to the next E-E' box
      continue;
    }
    // how does the next interpolation line meet this E-E' box?
    Ein_param->upper_hits.hit_box( dummy, Eout_ptr,
      Ein_param->data_E_0, Ein_param->data_E_1 );
    if( ( Eout_count > 0 ) && ( Ein_param->upper_hits.is_below( ) ) )
    {
      // we are done with this pair of E values
      break;
    }
    // integrate over this E-E' box
    one_Ebox( transfer, Eout_count, Ein_param );
  }
}
// ----------- standard_Legendre::one_Ebox --------------
// Integrate over one E-E' box
void standard_Legendre::one_Ebox( T_matrix& transfer, int Eout_count,
   standard_Legendre_param *Ein_param )
{
  // the E' energy range
  Ein_param->Eout_min = transfer.out_groups[ Eout_count ];
  Ein_param->Eout_max = transfer.out_groups[ Eout_count + 1 ];

  // set up common incident energies
  Ein_param->lower_hits.common_hits( Ein_param->upper_hits );

  // integrate depending on how the hyperbolas eta = const meet the box
  energy_hit_list::iterator low_hit_ptr = Ein_param->lower_hits.begin( );
  energy_hit_list::iterator next_low_ptr = low_hit_ptr;
  ++next_low_ptr;
  energy_hit_list::iterator high_hit_ptr = Ein_param->upper_hits.begin( );
  energy_hit_list::iterator next_high_ptr = high_hit_ptr;
  ++next_high_ptr;
  for( ; ( next_low_ptr != Ein_param->lower_hits.end( ) ) &&
         ( next_high_ptr != Ein_param->upper_hits.end( ) );
       low_hit_ptr = next_low_ptr, ++next_low_ptr,
         high_hit_ptr = next_high_ptr, ++next_high_ptr )
  {
    if( ( low_hit_ptr->hit_edge == ABOVE ) ||
        ( low_hit_ptr->hit_edge == TOP_OUT ) )
    {
      // do nothing---we are above the E-E' box
      continue;
    }
    else if( ( low_hit_ptr->hit_edge == TOP_IN ) ||
             ( low_hit_ptr->hit_edge == BOTTOM_IN ) ||
             ( low_hit_ptr->hit_edge == INSIDE ) )
    {
      // the lower eta = const hyperbola is inside the E-E' box
      Ein_param->use_Eout_min = false;
      // where is the upper hyperbola?
      if( ( high_hit_ptr->hit_edge == ABOVE ) ||
          ( high_hit_ptr->hit_edge == TOP_OUT ) )
      {
        // integrate up to the top of the E-E' bin
        Ein_param->use_Eout_max = true;
      }
      else
      {
        // integrate up to the next eta = const hyperbola
        Ein_param->use_Eout_max = false;
      }
    }
    else
    {
      // the lower eta = const hyperbola is below the E-E' box;
      // integrate from Eout_min
      Ein_param->use_Eout_min = true;
      // where is the upper eta = const hyperbola?
      if( ( high_hit_ptr->hit_edge == BOTTOM_OUT ) ||
          ( high_hit_ptr->hit_edge == BELOW ) )
      {
        // do nothing---we are below the E-E' box
        continue;
      }
      else if( ( high_hit_ptr->hit_edge == TOP_IN ) ||
               ( high_hit_ptr->hit_edge == BOTTOM_IN ) ||
               ( high_hit_ptr->hit_edge == INSIDE ) )
      {
        // the upper eta = const hyperbola is inside the E-E' box
        Ein_param->use_Eout_max = false;
      }
      else
      {
        // the upper eta = const hyperbola is above the E-E' box
        Ein_param->use_Eout_max = true;
      }
    }
    // the range of integration in incident energy
    Ein_param->Ein_0 = low_hit_ptr->E_in;
    Ein_param->Ein_1 = next_low_ptr->E_in;
    update_T( transfer, Eout_count, Ein_param );
  }
}
// ----------- standard_Legendre::next_ladder --------------
bool standard_Legendre::next_ladder( double E_in, standard_Legendre_param *Ein_param )
{
  bool done = Ein_param->update_bin_pointers( E_in );
  if( !done )
  {
    static double etol = Global.Value( "E_tol" );
    double E_tol = E_in * etol;
    //    double E_tol = 0.0;
    if( E_in + E_tol >= Ein_param->next_Ein->get_E_in( ) )
    {
      while( E_in + E_tol >= Ein_param->next_Ein->get_E_in( ) )
      {
        // get the next energy data
        Ein_param->this_Ein = Ein_param->next_Ein;
        ++Ein_param->next_Ein;
        if( Ein_param->next_Ein == end ( ) )
        {
          return true;
        }
      }
    }
  }

  return done;
}
// ----------- standard_Legendre::update_T --------------
// Increments the transfer matrix
void standard_Legendre::update_T( T_matrix &transfer, int Eout_count,
   standard_Legendre_param *Ein_param )
{
  static double tol = Global.Value( "quad_tol" );
  // differences of nearly-equal numbers can cause problems; when to skip an interval
  static double from_quad_tol = Global.Value( "abs_quad_tol" )/100;
  static double from_abs_tol = 1000*Global.Value( "abs_tol" );
  double skip_tol = ( from_quad_tol > from_abs_tol ) ? from_quad_tol : from_abs_tol;
  // a vector to store the integrals, one Legendre order
  coef_vector value( transfer.order, transfer.conserve );
  value.set_zero( );
  // parameters for the integration
  QuadParamBase *params = static_cast< QuadParamBase* >( Ein_param );

  double Ein_0 = Ein_param->Ein_0;
  double Ein_1 = Ein_param->Ein_1;
  // loop over the cross section data
  Ein_param->this_sigma = Ein_param->first_ladder_sigma;
  Ein_param->next_sigma = Ein_param->this_sigma;
  ++Ein_param->next_sigma;
  // Ein_0 may be past Ein_param->next_sigma
  while( ( Ein_param->this_sigma != Ein_param->last_ladder_sigma ) &&
         ( Ein_param->next_sigma->x < Ein_0 ) )
  {
    Ein_param->this_sigma = Ein_param->next_sigma;
    ++Ein_param->next_sigma;
  }
  for( ; ( Ein_param->this_sigma != Ein_param->last_ladder_sigma ) &&
         ( Ein_param->this_sigma->x <  Ein_1 );
       Ein_param->this_sigma = Ein_param->next_sigma, ++Ein_param->next_sigma )
  {
    Ein_param->Ein_0 = ( Ein_param->this_sigma->x < Ein_0 ) ? Ein_0 :
      Ein_param->this_sigma->x;
    Ein_param->Ein_1 = ( Ein_param->next_sigma->x > Ein_1 ) ? Ein_1 :
      Ein_param->next_sigma->x;
    // differences of nearly-equal numbers can cause problems
    if( ( Ein_param->Ein_1 - Ein_param->Ein_0 <= 
	  Ein_param->Ein_1 * skip_tol ) ||
	( Ein_param->Ein0_data.next_data.get_E_out( ) - Ein_param->Ein0_data.prev_data.get_E_out( ) <= 
	  Ein_param->Ein0_data.next_data.get_E_out( ) * skip_tol ) )
    {
      continue;  // skip this interval
    }
    else
    {
      quad_F::integrate( standard_Legendre_F::Ein_F, transfer.Ein_quad_method,
                         Ein_param->Ein_0,
			 Ein_param->Ein_1, params, tol, &value );
    }
    // add this integral
    transfer( Ein_param->Ein_count, Eout_count ) += value;
    // increment the function counts
    Ein_param->Ein_F_count += Ein_param->func_count;
    ++Ein_param->quad_count;
  }
}
// ----------- standard_Legendre::print --------------
void standard_Legendre::print( )
{
  for( standard_Legendre::iterator energy_ptr = begin( );
       energy_ptr != end( ); ++energy_ptr )
  {
    energy_ptr->print( );
  }
}

// *************** standard_Legendre_F::Ein_F **********************************
// Gets the energy probability density at given incident and outgpoing energies
void standard_Legendre_F::Ein_F( double E_in, QuadParamBase *Ein_param,
   coef_vector *value )
{
  // the parameters are really standard_Legendre_param *
  standard_Legendre_param *e_params = static_cast<standard_Legendre_param *>( Ein_param );
  e_params->func_count += 1;
  double scale = 1;  // jacobian for the unit-base map
  if( ( e_params->Ein_interp.qualifier == UNITBASE ) ||
      ( e_params->Ein_interp.qualifier == CUMULATIVE_POINTS ) )
  {
    e_params->unitbase_interpolate( E_in );
    scale = e_params->mid_ubase_map.Eout_max - e_params->mid_ubase_map.Eout_min;
  }
  else if( e_params->Ein_interp.flag == HISTOGRAM )
  {
    e_params->flat_interpolate( );
  }
  else  // direct lin-lin
  {
    e_params->direct_linlin_interpolate( E_in );
  }
  int use_order = e_params->use_prev_Eout.order;
  double Eout_high = e_params->use_next_Eout.get_E_out( );
  double Eout_low = e_params->use_prev_Eout.get_E_out( );
  double d_Eout = Eout_high - Eout_low;  // unit-base outgoing energy difference

  // omit really small intervals
  static double etol = Global.Value( "E_tol" );
  if( abs( d_Eout ) < etol*Eout_high )
  {
    value->set_zero( );  // return zero
    return;
  }
  // test for bad input
  if( ( d_Eout < 0.0 ) || ( scale <= 0.0 ) )
  {
    Warning( "standard_Legendre_F::Ein_F", "energies out of order" );
    if( ( value->conserve == NUMBER ) || ( value->conserve == BOTH ) )
    {
      for( int L_count = 0; L_count <= use_order; ++L_count )
      {
        value->weight_1[ L_count ] = 0.0;
      }
    }
    if( ( value->conserve == ENERGY ) || ( value->conserve == BOTH ) )
    {
      for( int L_count = 0; L_count <= use_order; ++L_count )
      {
        value->weight_E[ L_count ] = 0.0;
      }
    }
    return;
  }
  // input OK
  if( ( value->conserve == NUMBER ) || ( value->conserve == BOTH ) )
  {
    if( e_params->Eout_interp == HISTOGRAM )
    {
      for( int L_count = 0; L_count <= use_order; ++L_count )
      {
        value->weight_1[ L_count ] = d_Eout*e_params->mid_lower_Eout.data[ L_count ];
      }
    }
    else  // LINLIN
    {
      for( int L_count = 0; L_count <= use_order; ++L_count )
      {
        value->weight_1[ L_count ] = 0.5*d_Eout*
	  ( e_params->use_prev_Eout.data[ L_count ] +
	    e_params->use_next_Eout.data[ L_count ] );
      }
    }
  }
  if( ( value->conserve == ENERGY ) || ( value->conserve == BOTH ) )
  {
    double av_Eout = 0.5*( Eout_high + Eout_low );  // (unit-base)
    if( ( e_params->Ein_interp.qualifier == UNITBASE ) ||
	( e_params->Ein_interp.qualifier == CUMULATIVE_POINTS ) )
    {
      av_Eout = e_params->mid_ubase_map.un_unit_base( av_Eout );  // physical
    }
    if( e_params->Eout_interp == HISTOGRAM )
    {
      double integral = d_Eout*av_Eout;
      for( int L_count = 0; L_count <= use_order; ++L_count )
      {
        value->weight_E[ L_count ] = integral*e_params->mid_lower_Eout.data[ L_count ];
      }
    }
    else  // LINLIN
    {
      for( int L_count = 0; L_count <= use_order; ++L_count )
      {
        double av_Prob = 0.5*( e_params->use_prev_Eout.data[ L_count ] +
            e_params->use_next_Eout.data[ L_count ] );
        double Prob_slope = ( e_params->use_next_Eout.data[ L_count ] -
          e_params->use_prev_Eout.data[ L_count ] )/d_Eout;
        value->weight_E[ L_count ] = d_Eout*( av_Prob*av_Eout +
          scale*Prob_slope*d_Eout*d_Eout/12.0 );
      }
    }
  }
  // weight it by flux * cross section * multiplicity * model weight
  e_params->set_weight( E_in );
  *value *= e_params->current_weight;
}
