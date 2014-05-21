// Copyright 2009-2014 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2014, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#include "sst_config.h"
#include "sst/core/serialization.h"

#include <assert.h>

#include "emberengine.h"

#include "sst/core/element.h"
#include "sst/core/params.h"

#include "motifs/emberhalo2d.h"
#include "motifs/emberhalo2dNBR.h"
//#include "motifs/emberhalo3dnb.h"
#include "motifs/emberhalo3d.h"
#include "motifs/embersweep2d.h"
#include "motifs/embersweep3d.h"
#include "motifs/emberpingpong.h"
#include "motifs/emberring.h"
#include "motifs/emberfini.h"
#include "motifs/emberbarrier.h"
#include "motifs/emberallreduce.h"
#include "motifs/emberallpingpong.h"
#include "motifs/embernull.h"

using namespace SST;
using namespace SST::Ember;

static Component*
create_EmberComponent(SST::ComponentId_t id,
                  SST::Params& params)
{
    return new EmberEngine( id, params );
}

static Module*
load_PingPong( Component* comp, Params& params ) {
	return new EmberPingPongGenerator(comp, params);
}

static Module*
load_AllPingPong( Component* comp, Params& params ) {
	return new EmberAllPingPongGenerator(comp, params);
}

static Module*
load_Barrier( Component* comp, Params& params ) {
	return new EmberBarrierGenerator(comp, params);
}

static Module*
load_Ring( Component* comp, Params& params ) {
	return new EmberRingGenerator(comp, params);
}

static Module*
load_Fini( Component* comp, Params& params ) {
	return new EmberFiniGenerator(comp, params);
}

static Module*
load_Halo2D( Component* comp, Params& params ) {
	return new EmberHalo2DGenerator(comp, params);
}

static Module*
load_Halo2DNBR( Component* comp, Params& params ) {
	return new EmberHalo2DNBRGenerator(comp, params);
}

//static Module*
//load_Halo3DNB26( Component* comp, Params& params ) {
//	return new EmberHalo3DNB26Generator(comp, params);
//}

static Module*
load_Halo3D( Component* comp, Params& params ) {
	return new EmberHalo3DGenerator(comp, params);
}

static Module*
load_Sweep2D( Component* comp, Params& params ) {
	return new EmberSweep2DGenerator(comp, params);
}

static Module*
load_Sweep3D( Component* comp, Params& params ) {
	return new EmberSweep3DGenerator(comp, params);
}

static Module*
load_Allreduce( Component* comp, Params& params ) {
	return new EmberAllreduceGenerator(comp, params);
}

static Module*
load_Null( Component* comp, Params& params ) {
	return new EmberNullGenerator(comp, params);
}


static const ElementInfoParam component_params[] = {
    { "printStats", "Prints the statistics from the component", "0"},
    { "verbose", "Sets the output verbosity of the component", "0" },
    { "msgapi", "Sets the messaging API of the end point" },
    { "motif", "Sets the event generator or motif for the engine", "ember.EmberPingPongGenerator" },
    { "start_bin_width", "Bin width of the start time histogram", "5" },
    { "send_bin_width", "Bin width of the send time histogram", "5" },
    { "compute_bin_width", "Bin width of the compute time histogram", "5" },
    { "init_bin_width", "Bin width of the init time histogram", "5" },
    { "finalize_bin_width", "Bin width of the finalize time histogram", "5"},
    { "recv_bin_width", "Bin width of the recv time histogram", "5" },
    { "irecv_bin_width", "Bin width of the irecv time histogram", "5" },
    { "wait_bin_width", "Bin width of the wait time histogram", "5" },
    { "barrier_bin_width", "Bin width of the barrier time histogram", "5" },
    { "allreduce_bin_width", "Bin width of the allreduce time histogram", "5" },
    { "buffersize", "Sets the size of the message buffer which is used to back data transmission", "32768"},
    { "jobId", "Sets the job id", "-1"},
    { "noisegen", "Sets the noise generator for the system", "constant" },
    { "noisemean", "Sets the mean of a Gaussian noise generator", "1.0" },
    { "noisestddev", "Sets the standard deviation of a noise generator", "0.1" },
    { NULL, NULL, NULL }
};

static const ElementInfoPort component_ports[] = {
    {"nic", "Port connected to the nic", NULL},
    {"loop", "Port connected to the loopBack", NULL},
    {NULL, NULL, NULL}
};

static const ElementInfoParam pingpong_params[] = {
    	{	"messagesize",		"Sets the message size of the ping pong operation",	"1024"},
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"1"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam allpingpong_params[] = {
    	{	"messagesize",		"Sets the message size of the ping pong operation",	"128"},
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"1024"},
	{	"computetime",		"Sets the time spent computing some values",	 	"1000"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam barrier_params[] = {
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"1024"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam allreduce_params[] = {
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"1"},
	{	"count",		"Sets the number of elements to reduce",	 	"1"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam halo1d_params[] = {
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"10"},
	{	"computenano",		"Sets the number of nanoseconds to compute for", 	"1000"},
	{	"messagesize",		"Sets the message size (in bytes)",		 	"128"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam halo2d_params[] = {
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"10"},
	{	"computenano",		"Sets the number of nanoseconds to compute for", 	"10"},
	{	"messagesizex",		"Sets the message size in X-dimension (in bytes)",	"128"},
	{	"messagesizey",		"Sets the message size in Y-dimension (in bytes)",	"128"},
	{	"computecopy",		"Sets the time spent copying data between messages",	"5"},
	{	"sizex",		"Sets the processor decomposition in Y", "0"},
	{	"sizey",		"Sets the processor decomposition in X", "0"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam halo3d_params[] = {
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"10"},
	{	"computetime",		"Sets the number of nanoseconds to compute for", 	"10"},
	{	"flopspercell",		"Sets the number of number of floating point operations per cell, default is 26 (27 point stencil)", 	"26"},
	{	"peflops",		"Sets the FLOP/s rate of the processor (used to calculate compute time if not supplied, default is 10000000000 FLOP/s)", "10000000000"},
	{	"nx",			"Sets the problem size in X-dimension",			"100"},
	{	"ny",			"Sets the problem size in Y-dimension",			"100"},
	{	"nz",			"Sets the problem size in Z-dimension",			"100"},
	{	"pex",			"Sets the processors in X-dimension (0=auto)",		"0"},
	{	"pey",			"Sets the processors in Y-dimension (0=auto)",		"0"},
	{	"pez",			"Sets the processors in Z-dimension (0=auto)",		"0"},
	{	"copytime",		"Sets the time spent copying data between messages",	"5"},
	{	"doreduce",		"How often to do reduces, 1 = each iteration",		"1"},
	{	"fields_per_cell",	"Specify how many variables are being computed per cell (this is one of the dimensions in message size. Default is 1", "1"},
	{	"datatype_width",	"Specify the size of a single variable, single grid point, typically 8 for double, 4 for float, default is 8 (double). This scales message size to ensure byte count is correct.", "8"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam ring_params[] = {
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"1"},
	{	"messagesize",		"Sets the size of the message in bytes",	 	"1024"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam sweep3d_params[] = {
	{	"iterations",		"Sets the number of ping pong operations to perform", 	"1"},
	{	"pex",			"Sets the processor array size in X-dimension, 0 means auto-calculate",	"0"},
	{	"pey",			"Sets the processor array size in Y-dimension, 0 means auto-calculate",	"0"},
	{	"nx",			"Sets the problem size in the X-dimension",	"50"},
	{	"ny",			"Sets the problem size in the Y-dimension",	"50"},
	{	"nz",			"Sets the problem size in the Z-dimension",	"50"},
	{	"kba",			"Sets the KBA (Nz-K blocking factor, default is 1 (no blocking))", "1"},
	{	"computetime",		"Sets the compute time per KBA-data block in nanoseconds", "1000"},
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam fini_params[] = {
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoParam nullmotif_params[] = {
	{	NULL,	NULL,	NULL	}
};

static const ElementInfoModule modules[] = {
    { 	"PingPongMotif",
	"Performs a Ping-Pong Motif",
	NULL,
	NULL,
	load_PingPong,
	pingpong_params,
    	"SST::Ember::EmberGenerator"
    },
    { 	"RingMotif",
	"Performs a Ring Motif",
	NULL,
	NULL,
	load_Ring,
	ring_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"BarrierMotif",
	"Performs a Barrier Motif",
	NULL,
	NULL,
	load_Barrier,
	barrier_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"AllPingPongMotif",
	"Performs a All Ping Pong Motif",
	NULL,
	NULL,
	load_AllPingPong,
	allpingpong_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"Halo2DMotif",
	"Performs a 2D halo exchange Motif",
	NULL,
	NULL,
	load_Halo2D,
	halo2d_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"Halo2DNBRMotif",
	"Performs a 2D halo exchange Motif with non-blocking receives",
	NULL,
	NULL,
	load_Halo2DNBR,
	NULL,
    "SST::Ember::EmberGenerator"
    },
//    { 	"Halo3DNB26Motif",
//	"Performs a 3D halo exchange Motif with non-blocking receives, over 26-communication neighbors",
//	NULL,
//	NULL,
//	load_Halo3DNB26,
//	NULL,
//    "SST::Ember::EmberGenerator"
//    },
    { 	"Halo3DMotif",
	"Performs a 3D blocking motif",
	NULL,
	NULL,
	load_Halo3D,
	halo3d_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"Sweep2DMotif",
	"Performs a 2D sweep exchange Motif with multiple vertex communication ordering",
	NULL,
	NULL,
	load_Sweep2D,
	NULL,
    "SST::Ember::EmberGenerator"
    },
    { 	"Sweep3DMotif",
	"Performs a 3D sweep communication motif from all 8 vertices",
	NULL,
	NULL,
	load_Sweep3D,
	sweep3d_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"AllreduceMotif",
	"Performs a Allreduce operation with type set to float64 and operation SUM",
	NULL,
	NULL,
	load_Allreduce,
	allreduce_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"FiniMotif",
	"Performs a communication finalize Motif",
	NULL,
	NULL,
	load_Fini,
	fini_params,
    "SST::Ember::EmberGenerator"
    },
    { 	"NullMotif",
	"Performs an idle on the node, no traffic can be generated.",
	NULL,
	NULL,
	load_Null,
	nullmotif_params,
    "SST::Ember::EmberGenerator"
    },
    {   NULL, NULL, NULL, NULL, NULL, NULL, NULL  }
};

static const ElementInfoComponent components[] = {
    { "EmberEngine",
      "Base communicator motif engine.",
      NULL,
      create_EmberComponent,
      component_params,
      component_ports
    },
    { NULL, NULL, NULL, NULL }
};

extern "C" {
    ElementLibraryInfo ember_eli = {
        "Ember",
        "Message Pattern Generation Library",
        components,
	NULL, 		// Events
	NULL,		// Introspector
	modules,
	NULL,
	NULL
    };
}
