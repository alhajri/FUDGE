/*
 * ******** merced: calculate the transfer matrix *********
 * $Revision: 1 $
 * $Date: 2009-09-15 (Tue, 15 Sep 2009) $
 * $Author: hedstrom $
 * $Id: kalbach.cpp 1 2009-09-15 hedstrom $
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
// implementation of the classes used to handle kalbach energy-angle model

#include <cmath>
#include <cfloat>
#ifdef _OPENMP
 #include <omp.h>
#endif

#include "kalbach.hpp"
#include "math_util.hpp"
#include "messaging.hpp"
#include "global_params.hpp"

// ************* class kalbach_Ein_param *****************
// ----------- kalbach_Ein_param::set_Ecm_param --------------
// Sets up the parameters for integration over center-of-mass outgoing energy
void kalbach_Ein_param::set_Ecm_param( double E_in )
{
  Ecm_params.map = map;
  Ecm_params.setup( E_in, Ein0_data, Ein1_data, Eout_min, Eout_max, kalbach_a );
  Ecm_params.mu_quad_method = mu_quad_method;
}
// ----------- kalbach_Ein_param::start_Eout_cm --------------
// Initializes the pointers to the angular probabilities for this E_in
void kalbach_Ein_param::start_Eout_cm( )
{
  Ein0_data.E_in = this_Ein->get_E_in( );
  Ein1_data.E_in = next_Ein->get_E_in( );

  // initialize the pointers
  if( Ein0_data.Ein_interp.qualifier == DIRECT )
  {
    setup_Ein_linlin( );
  }
  else
  {
    setup_Ein_ubase( );
  }

  // for the range of Eout_cm values
  double lower_Eout;
  double higher_Eout;

  lower_Eout = ( left_ptr->x > right_ptr->x )?
      left_ptr->x : right_ptr->x;
  higher_Eout = ( next_left_ptr->x < next_right_ptr->x )?
      next_left_ptr->x : next_right_ptr->x;
  if( higher_Eout <= lower_Eout )
  {
    FatalError( "kalbach_Ein_param::start_Eout_cm", "Check the Eout values." );
  }

  // Interpolate to the common Eout_cm values
  common_low_Eoutcm( lower_Eout );
  common_high_Eoutcm( higher_Eout );
}
// ---------------- kalbach_Ein_param::setup_Ein_ubase ------------------
// Sets up the data for unit-base interpolation in incident energy
void kalbach_Ein_param::setup_Ein_ubase( )
{
  // lower incident energy
  left_ptr = this_Ein->begin( );
  next_left_ptr = left_ptr;
  ++next_left_ptr;
  last_left_ptr = this_Ein->end( );
  left_r_ptr = this_Ein->Eout_r.begin( );
  next_left_r_ptr = left_r_ptr;
  ++next_left_r_ptr;
  last_left_r_ptr = this_Ein->Eout_r.end( );

  // higher incident energy
  right_ptr = next_Ein->begin( );
  next_right_ptr = right_ptr;
  ++next_right_ptr;
  last_right_ptr = next_Ein->end( );
  right_r_ptr = next_Ein->Eout_r.begin( );
  next_right_r_ptr = right_r_ptr;
  ++next_right_r_ptr;
  last_right_r_ptr = next_Ein->Eout_r.end( );

  // save the unit-base interpolation
  Ein0_data.unit_base.copy( this_Ein->unit_base );
  Ein1_data.unit_base.copy( next_Ein->unit_base );
}
// ---------------- kalbach_Ein_param::setup_Ein_linlin ------------------
// Sets up the data for linlin interpolation in incident energy
void kalbach_Ein_param::setup_Ein_linlin( )
{
  // remove previous data
  if( !low_linlin.empty( ) )
  {
    low_linlin.erase( low_linlin.begin( ), low_linlin.end( ) );
    high_linlin.erase( high_linlin.begin( ), high_linlin.end( ) );
    low_linlin.Eout_r.erase( low_linlin.Eout_r.begin( ),
       low_linlin.Eout_r.end( ) );
    high_linlin.Eout_r.erase( high_linlin.Eout_r.begin( ),
       high_linlin.Eout_r.end( ) );
  }

  // get the outgoing energy range
  left_ptr = this_Ein->begin( );
  right_ptr = next_Ein->begin( );
  double Eout_min = ( left_ptr->x < right_ptr->x ) ? left_ptr->x : right_ptr->x;

  left_ptr = this_Ein->end( );
  --left_ptr;
  right_ptr = next_Ein->end( );
  --right_ptr;
  double Eout_max = ( left_ptr->x > right_ptr->x ) ? left_ptr->x : right_ptr->x;

  // extrapolate as zero
  low_linlin.extrapolate_copy( *this_Ein, Eout_min, Eout_max );
  low_linlin.Eout_r.extrapolate_copy( this_Ein->Eout_r, Eout_min, Eout_max );
  high_linlin.extrapolate_copy( *next_Ein, Eout_min, Eout_max );
  high_linlin.Eout_r.extrapolate_copy( next_Ein->Eout_r, Eout_min, Eout_max );

  // set pointers at lower incident energy
  left_ptr = low_linlin.begin( );
  next_left_ptr = left_ptr;
  ++next_left_ptr;
  last_left_ptr = low_linlin.end( );
  left_r_ptr = low_linlin.Eout_r.begin( );
  next_left_r_ptr = left_r_ptr;
  ++next_left_r_ptr;
  last_left_r_ptr = low_linlin.Eout_r.end( );

  // higher incident energy
  right_ptr = high_linlin.begin( );
  next_right_ptr = right_ptr;
  ++next_right_ptr;
  last_right_ptr = high_linlin.end( );
  right_r_ptr = high_linlin.Eout_r.begin( );
  next_right_r_ptr = right_r_ptr;
  ++next_right_r_ptr;
  last_right_r_ptr = high_linlin.Eout_r.end( );
}
// ---------------- kalbach_Ein_param::next_Eoutcm ------------------
// Sets up the next interval of Eout_cm values
bool kalbach_Ein_param::next_Eoutcm( )
{
  bool done = false;

  Ein0_data.this_Ecm = Ein0_data.next_Ecm;
  Ein0_data.this_f0 = Ein0_data.next_f0;
  Ein0_data.this_r = Ein0_data.next_r;
  Ein1_data.this_Ecm = Ein1_data.next_Ecm;
  Ein1_data.this_f0 = Ein1_data.next_f0;
  Ein1_data.this_r = Ein1_data.next_r;

  // update the pointers
  if( next_left_ptr->x <= Ein0_data.this_Ecm )
  {
    left_ptr = next_left_ptr;
    ++next_left_ptr;
    if( next_left_ptr == last_left_ptr )
    {
      return true;
    }
  }
  if( next_left_r_ptr->x <= Ein0_data.this_Ecm )
  {
    left_r_ptr = next_left_r_ptr;
    ++next_left_r_ptr;
    if( next_left_r_ptr == last_left_r_ptr )
    {
      return true;
    }
  }
  if( next_right_ptr->x <= Ein1_data.this_Ecm )
  {
    right_ptr = next_right_ptr;
    ++next_right_ptr;
    if( next_right_ptr == last_right_ptr )
    {
      return true;
    }
  }
  if( next_right_r_ptr->x <= Ein1_data.this_Ecm )
  {
    right_r_ptr = next_right_r_ptr;
    ++next_right_r_ptr;
    if( next_right_r_ptr == last_right_r_ptr )
    {
      return true;
    }
  }

  // set the common upper Eout_cm value
  double upper_Eout = ( next_left_ptr->x < next_right_ptr->x ) ?
    next_left_ptr->x : next_right_ptr->x;
  double upper_r_Eout = ( next_left_r_ptr->x < next_right_r_ptr->x ) ?
    next_left_r_ptr->x : next_right_r_ptr->x;
  if( upper_r_Eout < upper_Eout )
  {
    upper_Eout = upper_r_Eout;
  }
  common_high_Eoutcm( upper_Eout );
  return done;
}
// ---------------- kalbach_Ein_param::common_low_Eoutcm ------------------
// Interpolates (Eout_cm, probability) data to the lower common Eout_cm value
void kalbach_Ein_param::common_low_Eoutcm( double lower_Eout )
{
  Ein0_data.this_Ecm = lower_Eout;
  Ein1_data.this_Ecm = lower_Eout;

  if( ( left_ptr->x == lower_Eout ) || ( Ein0_data.Eout_interp == HISTOGRAM ) )
  {
    Ein0_data.this_f0 = left_ptr->y;
    Ein0_data.this_r = left_r_ptr->y;
  }
  else
  {
    Ein0_data.this_f0 = left_ptr->linlin_interp( lower_Eout, *next_left_ptr );
    Ein0_data.this_r = left_r_ptr->linlin_interp( lower_Eout, *next_left_r_ptr );
  }

  if( ( right_ptr->x == lower_Eout ) || ( Ein1_data.Eout_interp == HISTOGRAM ) )
  {
    Ein1_data.this_f0 = right_ptr->y;
    Ein1_data.this_r = right_r_ptr->y;
  }
  else
  {
    Ein1_data.this_f0 = right_ptr->linlin_interp( lower_Eout, *next_right_ptr );
    Ein1_data.this_r = right_r_ptr->linlin_interp( lower_Eout, *next_right_r_ptr );
  }
}
// ---------------- kalbach_Ein_param::common_high_Eoutcm ------------------
// Interpolates (Eout_cm, probability) data to the higher common Eout_cm value
void kalbach_Ein_param::common_high_Eoutcm( double higher_Eout )
{
  Ein0_data.next_Ecm = higher_Eout;
  Ein1_data.next_Ecm = higher_Eout;

  if( next_left_ptr->x == higher_Eout )
  {
    Ein0_data.next_f0 = next_left_ptr->y;
    Ein0_data.next_r = next_left_r_ptr->y;
  }
  else if( Ein0_data.Eout_interp == HISTOGRAM )
  {
    Ein0_data.next_f0 = left_ptr->y;
    Ein0_data.next_r = left_r_ptr->y;
  }
  else
  {
    Ein0_data.next_f0 = left_ptr->linlin_interp( higher_Eout, *next_left_ptr );
    Ein0_data.next_r = left_r_ptr->linlin_interp( higher_Eout, *next_left_r_ptr );
  }

  if( next_right_ptr->x == higher_Eout )
  {
    Ein1_data.next_f0 = next_right_ptr->y;
    Ein1_data.next_r = next_right_r_ptr->y;
  }
  else if( Ein1_data.Eout_interp == HISTOGRAM )
  {
    Ein1_data.next_f0 = right_ptr->y;
    Ein1_data.next_r = right_r_ptr->y;
  }
  else
  {
    Ein1_data.next_f0 = right_ptr->linlin_interp( higher_Eout, *next_right_ptr );
    Ein1_data.next_r = right_r_ptr->linlin_interp( higher_Eout, *next_right_r_ptr );
  }
}

// ************* class kalbach_Ecm_param *****************
// ----------- kalbach_Ecm_param::setup --------------
// Sets up the data for this incident energy
void kalbach_Ecm_param::setup( double E_in, const kalbach_data &Ein0_data,
  const kalbach_data &Ein1_data, double Eoutmin, double Eoutmax, Kalbach_a *kalbach_A )
{
  kalbach_a = kalbach_A;
  if( Ein0_data.Ein_interp.qualifier == UNITBASE )
  {
    Ein_data.unit_base_interp( E_in, Ein0_data, Ein1_data );
    // undo the unit-base map; from here on we work with interpolated physical data
    Ein_data.un_unit_base( );
  }
  else
  {
    Ein_data.linlin_interp( E_in, Ein0_data, Ein1_data );
  }
  Ecm_Elab_Ecm_param::setup( E_in, Eoutmin, Eoutmax, Ein_data.this_Ecm, Ein_data.next_Ecm );
  V_lab_sectors( );
}

// ************* class kalbach_mu_param **********************
// ----------- kalbach_mu_param::setup --------------
// Sets up the data for this incident energy
void kalbach_mu_param::setup( double Ein, double Eoutcm, const kalbach_Ecm_param& Ecm_param )
{
  Ecm_Elab_mu_param::setup( Ein, Eoutcm, Ecm_param );
  a = Ecm_param.kalbach_a->get_a( Ein, Eoutcm );
  gamma_in = ( Ecm_param.kalbach_a->projectile.A == 0 );
  Ecm_param.Ein_data.get_f0_r( Eoutcm, &Ecm_prob, &r );
}

// ************ class Kalbach_one_Ein **********************
// ----------- Kalbach_one_Ein::read_probability ------------------
// Reads the Kalbach energy probability density for one incident energy
void Kalbach_one_Ein::read_probability( data_parser &input_file,
  int num_Eout_cm )
{
  dd_entry next_Eout_cm;

  for( int Eout_cm_count = 0; Eout_cm_count < num_Eout_cm; ++Eout_cm_count )
  {
    next_Eout_cm.x = input_file.get_next_double( );  // Eout_cm
    next_Eout_cm.y = input_file.get_next_double( );  // probability density
    insert( end( ), next_Eout_cm );
  }
}
// ----------- Kalbach_one_Ein::read_r ------------------
// Reads the Kalbach r parameters for one incident energy
void Kalbach_one_Ein::read_r( data_parser &input_file,
  int num_Eout_cm )
{
  Eout_r.set_E_in( get_E_in( ) );
  dd_entry next_Eout_cm;

  for( int Eout_cm_count = 0; Eout_cm_count < num_Eout_cm; ++Eout_cm_count )
  {
    next_Eout_cm.x = input_file.get_next_double( );  // Eout_cm
    next_Eout_cm.y = input_file.get_next_double( );  // r parameter
    Eout_r.insert( Eout_r.end( ), next_Eout_cm );
  }
}
// -----------  Kalbach_one_Ein::chop_histogram ------------------
// Truncates histogram data at the maximum energy
void Kalbach_one_Ein::chop_histogram( double EMax )
{
  dd_vector::chop_histogram( EMax );
  renorm( );
  Eout_r.chop_histogram( EMax );
}

// ************ class Kalbach **********************
// ----------- Kalbach constructor --------------
Kalbach::Kalbach( )
{
  kalbach_a.map = &map;
}
// -----------  Kalbach::get_Ein_range --------------
//  Gets the range of nontrivial incident energy bins; computes first_Ein and last_Ein
// returns true if the threshold is too high for the energy bins
bool  Kalbach::get_Ein_range( const dd_vector& sigma_, const dd_vector& mult_,
    const dd_vector& weight_,
    const Flux_List& e_flux_, const Energy_groups& Ein_groups )
{
  double E_first;
  double E_last;
  kalbach_Ein_param initial_param;
  bool done = initial_param.get_Ein_range( sigma_, mult_, weight_, e_flux_,
                                         Ein_groups, &E_first, &E_last );
  if( done ) return true;

  // check the range of incident energies for the probability data
  Kalbach::const_iterator this_ptr = begin( );
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
// -----------  Kalbach::cm_Eout_ladder ------------------
// Adds to the transfer matrix for all E_out bins for a pair of incident energies.
void Kalbach::cm_Eout_ladder( T_matrix& transfer, kalbach_Ein_param *Ein_param )
{
  Ein_param->start_Eout_cm( );
  bool done = ( Ein_param->this_Ein->size( ) <= 1 );
  // loop over the Kalbach data for Ein_param->data_E_0 < E_in < Ein_param->data_E_1
  while( !done )
  {
    if( Ein_interp.qualifier == UNITBASE )
    {
      double Ecm = Ein_param->Ein0_data.un_unit_base( Ein_param->Ein0_data.this_Ecm );
      Ein_param->lower_hits.G0_data.set_energies( Ein_param->Ein0_data.E_in, Ecm );
      Ecm = Ein_param->Ein1_data.un_unit_base( Ein_param->Ein1_data.this_Ecm );
      Ein_param->lower_hits.G1_data.set_energies( Ein_param->Ein1_data.E_in, Ecm );
      Ecm = Ein_param->Ein0_data.un_unit_base( Ein_param->Ein0_data.next_Ecm );
      Ein_param->upper_hits.G0_data.set_energies( Ein_param->Ein0_data.E_in, Ecm );
      Ecm = Ein_param->Ein1_data.un_unit_base( Ein_param->Ein1_data.next_Ecm );
      Ein_param->upper_hits.G1_data.set_energies( Ein_param->Ein1_data.E_in, Ecm );
    }
    else
    {
      Ein_param->lower_hits.G0_data.set_energies( Ein_param->Ein0_data.E_in,
         Ein_param->Ein0_data.this_Ecm );
      Ein_param->lower_hits.G1_data.set_energies( Ein_param->Ein1_data.E_in,
          Ein_param->Ein1_data.this_Ecm );
      Ein_param->upper_hits.G0_data.set_energies( Ein_param->Ein0_data.E_in,
          Ein_param->Ein0_data.next_Ecm );
      Ein_param->upper_hits.G1_data.set_energies( Ein_param->Ein1_data.E_in,
         Ein_param->Ein1_data.next_Ecm );
    }

    lab_Eout_ladder( transfer, *Ein_param );
    done = Ein_param->next_Eoutcm( );
  }
}
// -----------  Kalbach::lab_Eout_ladder ------------------
// Loops over the outgoing lab energy bins for one pair of outgoing cm energies
void Kalbach::lab_Eout_ladder( T_matrix& transfer, kalbach_Ein_param &Ein_param )
{
  //  bool check_geometry = true;
  bool check_geometry = false;
  bool geom_OK;  // for checking the consistency of the geometry
  bool upper_hits_set = false;
  bool lower_hits_set = false;
  Vcm_Vlab_hit_list test_hits;
  test_hits.G0_data.gamma = map.gamma;
  test_hits.G1_data.gamma = map.gamma;
  double dummy = 0.0;
  double Ecm;
  int Eout_count = 0;
  vector< double >::const_iterator Eout_ptr = transfer.out_groups.begin( );
  vector< double >::const_iterator next_Eout = Eout_ptr;
  ++next_Eout;
  // Check for only forward emission
  if( Ein_param.upper_hits.G1_data.E_cm < Ein_param.upper_hits.G1_data.get_Etrans( ) )
  {
    geom_OK = Ein_param.upper_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0,
       Ein_param.data_E_1 );
    upper_hits_set = true;
    if( check_geometry )
    {
      cout << "Forward with next_Eout: " << *next_Eout << endl;
      Ein_param.upper_hits.print( );
    }
    if( !geom_OK )
    {
      Ecm = Ein_param.Ein0_data.un_unit_base( Ein_param.Ein0_data.next_Ecm );
      test_hits.G0_data.set_energies( Ein_param.Ein0_data.E_in, Ecm );
      Ecm = Ein_param.Ein1_data.un_unit_base( Ein_param.Ein1_data.next_Ecm );
      test_hits.G1_data.set_energies( Ein_param.Ein1_data.E_in, Ecm );
      geom_OK = test_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0, Ein_param.data_E_1 );
      test_hits.print( );
      FatalError( "Kalbach::lab_Eout_ladder", "Check the coding, 1" );
    }
    while( Ein_param.upper_hits.is_above( ) )
    {
      Eout_ptr = next_Eout;
      ++next_Eout;
      if( next_Eout == transfer.out_groups.end( ) )
      {
	return;
      }
      ++Eout_count;
      geom_OK = Ein_param.upper_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0,
         Ein_param.data_E_1 );
      if( check_geometry )
      {
        cout << "next Forward with next_Eout: " << *next_Eout << endl;
        Ein_param.upper_hits.print( );
      }
      if( !geom_OK )
      {
        Ecm = Ein_param.Ein0_data.un_unit_base( Ein_param.Ein0_data.next_Ecm );
        test_hits.G0_data.set_energies( Ein_param.Ein0_data.E_in, Ecm );
        Ecm = Ein_param.Ein1_data.un_unit_base( Ein_param.Ein1_data.next_Ecm );
        test_hits.G1_data.set_energies( Ein_param.Ein1_data.E_in, Ecm );
        geom_OK = test_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0, Ein_param.data_E_1 );
        test_hits.print( );
        FatalError( "Kalbach::lab_Eout_ladder", "Check the coding, 2" );
      }
    }
  }
  else if( Ein_param.lower_hits.G1_data.E_cm > Ein_param.lower_hits.G1_data.get_Etrans( ) )
  {
    // Check whether all emission is above the lab energy bin
    geom_OK = Ein_param.lower_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0,
       Ein_param.data_E_1 );
    lower_hits_set = true;
    if( check_geometry )
    {
      cout << "Backward with E_out: " << *Eout_ptr << endl;
      Ein_param.lower_hits.print( );
    }
    if( !geom_OK )
    {
      Ecm = Ein_param.Ein0_data.un_unit_base( Ein_param.Ein0_data.this_Ecm );
      test_hits.G0_data.set_energies( Ein_param.Ein0_data.E_in, Ecm );
      Ecm = Ein_param.Ein1_data.un_unit_base( Ein_param.Ein1_data.this_Ecm );
      test_hits.G1_data.set_energies( Ein_param.Ein1_data.E_in, Ecm );
      geom_OK = test_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0, Ein_param.data_E_1 );
      test_hits.print( );
      FatalError( "Kalbach::lab_Eout_ladder", "Check the coding, 3" );
    }
    while( Ein_param.lower_hits.is_below( ) )
    {
      Eout_ptr = next_Eout;
      ++next_Eout;
      if( next_Eout == transfer.out_groups.end( ) )
      {
	return;
      }
      ++Eout_count;
      geom_OK = Ein_param.lower_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0,
         Ein_param.data_E_1 );
      if( check_geometry )
      {
        cout << "backward with E_out: " << *Eout_ptr << endl;
        Ein_param.lower_hits.print( );
      }
      if( !geom_OK )
      {
        Ecm = Ein_param.Ein0_data.un_unit_base( Ein_param.Ein0_data.this_Ecm );
        test_hits.G0_data.set_energies( Ein_param.Ein0_data.E_in, Ecm );
        Ecm = Ein_param.Ein1_data.un_unit_base( Ein_param.Ein1_data.this_Ecm );
        test_hits.G1_data.set_energies( Ein_param.Ein1_data.E_in, Ecm );
        geom_OK = test_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0, Ein_param.data_E_1 );
        test_hits.print( );
        FatalError( "Kalbach::lab_Eout_ladder", "Check the coding, 4" );
      }
    }
  }
  // Now, compute integrals until the lab energy bin is above the E_cm data
  for( ; Eout_count < transfer.num_Eout_bins;
       ++Eout_count, Eout_ptr = next_Eout, ++next_Eout )
  {
    if( !upper_hits_set )
    {
      geom_OK = Ein_param.upper_hits.hit_box( dummy, Eout_ptr,
         Ein_param.data_E_0, Ein_param.data_E_1 );
      if( check_geometry )
      {
        cout << "Ein_param.upper_hits for Eout: " << *Eout_ptr << endl;
        Ein_param.upper_hits.print( );
      }
      if( !geom_OK )
      {
        Ecm = Ein_param.Ein0_data.un_unit_base( Ein_param.Ein0_data.next_Ecm );
        test_hits.G0_data.set_energies( Ein_param.Ein0_data.E_in, Ecm );
        Ecm = Ein_param.Ein1_data.un_unit_base( Ein_param.Ein1_data.next_Ecm );
        test_hits.G1_data.set_energies( Ein_param.Ein1_data.E_in, Ecm );
        geom_OK = test_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0, Ein_param.data_E_1 );
        test_hits.print( );
        FatalError( "Kalbach::lab_Eout_ladder", "Check the coding, 5" );
      }
    }
    if( Ein_param.upper_hits.is_below( ) )
    {
      break;  // we are done
    }
    if( !lower_hits_set )
    {
      geom_OK = Ein_param.lower_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0,
        Ein_param.data_E_1 );
      if( check_geometry )
      {
        cout << "Ein_param.lower_hits for Eout: " << *Eout_ptr << endl;
        Ein_param.lower_hits.print( );
      }
      if( !geom_OK )
      {
        Ecm = Ein_param.Ein0_data.un_unit_base( Ein_param.Ein0_data.this_Ecm );
        test_hits.G0_data.set_energies( Ein_param.Ein0_data.E_in, Ecm );
        Ecm = Ein_param.Ein1_data.un_unit_base( Ein_param.Ein1_data.this_Ecm );
        test_hits.G1_data.set_energies( Ein_param.Ein1_data.E_in, Ecm );
        geom_OK = test_hits.hit_box( dummy, Eout_ptr, Ein_param.data_E_0, Ein_param.data_E_1 );
        test_hits.print( );
        FatalError( "Kalbach::lab_Eout_ladder", "Check the coding, 6" );
      }
    }
    // integrate over this E-E' box
    one_Ebox( transfer, Eout_count, Ein_param );
    upper_hits_set = false;
    lower_hits_set = false;
  }
}
// ----------- Kalbach::setup_data --------------
// Initializes the quadrature parameters
void Kalbach::setup_data( kalbach_Ein_param *Ein_param )
{
  // set up the mapping to the lab frame
  kalbach_a.setup_params( );
  Ein_param->kalbach_a = &kalbach_a;

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
  }

  // the Vcm_Vlab_hit_list objects need the gamma for the energy of translation of the center of mass
  Ein_param->lower_hits.G0_data.gamma = map.gamma;
  Ein_param->lower_hits.G1_data.gamma = map.gamma;
  Ein_param->upper_hits.G0_data.gamma = map.gamma;
  Ein_param->upper_hits.G1_data.gamma = map.gamma;
}
// -----------  Kalbach::set_Ein_range ------------------
// Sets the range of incident energies for this intergration
void Kalbach::set_Ein_range( int Ein_bin, kalbach_Ein_param &Ein_param )
{
  Ein_param.set_Ein_range( );
  double this_E = Ein_param.this_Ein->get_E_in( );
  if( this_E > Ein_param.data_E_0 ) Ein_param.data_E_0 = this_E;
  this_E = Ein_param.next_Ein->get_E_in( );
  if( this_E < Ein_param.data_E_1 ) Ein_param.data_E_1 = this_E;

  if( Ein_param.data_E_1 < Ein_param.data_E_0 )
  {
    FatalError( "Kalbach::set_Ein_range", "check the Kalbach incident energies" );
  }
  Ein_param.set_sigma_range( );
}
// -----------  Kalbach::one_Ebox ------------------
// Does the integration for one Eout_lab annulus between a pair of incident energies
void Kalbach::one_Ebox( T_matrix& transfer, int Eout_count,
  kalbach_Ein_param &Ein_param )
{
  // the E' energy range
  Ein_param.Eout_min = transfer.out_groups[ Eout_count ];
  Ein_param.Eout_max = transfer.out_groups[ Eout_count + 1 ];
  //  cout << Ein_param.Eout_min << " < E_out < " << Ein_param.Eout_max << endl;
  // set up common incident energies
  Ein_param.lower_hits.common_hits( Ein_param.upper_hits );

  // integrate depending on how the arcs E_cm = const meet the box
  Vcm_Vlab_hit_list::iterator low_hit_ptr = Ein_param.lower_hits.begin( );
  Vcm_Vlab_hit_list::iterator next_low_ptr = low_hit_ptr;
  ++next_low_ptr;
  Vcm_Vlab_hit_list::iterator high_hit_ptr = Ein_param.upper_hits.begin( );
  Vcm_Vlab_hit_list::iterator next_high_ptr = high_hit_ptr;
  ++next_high_ptr;
  for( ; ( next_low_ptr != Ein_param.lower_hits.end( ) ) &&
         ( next_high_ptr != Ein_param.upper_hits.end( ) );
       low_hit_ptr = next_low_ptr, ++next_low_ptr,
         high_hit_ptr = next_high_ptr, ++next_high_ptr )
  {
    if( ( ( low_hit_ptr->hit_edge == ABOVE ) &&
	  ( high_hit_ptr->hit_edge == ABOVE ) ) ||
	( ( next_low_ptr->hit_edge == ABOVE ) &&
	  ( next_high_ptr->hit_edge == ABOVE ) ) ||
	( ( low_hit_ptr->hit_edge == ABOVE_FORWARD ) &&
	  ( high_hit_ptr->hit_edge == ABOVE_FORWARD ) ) ||
	( ( next_low_ptr->hit_edge == ABOVE_FORWARD ) &&
	  ( next_high_ptr->hit_edge == ABOVE_FORWARD ) ) ||
	( ( low_hit_ptr->hit_edge == BELOW ) &&
	  ( high_hit_ptr->hit_edge == BELOW ) ) ||
	( ( next_low_ptr->hit_edge == BELOW ) &&
	  ( next_high_ptr->hit_edge == BELOW ) ) )
    {
      continue;
    }
    // the range of integration in incident energy
    Ein_param.Ein_0 = low_hit_ptr->E_in;
    Ein_param.Ein_1 = next_low_ptr->E_in;
    update_T( transfer, Eout_count, Ein_param );
  }
}
// -----------  Kalbach::next_ladder ------------------
// Go to the next pair of incident energies.  Returns "true" when finished.
bool Kalbach::next_ladder( double E_in, kalbach_Ein_param *Ein_param )
{
  bool done = Ein_param->update_bin_pointers( E_in );
  static double etol = Global.Value( "E_tol" );
  double E_tol = E_in * etol;
  if( !done )
  {
    if( E_in + E_tol >= Ein_param->this_Ein->get_E_in( ) )
    {
      while( E_in + E_tol >= Ein_param->next_Ein->get_E_in( ) )
      {
        // get the next E_in Kalbach data
        Ein_param->this_Ein = Ein_param->next_Ein;
        ++Ein_param->next_Ein;
        if( Ein_param->next_Ein == end( ) )
        {
          return true;
        }
      }
    }
  }
  return done;
}
// -----------  Kalbach::update_T ------------------
// Adds to an element of transfer the integral between the intersections of 2 Eout_cm = const arcs with the Eout_lab box
  void Kalbach::update_T( T_matrix &transfer, int Eout_count,
     kalbach_Ein_param &Ein_param )
{
  static double tol = Global.Value( "quad_tol" );
  // a vector to store the integrals
  coef_vector value( transfer.order, transfer.conserve );
  value.set_zero( );

  // parameters for the integration
  QuadParamBase *params = static_cast< QuadParamBase* >( &Ein_param );

  // loop over the cross section data
  Ein_param.this_sigma = Ein_param.first_ladder_sigma;
  Ein_param.next_sigma = Ein_param.this_sigma;
  ++Ein_param.next_sigma;
  // Ein_param.Ein_0 may be past Ein_param.next_sigma
  while( ( Ein_param.this_sigma != Ein_param.last_ladder_sigma ) &&
         ( Ein_param.next_sigma->x < Ein_param.Ein_0 ) )
  {
    Ein_param.this_sigma = Ein_param.next_sigma;
    ++Ein_param.next_sigma;
  }
  for( ; ( Ein_param.this_sigma != Ein_param.last_ladder_sigma ) &&
         ( Ein_param.this_sigma->x <  Ein_param.Ein_1 );
       Ein_param.this_sigma = Ein_param.next_sigma, ++Ein_param.next_sigma )
  {
    double left_E = ( Ein_param.this_sigma->x < Ein_param.Ein_0 ) ? Ein_param.Ein_0 :
      Ein_param.this_sigma->x;
    double right_E = ( Ein_param.next_sigma->x > Ein_param.Ein_1 ) ? Ein_param.Ein_1 :
      Ein_param.next_sigma->x;
    // evaluate the integral
    quad_F::integrate( Kalbach_F::Ein_F, transfer.Ein_quad_method, left_E, right_E,
		       params, tol, &value );

    // add this integral
    transfer( Ein_param.Ein_count, Eout_count ) += value;
    // increment the function counts
    Ein_param.Ein_F_count += Ein_param.func_count;
    Ein_param.quad_count += Ein_param.Vcm_hit_count;
  }
}
// -----------  Kalbach::read_probability ------------------
// Reads the Kalbach energy probability densities
void Kalbach::read_probability( data_parser &input_file, int num_Ein )
{
  interp_flag_F::read_2d_interpolation( input_file, &Ein_interp, &Eout_interp );
  Kalbach_one_Ein next_Ein;
  Kalbach::iterator next_Ein_ptr;
  // read the data
  for( int Ein_count = 0; Ein_count < num_Ein; ++Ein_count )
  {
    // make Kalbach list for a new incident energy
    insert( end( ), next_Ein );
    // point to it
    next_Ein_ptr = end( );
    --next_Ein_ptr;
    // get the incident energy and the data pairs
    next_Ein_ptr->set_E_in( input_file.get_next_double( ) );
    int num_Eout_cm = input_file.get_next_int( );
    next_Ein_ptr->read_probability( input_file, num_Eout_cm );
  }
}
// -----------  Kalbach::read_r ------------------
// Reads the Kalbach energy r parameters
void Kalbach::read_r( data_parser &input_file, int num_Ein )
{
  static double etol = Global.Value( "abs_tol" );

  interp_flag_F::read_2d_interpolation( input_file, &Ein_r_interp, &r_interp );
  Kalbach::iterator next_Ein_ptr = begin( );
  // read the data
  for( int Ein_count = 0; Ein_count < num_Ein; ++Ein_count, ++next_Ein_ptr )
  {
    // get the incident energy and the data pairs
    double r_Ein = input_file.get_next_double( );
    double prob_Ein = next_Ein_ptr->get_E_in( );
    if( abs( r_Ein - prob_Ein ) > etol * prob_Ein )
    {
      cout << "probability Ein: " << prob_Ein << endl;
      cout << "r Ein: " << r_Ein << endl;
      FatalError( "Kalbach::read_r", "inconsistent incident energies" );
    }

    next_Ein_ptr->Eout_r.set_E_in( r_Ein );
    int num_Eout_cm = input_file.get_next_int( );
    next_Ein_ptr->read_r( input_file, num_Eout_cm );
  }
}
// -----------  Kalbach::check_data ------------------
// Checks the data for consistency
void Kalbach::check_data( )
{
  if( ( ( Ein_interp.qualifier != UNITBASE ) &&
	( Ein_interp.qualifier != DIRECT ) ) ||
      ( Ein_interp.flag != LINLIN ) )
  {
    FatalError( "Kalbach::check_data", "incident energy interpolation not implemented" );
  }
  if( ( Eout_interp != HISTOGRAM ) && ( Eout_interp != LINLIN ) )
  {
    FatalError( "Kalbach::check_data", "outgoing energy interpolation not implemented" );
  }
  if( kalbach_a.eject.mass == 0.0 )
  {
    FatalError( "Kalbach::check_data", "Photon emission not implemented" );
  }
  if( Ein_interp.qualifier == DIRECT )
  {
    Warning( "Kalbach::check_data",
      "linear-linear direct interpolation may violate energy conservation" );
  }
  if( ( Ein_interp.qualifier == UNITBASE ) &&
      ( Ein_r_interp.qualifier != UNSCALED_UNITBASE ) )
  {
    FatalError( "Kalbach::check_data",
       "incident energy unit base interpolation inconsistent" );
  }
  if( ( Ein_interp.qualifier == DIRECT ) && 
      ( Ein_r_interp.qualifier != UNSCALED_DIRECT ) )
  {
    FatalError( "Kalbach::check_data",
       "incident energy lin-lin interpolation inconsistent" );
  }

  // check the ranges of outgoing energy
  double etol = Global.Value( "abs_tol" );
  for( Kalbach::iterator Ein_ptr = begin( ); Ein_ptr != end( );
       ++Ein_ptr )
  {
    Kalbach_one_Ein::const_iterator Eout_ptr = Ein_ptr->begin( );
    dd_vector::const_iterator r_ptr = Ein_ptr->Eout_r.begin( );
    if( abs( Eout_ptr->x - r_ptr->x ) > etol*Eout_ptr->x )
    {
      FatalError( "Kalbach::check_data",
	 pastenum( "first r outgoing energy ", r_ptr->x ) + " inconsistent" );
    }
    Eout_ptr = Ein_ptr->end( );
    --Eout_ptr;
    r_ptr = Ein_ptr->Eout_r.end( );
    --r_ptr;
    if( abs( Eout_ptr->x - r_ptr->x ) > etol*Eout_ptr->x )
    {
      FatalError( "Kalbach::check_data",
	 pastenum( "final r outgoing energy ", r_ptr->x ) + " inconsistent" );
    }
  }
}
// -----------  Kalbach::get_T ------------------
// Calculates the transfer matrix for this particle
void Kalbach::get_T( const dd_vector& sigma, const dd_vector& multiple, 
  const dd_vector& weight, T_matrix& transfer )
{
  // Check the data
  check_data( );

  // Is there data inside the incident energy range?
  bool done = get_Ein_range( sigma, multiple, weight, transfer.e_flux,
    transfer.in_groups );
  if( done )
  {
    transfer.zero_transfer( );
  }

  for( Kalbach::iterator this_link = begin( ); this_link != end( ); ++this_link )
  {
    this_link->interp_type = Eout_interp;
    this_link->Eout_r.interp_type = Eout_interp;
    // make sure that the histogram data ends properly
    if( Eout_interp == HISTOGRAM )
    {
      this_link->chop_histogram( transfer.out_groups[ transfer.num_Eout_bins ] );
    }
  }
  if( Ein_interp.qualifier == UNITBASE )
  {  // map to unit base
    for( Kalbach::iterator this_link = begin( ); this_link != end( ); ++this_link )
    {
      this_link->Eout_r.mapto_01( &this_link->unit_base );
      this_link->dd_vector::unit_base( true, &this_link->unit_base );
    }
  }

  long int quad_count = 0;  // number of 3-d quadratures
  long int Ein_F_count= 0;  // number of calls to Kalbach_F::Ein_F
  long int Ecm_F_count= 0;  // number of calls to Kalbach_F::Ecm_F
  long int mu_F_count = 0;  // number of calls to Kalbach_F::mu_F

  // now do the integrals incident bin by incident bin
#pragma omp parallel for schedule( dynamic, 1 ) default( none )	\
  shared( sigma, multiple, weight, transfer ) \
  reduction( +: quad_count ) reduction( +: Ein_F_count ) \
  reduction( +: Ecm_F_count ) reduction( +: mu_F_count )
  for( int Ein_bin = first_Ein; Ein_bin < last_Ein; ++Ein_bin )
  {
    kalbach_Ein_param Ein_param;
    Ein_param.map = &map;
    Ein_param.Eout_quad_method = transfer.Eout_quad_method;
    Ein_param.mu_quad_method = transfer.mu_quad_method;
    // set up the data range for this bin
    Ein_param.setup_bin( Ein_bin, sigma, multiple, weight, transfer.e_flux,
                         transfer.in_groups );
    setup_data( &Ein_param );
    // work on this bin
    for( ; ; )
    {
      // get the incident energy interval common to all data
      set_Ein_range( Ein_bin, Ein_param );
      cm_Eout_ladder( transfer, &Ein_param );  // loop over the outgoing cm energies
      // go to the next interval
      bool Done = next_ladder( Ein_param.data_E_1, &Ein_param );
      if( Done )
      {
        break;
      }
    }
    quad_count += Ein_param.quad_count;
    Ein_F_count += Ein_param.Ein_F_count;
    Ecm_F_count += Ein_param.Ecm_F_count;
    mu_F_count += Ein_param.mu_F_count;
  } // end of parallel loop

  // print the counts of function evaluations
  cout << "3d quadratures: " << quad_count << endl;
  cout << "Kalbach_F::Ein_F calls: " << Ein_F_count << endl;
  cout << "Kalbach_F::Ecm_F calls: " << Ecm_F_count << endl;
  cout << "Kalbach_F::mu_F calls: " << mu_F_count << endl;
  cout << "average Kalbach_F::Ein_F calls: " << 1.0*Ein_F_count/quad_count << endl;
  cout << "average Kalbach_F::Ecm_F calls: " << 1.0*Ecm_F_count/Ein_F_count << endl;
  cout << "average Kalbach_F::mu_F calls: " << 1.0*mu_F_count/Ecm_F_count << endl;
}

// **************** Functions to integrate *********************
// --------------------  Kalbach_F::mu_F ------------------
// Function for the 1-d quadrature over cm cosine
void Kalbach_F::mu_F( double mu, QuadParamBase *mu_quad_param, coef_vector *value )
{
  // the parameters are really kalbach_mu_param
  kalbach_mu_param *mu_params = static_cast< kalbach_mu_param* >( mu_quad_param );
  mu_params->func_count += 1;

  double Eout_lab;
  double mu_lab;
  mu_params->map->get_E_mu_lab( mu_params->E_in, mu_params->Eout_cm, mu, &Eout_lab,
    &mu_lab );

  // the Legendre polynomials
  math_F::Legendre( mu_lab, value );
  // the energy-angle probability density
  double aa = mu_params->a;
  double normalize = 2*sinh( aa )/aa;
  double Prob;
  if( !mu_params->gamma_in )
  {
    Prob = ( mu_params->Ecm_prob / normalize )*
      ( cosh( aa*mu ) + mu_params->r*sinh( aa*mu ) );
  }
  else  // incident gamma
  {
    Prob = ( mu_params->Ecm_prob ) * ( ( 1.0 - mu_params->r )/2 +
      mu_params->r * exp( aa*mu )/normalize );
  }
  *value *= Prob;

  // do the energy weighting if necessary
  if( ( value->conserve == ENERGY ) || ( value->conserve == BOTH ) )
  {
    value->scale_E( Eout_lab );
  }
}
// ------------------- Kalbach_F::Ecm_F ------------------
// Function for the 2-d quadrature over cm cosine and Eout_cm
void Kalbach_F::Ecm_F( double Eout_cm, QuadParamBase *Ecm_quad_param, coef_vector *value )
{
  // the parameters are really kalbach_Ecm_param *
  kalbach_Ecm_param *Ecm_param = static_cast<kalbach_Ecm_param *>( Ecm_quad_param );
  Ecm_param->func_count += 1;
  /*
   *  if( Ecm_param->func_count % 100 == 0 )
   *  {
   *    Info( "Kalbach_F::Ecm_F", pastenum( "got ", Ecm_param->func_count ) + " evaluations");
   *    cout << "lab_Eout_max: " << Ecm_param->lab_Eout_max <<
   *     " E_trans: " << Ecm_param->kalbach_a->map->get_Etrans( Ecm_param->Ein_data.E_in ) <<
   *      " Ecm_max: " << Ecm_param->Ecm_max << endl;
   *  }
   */
  // The value of Kalbach_F::Ecm_F is itself an integral over cm cosine.
  // *value comes in as 0.  

  // parameters for the integration over cm cosine
  kalbach_mu_param mu_param;
  mu_param.setup( Ecm_param->Ein_data.E_in, Eout_cm, *Ecm_param );

  // evaluate the integral over eta
  QuadParamBase *params = static_cast< QuadParamBase* >( &mu_param );
  static double tol = Global.Value( "quad_tol" );
  quad_F::integrate( Kalbach_F::mu_F, Ecm_param->mu_quad_method, mu_param.mu_cm_min,
		     mu_param.mu_cm_max, params, tol, value );
  Ecm_param->mu_F_count += mu_param.func_count;
}
// ------------------- Kalbach_F::Ein_F ------------------
// Function for the 3-d quadrature over E_in, and Eout_cm and cm cosine
// The value of Kalbach_F::Ein_F is itself an integral over Eout_cm and cm cosine.
void Kalbach_F::Ein_F( double E_in, QuadParamBase *Ein_quad_param, coef_vector *value )
{
  value->set_zero( );  // initialize to zero
  // the parameters are really kalbach_Ein_param *
  kalbach_Ein_param *Ein_param = static_cast<kalbach_Ein_param *>( Ein_quad_param );
  Ein_param->Vcm_hit_count = 0;   // number of local calls to quad_F::integrate

  // set up parameters for the integration over Eout_cm and cm cosine
  Ein_param->set_Ecm_param( E_in );
  QuadParamBase *params = static_cast< QuadParamBase* >( &Ein_param->Ecm_params );

  // Integrate over sectors of ( Eout_lab, mu_cm ) space
  coef_vector one_value( value->order, value->conserve );

  list< Vcm_quadBox_Hit >::const_iterator this_V_hit =
      Ein_param->Ecm_params.V_cm_limits.begin( );
  list< Vcm_quadBox_Hit >::const_iterator next_V_hit = this_V_hit;
  ++next_V_hit;
  for( ; next_V_hit != Ein_param->Ecm_params.V_cm_limits.end( );
         this_V_hit = next_V_hit, ++next_V_hit )
  {
    if( ( next_V_hit->V_cm <= Ein_param->Ecm_params.min_V_cm ) ||
        ( this_V_hit->hit_corner == V_BELOW ) )
    {
      continue;  // current V_cm values are below
    }
    else if( ( this_V_hit->V_cm >= Ein_param->Ecm_params.max_V_cm ) ||
             ( next_V_hit->hit_corner == V_ABOVE ) )
    {
      break;  // all remaining V_cm values are above
    }
    else
    {
      Ein_param->Vcm_hit_min = *this_V_hit;
      Ein_param->Vcm_hit_max = *next_V_hit;
    }
    //    cout << "integrate V_lab" << endl;
    //    Ein_param->Vcm_hit_min.print( );
    //    Ein_param->Vcm_hit_max.print( );
    Ein_param->Ecm_params.min_hit_corner = Ein_param->Vcm_hit_min.hit_corner;
    Ein_param->Ecm_params.max_hit_corner = Ein_param->Vcm_hit_max.hit_corner;
    double tol = Ein_param->Ecm_params.Ecm_range( );
    Ein_param->Ecm_params.mu_F_count = 0;

    quad_F::integrate( Kalbach_F::Ecm_F, Ein_param->Eout_quad_method,
                       Ein_param->Ecm_params.Ecm_min,
		       Ein_param->Ecm_params.Ecm_max, params, tol, &one_value );
    *value += one_value;
    // we actually want to count the number of 3d integrals
    Ein_param->Vcm_hit_count += 1;
    Ein_param->func_count += 1;
    Ein_param->Ecm_F_count += Ein_param->Ecm_params.func_count;
    Ein_param->mu_F_count += Ein_param->Ecm_params.mu_F_count;
//    if( Ein_param->func_count % 100 == 0 )
//    {
//      Info( "Kalbach_F::Ein_F", pastenum( "got ", Ein_param->func_count ) +
//        " evaluations");
//    }
  }
  // weight it by flux * cross section
  Ein_param->set_weight( E_in );
  *value *= Ein_param->current_weight;
  //  cout << "E_in: " << E_in << " eta_0: " << eta_0 << " eta_1: " <<
  //    eta_1 << endl;
  //  value->print( );
}
