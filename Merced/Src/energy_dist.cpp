/*
 * ******** merced: calculate the transfer matrix *********
 * $Revision: 1 $
 * $Date: 2011-04-28 19:06:56 -0800 (Fri, 28 Feb 2011) $
 * $Author: hedstrom $
 * $Id: energy_dist.cpp 1 2011-04-28 03:06:56Z hedstrom $
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
// implementation of the classes used to handle energy distributions

#include <cmath>

#include "energy_dist.hpp"
#include "global_params.hpp"
#include "messaging.hpp"

// ----------- energy_dist::destructor --------------
energy_dist::~energy_dist( )
{
  if( number_Ein > 0 )
  {
    delete [] EProb_data;
  }
}
// ----------- energy_dist::read_data --------------
// Reads the ENDL data for one Legendre order
void energy_dist::read_data( data_parser& input_file, int num_Ein )
{
  number_Ein = num_Ein;
  EProb_data = new Eprob_vector[ num_Ein ];
  Eprob_vector *new_energy_ptr;
  // loop over incident energy
  for( int Ein_count = 0; Ein_count < num_Ein; ++Ein_count )
  {
    // make a new energy distribution
    new_energy_ptr = &( EProb_data[ Ein_count ] );
    new_energy_ptr->set_E_in( input_file.get_next_double( ) );
    new_energy_ptr->interp_type = Eout_interp;
    // read the (energy, probability density) pairs
    int num_Eout = input_file.get_next_int( );
    for( int Eout_count = 0; Eout_count < num_Eout; ++Eout_count )
    {
      double E_out = input_file.get_next_double( );
      double Prob = input_file.get_next_double( );
      new_energy_ptr->add_entry( E_out, Prob );
    }
  }
}
// ----------- energy_dist::unit_base --------------
// Maps the data to unit base
void energy_dist::unit_base( int L_order )
{
  for( int Ein_count = 0; Ein_count < number_Ein; ++Ein_count )
  {
    EProb_data[ Ein_count ].unit_base( L_order );
  }
}

// ************* class energy_moments *****************
// ----------- energy_moments::destructor --------------
energy_moments::~energy_moments( )
{
  if( data_order >= 0 )
  {
    delete [] ENDL_data;
  }
}
// ----------- energy_moments::read_data --------------
void energy_moments::read_data( data_parser& input_file, int num_moments )
{
  // Read the interpolation rules
  interp_flag_F::read_2d_interpolation( input_file, &Ein_interp, &Eout_interp );
  data_order = num_moments - 1;
  ENDL_data = new energy_dist[ data_order + 1 ];
  energy_dist *new_moment_ptr;

  // read the data
  for( int Legendre_order = 0; Legendre_order <= data_order;
    ++Legendre_order )
  {
    // the next Legendre order
    new_moment_ptr = &ENDL_data[ Legendre_order ];
    // is the Legendre order consistent?
    int file_order = input_file.get_next_int( );
    if( file_order != Legendre_order )
    {
      FatalError( "energy_moments::read_data",
		  pastenum( "improper Legendre order: ", file_order ) );
    }
    new_moment_ptr->Ein_interp = Ein_interp;
    new_moment_ptr->Eout_interp = Eout_interp;
    // how many incident energies
    int num_Ein = input_file.get_next_int( );
    new_moment_ptr->read_data( input_file, num_Ein );
    if( Ein_interp.qualifier == UNITBASE )
    {
      new_moment_ptr->unit_base( Legendre_order );
    }
  }
}
// ----------- energy_moments::zero_order --------------
// Converts isotropic ENDL data to ENDF format
void energy_moments::zero_order( )
{
  // copy the interpolation rules
  ENDF_data.Ein_interp.flag = Ein_interp.flag;
  ENDF_data.Ein_interp.qualifier = Ein_interp.qualifier;
  ENDF_data.Eout_interp = Eout_interp;

  energy_dist *ENDL_ptr = &ENDL_data[ 0 ];
  int num_Ein = ENDL_ptr->number_Ein;
  Eprob_vector *ENDL_Ein_ptr;
  Eprob_vector::const_iterator ENDL_Eout_ptr;
  standard_Legendre::iterator ENDF_Ein_ptr;
  standard_Legendre_vector::iterator ENDF_Eout_ptr;
  // iterate through the incident energies
  for( int Ein_count = 0; Ein_count < num_Ein; ++Ein_count )
  {
    ENDL_Ein_ptr = &( ENDL_ptr->EProb_data[ Ein_count ] );
    // make a new standard_Legendre_vector
    ENDF_Ein_ptr = ENDF_data.insert( ENDF_data.end( ), standard_Legendre_vector( ) );
    ENDF_Ein_ptr->set_E_in( ENDL_Ein_ptr->get_E_in( ) );  // energy of incident particle
    ENDF_Ein_ptr->Eout_interp = Eout_interp;
    if( Ein_interp.qualifier == UNITBASE )
    {
      ENDF_Ein_ptr->ubase_map.copy( ENDL_Ein_ptr->ubase_map );
    }

    // go through the pairs ( E_out, probability_density )
    for( ENDL_Eout_ptr = ENDL_Ein_ptr->begin( );
	 ENDL_Eout_ptr != ENDL_Ein_ptr->end( ); ++ENDL_Eout_ptr )
    {
      // make a new set of Legendre coefficients
      ENDF_Eout_ptr = ENDF_Ein_ptr->insert( ENDF_Ein_ptr->end( ), Legendre_coefs( ) );
      ENDF_Eout_ptr->initialize( 0 );
      ENDF_Eout_ptr->set_E_out( ENDL_Eout_ptr->x );  // energy of outgoing particle
      ( *ENDF_Eout_ptr )[ 0 ] = ENDL_Eout_ptr->y;
    }
    if( Ein_interp.qualifier == CUMULATIVE_POINTS )
    {
      ENDF_Ein_ptr->form_cum_prob( );
    }
  }
}
// ----------- energy_moments::to_ENDF --------------
// Converts ENDL data to ENDF format
void energy_moments::to_ENDF( )
{
  // Do we need to interpolate with respect to incident energy?
  check_Ein( );
  // copy the interpolation rules
  ENDF_data.Ein_interp.flag = Ein_interp.flag;
  ENDF_data.Ein_interp.qualifier = Ein_interp.qualifier;
  ENDF_data.Eout_interp = Eout_interp;

  // iterate over incident energy
  int num_Ein = ENDL_data[ 0 ].number_Ein;
  for( int energy_count = 0; energy_count < num_Ein; ++energy_count )
  {
    one_Ein_to_ENDF( energy_count );
  }
}
// ----------- energy_moments::check_Ein --------------
// Checks to see that the incident energies are consistent for all Legendre orders
void energy_moments::check_Ein( ) const
{
  // first, check the amount of data
  int num_Ein = ENDL_data[ 0 ].number_Ein;
  for( int L_order = 1; L_order <= data_order; ++L_order )
  {
    if( ENDL_data[ L_order ].number_Ein != num_Ein )
    {
      FatalError( "energy_moments::check_Ein",
		"Implement differing numbers of incident energies" );
    }
  }
  // now check the incident energies
  static double tol = Global.Value( "E_tol" );
  for( int Ein_count = 0; Ein_count < num_Ein; ++Ein_count )
  {
    double this_Ein = ENDL_data[ 0 ].EProb_data[ Ein_count ].get_E_in( );
    for( int L_order = 1; L_order <= data_order; ++L_order )
    {
      if( abs( ENDL_data[ L_order ].EProb_data[ Ein_count ].get_E_in( ) - this_Ein )>
	  tol*this_Ein )
      {
        FatalError( "energy_moments::check_Ein",
		"Implement interpolation with respect to incident energy" );
      }
    }
  }
}
// ----------- energy_moments::one_Ein_to_ENDF --------------
//! Converts ENDL data to ENDF format for one incident energy
void energy_moments::one_Ein_to_ENDF( int Ein_count )
{
  int L_order;
  // make a new standard_Legendre_vector for this incident energy
  standard_Legendre::iterator ENDF_Ein_ptr;
  standard_Legendre_vector::iterator ENDF_Eout_ptr;
  // where we are in the ENDL data
  Eprob_vector::const_iterator *ENDL_Eout_ptr =
    new Eprob_vector::const_iterator[ data_order + 1 ];
  Eprob_vector::const_iterator *next_ENDL_ptr =
    new Eprob_vector::const_iterator[ data_order + 1 ];
  for( L_order = 0; L_order <= data_order; ++L_order )
  {
    ENDL_Eout_ptr[ L_order ] = ENDL_data[ L_order ].EProb_data[ Ein_count ].begin( );
    next_ENDL_ptr[ L_order ] = ENDL_Eout_ptr[ L_order ];
    ++next_ENDL_ptr[ L_order ];
  }
  ENDF_Ein_ptr = ENDF_data.insert( ENDF_data.end( ), standard_Legendre_vector( ) );
  ENDF_Ein_ptr->set_E_in( ENDL_data[ 0 ].EProb_data[ Ein_count ].get_E_in( ) );
  ENDF_Ein_ptr->Eout_interp = Eout_interp;
  if( Ein_interp.qualifier == UNITBASE )
  {
    ENDF_Ein_ptr->ubase_map.copy( ENDL_data[ 0 ].EProb_data[ Ein_count ].ubase_map );
  }
  // go through the ENDL data
  bool done = false;
  double next_Eout = ENDL_Eout_ptr[ 0 ]->x;  // this has to be zero
  for( ; ; )
  {
    // make a new set of Legendre coefficients
    ENDF_Eout_ptr = ENDF_Ein_ptr->insert( ENDF_Ein_ptr->end( ), Legendre_coefs( ) );
    ENDF_Eout_ptr->initialize( output_order );
    ENDF_Eout_ptr->set_E_out( next_Eout );  // energy of outgoing particle
    for( L_order = 0; L_order <= data_order; ++L_order )
    {
      if( Ein_interp.qualifier == UNITBASE )
      {
        ( *ENDF_Eout_ptr )[ L_order ] = 
  	  ENDL_Eout_ptr[ L_order ]->linlin_interp( next_Eout, *next_ENDL_ptr[ L_order ] );
      }
      else
      {
	( *ENDF_Eout_ptr )[ L_order ] = ENDL_Eout_ptr[ L_order ]->y;
      }
    }
    if( done )
    {
      break;
    }
    // get the next outgoing energy
    static double tol = Global.Value( "E_tol" );
    double huge_E = 1.0e20;  // a dummy large value
    next_Eout = huge_E;
    for( L_order = 0; L_order <= data_order; ++L_order )
    {
      if( next_ENDL_ptr[ L_order ]->x < ( 1.0 - tol )*next_Eout )
      {
	next_Eout = next_ENDL_ptr[ L_order ]->x;
      }
    }
    // update the pointers
    if( next_Eout > 1.0 - tol )
    {
      done = true;
    }
    else
    {
      for( L_order = 0; L_order <= data_order; ++L_order )
      {
        if( next_ENDL_ptr[ L_order ]->x < ( 1.0 + tol )*next_Eout )
        {
	  ENDL_Eout_ptr[ L_order ] = next_ENDL_ptr[ L_order ];
	  ++next_ENDL_ptr[ L_order ];
	}
      }
    }
  }
  delete [] ENDL_Eout_ptr;
  delete [] next_ENDL_ptr;
}
// ----------- energy_moments::get_T --------------
// Calculates the transfer matrix for this particle.
// sigma is the cross section.
void energy_moments::get_T( const dd_vector& sigma,
  const dd_vector& mult, const dd_vector& weight, T_matrix& transfer )
{
  bool interp_OK = ( ( Ein_interp.qualifier == UNITBASE ) && 
                     ( Ein_interp.flag == LINLIN ) ) ||
    ( ( Ein_interp.qualifier == DIRECT ) &&
      ( Ein_interp.flag == HISTOGRAM ) );
  if( !interp_OK )
  {
    FatalError( "energy_moments::get_T",
      "Incident energy interpolation not implemented" );
  }
  interp_OK = ( Eout_interp == LINLIN ) || ( Eout_interp == HISTOGRAM );
  if( !interp_OK )
  {
    FatalError( "energy_moments::get_T",
      "Outgoing energy interpolation not implemented" );
  }
  output_order = transfer.order;
  if( data_order > output_order )
  {
    data_order = output_order;  // truncate the data
  }
  // convert to ENDF format
  if( output_order == 0 )
  {
    zero_order( );
  }
  else
  {
    to_ENDF( );
  }
  ENDF_data.Ein_interp = Ein_interp;
  ENDF_data.Eout_interp = Eout_interp;
  ENDF_data.get_T( sigma, mult, weight, transfer );
}
