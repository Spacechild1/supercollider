/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _SC_WireSpec_
#define _SC_WireSpec_

#include "SC_Types.h"

enum {
	calc_Scalar,
	calc_Buf,
	calc_Full
};

struct InputSpec
{
	// read from file:
	int16 mFromUnitIndex;
	int16 mFromOutputIndex;
	// computed:
	int16 mWireIndex;
};
typedef struct InputSpec InputSpec;

struct OutputSpec
{
	// read from file:
	int16 mCalcRate;
	// computed:
	int16 mWireIndex;
	int16 mBufferIndex;
	int16 mNumConsumers;
};
typedef struct OutputSpec OutputSpec;

#endif
