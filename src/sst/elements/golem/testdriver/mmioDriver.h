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

#ifndef _MMIODRIVER_H
#define _MMIODRIVER_H

#include <sst/core/component.h>
#include <sst/core/event.h>
#include <sst/core/interfaces/stdMem.h>
#include <sst/core/rng/marsaglia.h>

#include <vector>
#include <unordered_map>

using namespace std;

namespace SST {
namespace Golem {

class MMIODriver : public Component {

public:
/* Element Library Info */
    SST_ELI_REGISTER_COMPONENT(MMIODriver, "Golem", "MMIODriver", SST_ELI_ELEMENT_VERSION(1,0,0),
            "Test driver for MMIO Tile", COMPONENT_CATEGORY_PROCESSOR)

    SST_ELI_DOCUMENT_PARAMS(
            {"clock",              "(string) Clock frequency in Hz or period in s", "1GHz"},
            {"verbose",            "Verbosity of outputs", "0"},
            {"mmioAddrs",          "List of MMIO addresses for each tile, sets the number of tiles in the simulation."}
            {"numsArrays",          "List of numbers of distinct arrays in the the tile.",},
            {"arrayInputSize",     "List of lengths of input vector. Implies array rows."},
            {"arrayOutputSize",    "List of lengths of output vector. Implies array columns."},
            {"inputOperandSize",   "List of sizes of input operand in bytes."},
            {"outputOperandSize",  "List of sizes of output operand in bytes."},
            {"memRequestSizes",    "List of memory request sizes (in bytes) for each tile."},
            {"numTests",           "Number of full array operations to run per tile", 1},
            {"rngseed",            "Set a seed for the random generator used to create requests", "85"},

    )

    SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS( { "memory", "Interface to memory hierarchy", "SST::Interfaces::StandardMem" } )

/* Begin class definition */
    MMIODriver(ComponentId_t id, Params& params);
    void init(unsigned int phase) override;
    void setup() override;
    void finish() override;
    void emergencyShutdown() override;


    class StandardMemHandlers : public Interfaces::StandardMem::RequestHandler {
    public:
        friend class MMIODriver;

        StandardMemHandlers(MMIODriver* driver, SST::Output *out) : Interfaces::StandardMem::RequestHandler(out), driver(driver) {}
        virtual ~StandardMemHandlers() {}
        virtual void handle(Interfaces::StandardMem::ReadResp* resp) override;
        virtual void handle(Interfaces::StandardMem::WriteResp* write) override;

        MMIODriver* driver;
    };


private:
    void handleEvent( Interfaces::StandardMem::Request *ev );
    virtual bool tick( Cycle_t );

    Output out;

    int numTiles;

    std::vector<int> numsArrays;
    std::vector<int> arrayInputSizes;
    std::vector<int> arrayOutputSizes;
    std::vector<int> inputOperandSizes;
    std::vector<int> outputOperandSize;
    std::vector<int> memRequestSizes;

    std::vector<uint64_t> mmioStartAddrs;
    std::vector<uint64_t> inputStartAddrs;
    std::vector<uint64_t> outputStartAddrs;
    std::vector<uint64_t> inputDataSizes;
    std::vector<uint64_t> outputDataSizes;
    std::vector<uint64_t> inputTotalSizes;
    std::vector<uint64_t> outputTotalSizes;

    Interfaces::StandardMem *memory;
    ComputeArray * array;
    StandardMemHandlers* memHandlers;

    //tracking variables
    std::vector<char> tileStates;
    std::vector<int> tileCursors;
    std::vector<int> testsCompleted;

    std::vector<std::vector<uint64_t>> testInputs;
    std::vector<std::vector<uint64_t>> testOutputs;

    TimeConverter *clockTC;                 // Clock object
    Clock::HandlerBase *clockHandler;       // Clock handler
    SST::RNG::MarsagliaRNG rng;             // Random number generator for inputs
    std::unordered_map<uint64_t, std::pair<int, int>> requests;


};

}
}
#endif /* _MMIODRIVER_H */
