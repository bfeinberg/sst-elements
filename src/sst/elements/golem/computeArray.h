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

#ifndef _COMPUTEARRAY_H
#define _COMPUTEARRAY_H

#include <sst/core/subcomponent.h>
#include <sst/core/link.h>

#include <sst/core/component.h>
#include <sst/core/link.h>

#include "arrayEvent.h"

namespace SST {
namespace Golem {

class ComputeArray : public SST::SubComponent {
public:
    SST_ELI_REGISTER_SUBCOMPONENT_API(SST::Golem::ComputeArray,
        TimeConverter *,
        Event::HandlerBase*, 
        std::vector<std::vector<uint64_t>>*, 
        std::vector<std::vector<uint64_t>>*
    )

    SST_ELI_DOCUMENT_PARAMS(
        { "verbose", "Verbosity of outputs", "1"},
     )

    ComputeArray(ComponentId_t id, Params& params,
        TimeConverter * tc,
        Event::HandlerBase * handler,
        std::vector<std::vector<uint64_t>>* ins,
        std::vector<std::vector<uint64_t>>* outs) : SubComponent(id) {

        out.init("", params.find<int>("verbose", 1), 0, Output::STDOUT);

        //Derive from array inputs
        //Not error checking here because they should have been checked in the tile constructor
        numArrays = ins->size();
        arrayInSize = ins[0].size();
        arrayOutSize = outs[0].size();

        out.verbose(CALL_INFO, 1, 0, "%s: numArrays: %d, arrayInSize: %d, arrayOutSize: %d \n",
            getName().c_str(), numArrays, arrayInSize, arrayOutSize);

        tileHandler = handler;

        inVecs = ins;
        outVecs = outs;
        // std::vector<std::vector<uint64_t>> inVecs;
        // std::vector<std::vector<uint64_t>> outVecs;

        selfLink = configureSelfLink("Self", tc, new Event::Handler<ComputeArray>(this, &ComputeArray::handleSelfEvent));
    }


    virtual void beginComputation(uint32_t arrayID) {

        //latency should be in units of the component TimeBase
        SimTime_t latency = getArrayLatency(arrayID);
        ArrayEvent* ev = new ArrayEvent(arrayID);
        selfLink->send(latency, ev);
    }

    virtual void handleSelfEvent(Event* ev) {
        ArrayEvent* aev = static_cast<ArrayEvent*>(ev);
        uint32_t arrayID = aev->getArrayID();

        compute(arrayID);

        (*tileHandler)(ev);
    }


    virtual void init(unsigned int phase) = 0;
    virtual void setup() = 0;
    virtual void finish() = 0;
    virtual void emergencyShutdown() = 0;
    
    //compute should perform the actual computation using the data in inVecs[arrayID]
    //And placing the result in outVecs[arrayID]
    virtual void compute(uint32_t arrayID) = 0;
    //getArrayLatency should return a number of cycles in the component TimeBase for the array latency
    virtual SimTime_t getArrayLatency(uint32_t arrayID) = 0;



protected:
    SST::Output out;

    uint32_t numArrays; //Number of arrays in this tile
    uint32_t arrayInSize; //Size of arrayIn buffer
    uint32_t arrayOutSize; //Size of arrayOut buffer

    std::vector<std::vector<uint64_t>>* inVecs;
    std::vector<std::vector<uint64_t>>* outVecs;
    // std::vector<std::vector<uint64_t>> inVecs;
    // std::vector<std::vector<uint64_t>> outVecs;

    SST::Link * selfLink; // self link for delay events
    SST::Event::HandlerBase * tileHandler; //Event handler to call in the tile

};


}
}
#endif /* _COMPUTEARRAY_H */
