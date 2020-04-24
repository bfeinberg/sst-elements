// Copyright 2009-2018 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2018, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#include <sst_config.h>
#include "rdmaLib.h"

using namespace SST::Aurora::RDMA;
using namespace Hermes;


RdmaLib::RdmaLib( ComponentId_t id, Params& params) : 
	HostLib< Hermes::RDMA::Interface, NicCmd, RetvalResp >( id, params )
{
	dbg().debug(CALL_INFO,1,2,"\n");
}
