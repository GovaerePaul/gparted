/* Copyright (C) 2004 Bart
 * Copyright (C) 2008, 2009, 2010 Curtis Gedak
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
 
 /* READ THIS!!
  * Partition isn't really a partition. It's more like a geometry, a continuous part of the disk. 
  * I use it to represent partitions as well as unallocated spaces
  */

#ifndef GPARTED_PARTITION_H
#define GPARTED_PARTITION_H

#include "../include/Utils.h"
#include "../include/PartitionVector.h"

#include <glibmm/ustring.h>

namespace GParted
{
	

enum PartitionType {
	TYPE_PRIMARY		=	0,
	TYPE_LOGICAL		=	1,
	TYPE_EXTENDED		=	2,
	TYPE_UNALLOCATED	=	3 
};

enum PartitionStatus {
	STAT_REAL	=	0,
	STAT_NEW	=	1,
	STAT_COPY	=	2,
	STAT_FORMATTED	=	3
};

enum PartitionAlignment {
	ALIGN_CYLINDER = 0,    //Align to nearest cylinder
	ALIGN_MEBIBYTE = 1,    //Align to nearest mebibyte
	ALIGN_STRICT   = 2     //Strict alignment - no rounding
	                       //  Indicator if start and end sectors must remain unchanged
};

class Partition;        // Forward declarations as Partition and PartitionVector are
class PartitionVector;  // mutually recursive classes.
                        // References:
                        // *   Mutually recursive classes
                        //     http://stackoverflow.com/questions/3410637/mutually-recursive-classes
                        // *   recursive definition in CPP
                        //     http://stackoverflow.com/questions/4300420/recursive-definition-in-cpp

class Partition
{
public:
	Partition() ;
	virtual ~Partition();
	virtual Partition * clone() const;

	void Reset() ;
	
	//simple Set-functions.  only for convenience, since most members are public
	void Set( const Glib::ustring & device_path,
	          const Glib::ustring & partition,
	          int partition_number,
	          PartitionType type,
	          bool whole_device,
	          FILESYSTEM filesystem,
	          Sector sector_start,
	          Sector sector_end,
	          Byte_Value sector_size,
	          bool inside_extended,
	          bool busy );

	void set_sector_usage( Sector sectors_fs_size, Sector sectors_fs_unused ) ;
	virtual bool sector_usage_known() const;
	virtual Sector estimated_min_size() const;
	virtual Sector get_sectors_used() const;
	virtual Sector get_sectors_unused() const;
	virtual Sector get_sectors_unallocated() const;
	void get_usage_triple( int imax, int & i1, int & i2, int & i3 ) const ;

	void Set_Unallocated( const Glib::ustring & device_path,
	                      bool whole_device,
	                      Sector sector_start,
	                      Sector sector_end,
	                      Byte_Value sector_size,
	                      bool inside_extended );

	//update partition number (used when a logical partition is deleted) 
	void Update_Number( int new_number );
	
	void add_path( const Glib::ustring & path, bool clear_paths = false ) ;
	void add_paths( const std::vector<Glib::ustring> & paths, bool clear_paths = false ) ;
	Byte_Value get_byte_length() const ;
	Sector get_sector_length() const ; 
	Glib::ustring get_path() const ;
	std::vector<Glib::ustring> get_paths() const ;
	void add_mountpoint( const Glib::ustring & mountpoint, bool clear_mountpoints = false ) ;
	void add_mountpoints( const std::vector<Glib::ustring> & mountpoints, bool clear_mountpoints = false ) ;
	Glib::ustring get_mountpoint() const ; 
	void clear_mountpoints() ;
	std::vector<Glib::ustring> get_mountpoints() const ;
	Sector get_sector() const ;
	bool test_overlap( const Partition & partition ) const ;
	bool filesystem_label_known() const;
	virtual Glib::ustring get_filesystem_label() const;
	void set_filesystem_label( const Glib::ustring & filesystem_label );

	// Message accessors.  Messages are stored locally and accessed globally.
	// Stored locally means the messages are stored in the Partition object to which
	// they are added so push_back_messages() and append_messages() are non-virtual.
	// Accessed globally means that in the case of derived objects which are composed
	// of more that one Partition object, i.e. PartitionLUKS, the messages have to be
	// accessible from all copies within the derived object hierarchy.  Hence
	// have_messages(), get_messages() and clear_messages() are virtual to allow for
	// overridden implementations.
	virtual bool have_messages() const                             { return ! messages.empty(); };
	virtual std::vector<Glib::ustring> get_messages() const        { return messages; };
	virtual void clear_messages()                                  { messages.clear(); };
	void push_back_message( Glib::ustring msg )                    { messages.push_back( msg ); };
	void append_messages( const std::vector<Glib::ustring> msgs )
	                                { messages.insert( messages.end(), msgs.begin(), msgs.end() ); }

	bool operator==( const Partition & partition ) const ;
	bool operator!=( const Partition & partition ) const ;
		
	//some public members
	Glib::ustring device_path ;
	int partition_number;
	PartitionType type;// UNALLOCATED, PRIMARY, LOGICAL, etc...
	bool whole_device;  // Is this a virtual partition spanning a whole unpartitioned disk device?
	PartitionStatus status; //STAT_REAL, STAT_NEW, etc..
	PartitionAlignment alignment;   //ALIGN_CYLINDER, ALIGN_STRICT, etc
	FILESYSTEM filesystem ;
	Glib::ustring uuid ;
	Glib::ustring name;
	Sector sector_start;
	Sector sector_end;
	Sector sectors_used;
	Sector sectors_unused;
	Sector sectors_unallocated;  //Difference between the size of the partition and the file system
	Sector significant_threshold;  //Threshold from intrinsic to significant unallocated sectors
	bool inside_extended;
	bool busy;
	std::vector<Glib::ustring> flags ;

	PartitionVector logicals;

	bool strict_start ;	//Indicator if start sector must stay unchanged
	Sector free_space_before ;  //Free space preceding partition value

	Byte_Value sector_size ;  //Sector size of the disk device needed for converting to/from sectors and bytes.
	Byte_Value fs_block_size;  // Block size of of the file system, or -1 when unknown.

private:
	Partition & operator=( Partition & rhs );  // Not implemented copy assignment operator

	static void get_usage_triple_helper( Sector stot, Sector s1, Sector s2, Sector s3, int imax, int & i1, int & i2, int & i3 ) ;

	void sort_paths_and_remove_duplicates() ;
	Sector calc_significant_unallocated_sectors() const ;

	static bool compare_paths( const Glib::ustring & A, const Glib::ustring & B ) ;
	
	std::vector<Glib::ustring> paths ;
	std::vector<Glib::ustring> mountpoints ;
	bool have_filesystem_label;
	Glib::ustring filesystem_label;
	std::vector<Glib::ustring> messages;
};

}//GParted

#endif /* GPARTED_PARTITION_H */
