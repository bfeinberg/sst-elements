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

#include <sst/core/sst_config.h>

#include "mmioDriver.h"
#include "golemUtil.h"

using namespace SST;
using namespace SST::Interfaces;
using namespace SST::Golem;

MMIODriver::MMIODriver(ComponentId_t id, Params& params) : Component(id), rng(id, 13)
{
    
    uint32_t z_seed = params.find<uint32_t>("rngseed", 85);
    rng.restart(z_seed, 13);

    out.init("", params.find<int>("verbose", 0), 0, Output::STDOUT);

    // Required parameters
    bool found;

    if (params.is_value_array(mmioAddrs)){
        params.find_array("mmioAddrs", mmioStartAddrs)
    } else {
        out.fatal(CALL_INFO, -1,"%s, Error: parameter 'mmioAddrs' was not provided\n",
            getName().c_str());
    }

    numTiles = mmioAddrs.size()

    if (params.is_value_array(numsArrays)){
        params.find_array("numsArrays", numsArrays)
    } else {
        out.fatal(CALL_INFO, -1,"%s, Error: parameter 'numsArrays' was not provided\n",
            getName().c_str());
    }

    if (params.is_value_array(arrayInputSizes)){
        params.find_array("arrayInputSizes", arrayInputSizes)
    } else {
        out.fatal(CALL_INFO, -1,"%s, Error: parameter 'arrayInputSizes' was not provided\n",
            getName().c_str());
    }

    if (params.is_value_array(arrayOutputSizes)){
        params.find_array("arrayOutputSizes", arrayOutputSizes)
    } else {
        out.fatal(CALL_INFO, -1,"%s, Error: parameter 'arrayOutputSizes' was not provided\n",
            getName().c_str());
    }

    if (params.is_value_array(inputOperandSizes)){
        params.find_array("inputOperandSizes", inputOperandSizes)
    } else {
        out.fatal(CALL_INFO, -1,"%s, Error: parameter 'inputOperandSizes' was not provided\n",
            getName().c_str());
    }

    if (params.is_value_array(outputOperandSizes)){
        params.find_array("outputOperandSizes", outputOperandSizes)
    } else {
        out.fatal(CALL_INFO, -1,"%s, Error: parameter 'outputOperandSizes' was not provided\n",
            getName().c_str());
    }

    if (params.is_value_array(memRequestSizes)){
        params.find_array("memRequestSizes", memRequestSizes)
    } else {
        out.fatal(CALL_INFO, -1,"%s, Error: parameter 'memRequestSizes' was not provided\n",
            getName().c_str());
    }

    //Make sure all the inputs are consistent
    if (numsArrays.size() != numTiles) {
        out.fatal(CALL_INFO, -1,"%s, Error: numsArrays length (%lu) does not match numTiles (%lu)\n",
            getName().c_str(), numsArrays.size(), numTiles);
    }

    if (arrayInputSizes.size() != numTiles) {
        out.fatal(CALL_INFO, -1,"%s, Error: arrayInputSizes length (%lu) does not match numTiles (%lu)\n",
            getName().c_str(), arrayInputSizes.size(), numTiles);
    }

    if (arrayOutputSizes.size() != numTiles) {
        out.fatal(CALL_INFO, -1,"%s, Error: arrayOutputSizes length (%lu) does not match numTiles (%lu)\n",
            getName().c_str(), arrayOutputSizes.size(), numTiles);
    }

    if (inputOperandSizes.size() != numTiles) {
        out.fatal(CALL_INFO, -1,"%s, Error: inputOperandSizes length (%lu) does not match numTiles (%lu)\n",
            getName().c_str(), inputOperandSizes.size(), numTiles);
    }

    if (outputOperandSizes.size() != numTiles) {
        out.fatal(CALL_INFO, -1,"%s, Error: outputOperandSizes length (%lu) does not match numTiles (%lu)\n",
            getName().c_str(), outputOperandSizes.size(), numTiles);
    }

    if (outputOperandSizes.size() != memRequestSizes) {
        out.fatal(CALL_INFO, -1,"%s, Error: outputOperandSizes length (%lu) does not match memRequestSizes (%lu)\n",
            getName().c_str(), outputOperandSizes.size(), memRequestSizes);
    }

    // other params
    numTests = params.find<uint64_t>("numTests", 1);

    //Set the clock
    UnitAlgebra clock = params.find<UnitAlgebra>("clock", "1GHz");
    clockHandler = new Clock::Handler<MMIODriver>(this, &MMIODriver::tick);
    clockTC = registerClock( clock, clockHandler );

    // tell the simulator not to end without us
    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

    // Set the sizes of the tile trackers vectors
    tileStates.resize(numTiles);
    tileCursors.resize(numTiles);
    testsCompleted.resize(numTiles);

    testInputs.resize(numTiles);
    testOutputs.resize(numTiles);
    for (int i = 0; i < numTiles; i++) {
        testInputs[i].resize(numsArrays[i] * arrayInputSizes[i])
        testOutputs[i].resize(numsArrays[i] * arrayOutputSizes[i])
    }

    // Set the address delimiters
    for (int i = 0; i < numTiles; i++) {
        inputDataSizes[i] = inputOperandSizes[i] * arrayInputSizes[i];
        inputTotalSizes[i] = inputDataSizes[i] * numsArrays[i];
        outputDataSizes[i] = outputOperandSizes[i] * arrayOutputSizes[i];
        outputTotalSize[i] = outputDataSizes[i] * numsArrays[i];
        inputStartAddrs[i] = mmioStartAddrs[i] + numsArrays[i];
        outputStartAddrs[i] = inputStartAddrs[i] + inputTotalSizes[i];
    }
    memory = loadUserSubComponent<StandardMem>("memory", ComponentInfo::SHARE_NONE, clockTC, new StandardMem::Handler<MMIODriver>(this, &MMIODriver::handleEvent));
    if ( !memory ) {
        out.fatal(CALL_INFO, -1, "Unable to load standardInterface subcomponent.\n");
    }

    memHandlers = new StandardMemHandlers(this, &out);

}


void MMIODriver::init(unsigned int phase) {
    memory->init(phase);

    for (int i = 0; i < numTiles; i++) {
        tileStates[i] = 1;
        tileCursors[i] = 0;
        testsCompleted[i] = 0;
    }
}

void MMIODriver::setup() {
    memory->setup();

    // initialize the test arrays
    for (int i = 0; i < numTiles; i++) {
        for (int j = 0; j < inputDataSizes[i]; j++) {
            testInputs[i][j] = rng.generateNextUInt32() % 7;
        }
    }
    //print the test setup

}

void MMIODriver::finish() {
    memory->finish();
}

void MMIODriveremergencyShutdown() {}


//Perform N tile operations
//This assumes that all arrays in a time start and finish at the time
//This isn't a requirement of the array model, simply a simplifying assumption for this test
//Different tiles can execute at different rates
bool MMIODriver::tick(Cycle_t time) {
    for (int tileID; tileID < numTiles; tileID++) {
        //issue writes of the input data
        if (tileStates[tileID] == 1) {
            int remaining = arrayInputSizes[tileID] - tileCursors[tileID];
            if (remaining > 0) {
                int bytes = std::min(memRequestSizes[tileID], remaining * inputOperandSizes[tileID]);
                std::vector<char> payload(bytes);
                for (int i = 0; i < bytes; i++) {
                    payload[i] = testInputs[tileID][i + tileCursors[tileID]];
                }

                uint64_t addr = inputStartAddrs[tileID] + tileCursors[tileID];
                bool final = (tileCursors[tileID] + bytes) > arrayInputSizes;
                out.verbose(CALL_INFO, 2, 0, "%s, T%d: Writing %dB [Data] to 0x%" PRIx64 ", Final?: %d\n", getName().c_str(), tileID, bytes, addr, final);
                
                //On the last request make the write posted and track it
                //Since we only track the last request only need tileID to associate it correctly
                StandardMem::Request* req = new Interfaces::StandardMem::Write(addr, bytes, payload, !final);
                if (final){
                    requests[req->getID()] = std::make_pair(tileID, 0);
                }
                memory->send(req);

                tileCursors[tileID] += memRequestSizes[tileID];
            } else {
                //reset cursor and transistion to the next state
                tileCursors[tileID] = 0;
                tileStates[tileID]++;
            }

        }
        
        //wait for input data to be written
        else if (tileStates[tileID] == 2) {
            //Nothing to do here, we're waiting
        }

        //issue writes of the start signals
        else if (tileStates[tileID] == 3) {
            int remaining = numsArrays[tileID] - tileCursors[tileID];
            if (remaining > 0) {
                int operands = std::min(memRequestSizes[tileID, remaining]);
                std::vector<char> payload(operands);
                for(int i = 0; i< operands; i++) {
                    //Status registers are 1B wide
                    //Start signal is any non-zero value, 1 for simplicity 
                    payload[i] = 1;
                }
                uint64_t addr = mmioStartAddrs[tileID] + tileCursors[tileID];
                bool final = memRequestSizes[tileID] > operands;
                out.verbose(CALL_INFO, 2, 0, "%s, T%d: Writing %dB [Status] to 0x%" PRIx64 ", Final?: %d\n", getName().c_str(), tileID, bytes, addr, final);

                //On the last request make the write posted and track it
                //Since we only track the last request only need tileID to associate it correctly
                StandardMem::Request* req = new Interfaces::StandardMem::Write(addr, bytes, payload, !final);
                if (final){
                    requests[req->getID()] = std::make_pair(tileID, 0);
                }

                tileCursors[tileID] += operands;

            } else {
                tileCursors[tileID] = 0;
                tileStates[tileID]++;
            }

        }
        //wait for status register values to be written
        else if (tileStates[tileID] == 4) {
            //Nothing to do here we're waiting
        }

        //poll the tile status register
        else if (tileStates[tileID] == 5) {
            //issue polling read
        }
        //wait for status register read to return
        else if (tileStates[tileID] == 6) {
            //Nothing to do here we're waiting
        }

        //issue reads of the input data
        else if (tileStates[tileID] == 7) {
            int remaining = arrayOutputSizes[tileID] - tileCursors[tileID];
            if (remaining > 0) {
                int operands = std::min(memRequestSizes[tileID] / arrayOutputSizes[tileID], remaining);
                std::vector<char> payload(operands * outputOperandSizes[tileID]);
                //build the payload
                //send request
                // print
                //add tracking
                tileCursors[tileID] += operands;

            } else {
                //reset cursor and transistion to the next state
                tileCursors[tileID] = 0;
                tileStates[tileID]++;
            }
        }

        //wait for reads to finish
        else if (tileStates[tileID] == 8) {
            //Nothing to do here, we're waiting
        }

        // output test results and reset or finish
        else if (tileStates[tileID] == 9) {
            //TODO: print outputs
            testsCompleted[tileID] ++;
            if (testsCompleted[tileID] == numTests) {
                tileStates[tileID] = 0;

                //check if everyone else has finished
                bool allDone = true;
                for (int i; i < numTiles; i++) {
                    if (tileStates[i]) {
                        allDone = false;
                        break;
                    }
                }
                if (allDone) {
                    primaryComponentOKToEndSim();
                    return true;
                }
            }
            else {
                tileStates[tileID] = 1;

                // renitialize the test array
                for (int j = 0; j < inputDataSizes[i]; j++) {
                    testInputs[tileID][j] = rng.generateNextUInt32() % 7;
                }

                //print starting new test
            }

        }
    }
    return false;

}

void MMMIODriver::handleEvent (Interfaces::StandardMem::Request *ev) {
    req->handle(memHandlers);
    delete req;
}

void MMIOTile::StandardMemHandlers::handle(StandardMem::ReadResp* resp) { }

void MMIOTile::StandardMemHandlers::handle(StandardMem::WriteResp* resp) { 
    std::unordered_map<uint64_t, std::pair<int, int>>::iterator i = driver->requests.find(response->getID());
    if (driver->requests.end() == i) {
        out->fatal(CALL_INFO, -1, "%s: Event (%" PRIx64 ") not found!\n",
            driver->getName().c_str(), resp->getID());
    }
    else {
        //Just advance the state
        int tileID = i->second->second;
        driver->tileStates[tileID]++
    }
    requests.erase(i);
}
