// Copyright 2023 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2023, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _TESTCOMPUTEARRAY_H
#define _TESTCOMPUTEARRAY_H

#include <sst/core/component.h>

#include <vector>

namespace SST {
namespace Golem {

class TestComputeArray : public ComputeArray {
public:
	
    SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(TestComputeArray, "golem", "TestComputeArray",
                                      SST_ELI_ELEMENT_VERSION(1, 0, 0),
                                      "Implements a simple compute array that adds the array ID to each input",
                                      SST::Golem::ComputeArray)

    SST_ELI_DOCUMENT_PARAMS(
        {"arrayLatency",       "Latency of array compution, include all data conversion (ADC, DAC) latencies", "100ns" },
     )

    TestComputeArray(ComponentId_t id, Params& params, 
        TimeConverter * tc,
        Event::HandlerBase * handler,
        std::vector<std::vector<uint64_t>>* ins,
        std::vector<std::vector<uint64_t>>* outs) : ComputeArray(id, params, tc, handler, ins, outs) {
        
        //All operations have the same latency so just set it here
        //Because of the fixed latency just reset the TimeBase here from the TimeBase of parent component in genericArray
        arrayLatency = params.find<UnitAlgebra>("arrayLatency", "100ns");
        latencyTC = getTimeConverter(arrayLatency);
        selfLink->setDefaultTimeBase(latencyTC);
    }

    virtual void init(unsigned int phase) override {}
    virtual void setup() override {}
    virtual void finish() override {}
    virtual void emergencyShutdown() override {}

    virtual void compute(uint32_t arrayID) override {
        //Assuming square arrays for simplicity
        for (int i = arrayID * arrayInSize; i < (arrayID+1) * arrayInSize; ++i) {
            (*outVecs)[arrayID][i] = (*inVecs)[arrayID][i] + arrayID;
        }
    }

    //Since we set the timebase in the constructor the latency is just 1 timebase
    virtual SimTime_t getArrayLatency(uint32_t arrayID) {
        return 1;
    }

protected:
    UnitAlgebra arrayLatency; //Latency of array operation including array conversion times
    TimeConverter* latencyTC; //TimeConveter corrosponding to the above

};


}
}
#endif /* _TESTCOMPUTEARRAY_H */
