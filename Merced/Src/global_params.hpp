/*
 * ******** merced: calculate the transfer matrix *********
 * $Revision: 1 $
 * $Date: 2006-02-01 19:06:56 -0800 (Wed, 01 Feb 2006) $
 * $Author: hedstrom $
 * $Id:global_params.hpp 1 2006-02-02 03:06:56Z hedstrom $
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

#ifndef GLOBAL_PARAMS
#define GLOBAL_PARAMS

#include <cstdio>             // standard I/O package
#include <fstream>             // standard file streams
#include <iostream>            // for file I/O functions
#include <sstream>
#include <iomanip>             // I/O formatting
#include <string>              // for string fun
#include <cmath>               // math library
#include <list>
#include <vector>

using namespace std;

// ----------------------- class ss_link -------------------
//! Links for the list of input parameters
class ss_link
{
private:
  string x;  // the variable name
  string y;  // the value of the variable

public:
  // is this set in the command line?
  bool set_in_command;

  // default constructor
  ss_link( ): set_in_command( false )
  { }

  // default deonstructor
  ~ss_link( )
  { }

  //!Common name used to access the X value of this list.
  inline string name( )
  {
    return x;
  }

  //!Common name used to access the Y value of this list.
  inline string value( )
  {
    return y;
  }

  //!Sets the X value of this list.
  //! \param xx the label portion of this entry
  inline void set_name( const string &xx )
  {
    x = xx;
  }

  //!Sets the Y value of this list.
  //! \param yy the value portion of this entry
  inline void set_value( const string &yy )
  {
    y = yy;
  }

  //! Method for printing this link.
  void print( );
};

// ----------------------- class ss_list -------------------
//! List of ss_links.
class ss_list : public list< ss_link >
{
public:
  //! Function to get the link at "x".
  //! Returns "false" if the entry is not found.
  //! \param x the desired label
  //! \param link the link with this label, if any
  bool at(string x, ss_list::iterator& link);

  void print( );

};

// ----------------------- class GlobalParameterClass -----------
//! Read/store the global parameter settings for the translation
class GlobalParameterClass
{
public:

  //!The output file when we write to only one file
  fstream output_file;

  //!Default constructor - initializes Parameter list and values
  GlobalParameterClass();

  //!Default destructor.
  ~GlobalParameterClass();

  //! Function to get the link at "x"
  //! Returns "false" if the entry is not found.
  //! \param x the desired label
  //! \param link the link with this label, if any
  bool find(string x, ss_list::iterator& link);

  //!The function used to input information into the Parameter list.
  //! \param gp the label for this entry
  //! \param gp_num the numerical value for this entry
  void set( string gp, double gp_num);

  //!The function used to input information into the Parameter list.
  //! \param gp the label for this entry
  //! \param gp_val the string value for this entry
  void set( string gp, string gp_val);

  //! Function to print a table of parameters and values used in the translation.
  void print();

  //! Function to parse command line arguments and update Parameter list.
  //! \param argc number of words in the command line
  //! \param argv the command line
  void read_command_line(int argc, char* argv[] );

  //! Function to print usage info & print parameter names.
  void get_help( void );

  //!Delivers the stored value of the requested parameter.
  //! \param ParamName the desired label
  double Value( string ParamName );   

  //!Delivers the string representation of the requested parameter.
  //! \param ParamName the desired label
  string Flag( string ParamName );

  //! Gets the datafield precision for output
  int get_field_width( );

  //! Sets the number of threads to use in parallel computing
  void set_num_threads( );

private:

  //! Local string buffer
  vector< string > spar;

  //!Parameters stored in name indexed link list
  ss_list Parameters;

  //! Sets the set_in_command flag
  //! \param ParamName the desired label
  void set_command( string ParamName );
};

extern GlobalParameterClass Global;

#endif
