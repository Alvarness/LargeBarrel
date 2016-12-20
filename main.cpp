/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file main.cpp
 */

#include <DBHandler/HeaderFiles/DBHandler.h>
#include <JPetManager/JPetManager.h>
#include <JPetTaskLoader/JPetTaskLoader.h>
#include "ModuleA.h"
#include "ModuleB.h"
#include "ModuleC2.h"
#include "ModuleD.h"

using namespace std;

int main(int argc, char* argv[]) {
    DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::getManager();
  manager.parseCmdLine(argc, argv);

  // Here create all analysis modules to be used:

  manager.registerTask([](){
      return new JPetTaskLoader("hld", "tslot_raw",
                  new ModuleA("Module: Unp to TSlot Raw",
                        "Process unpacked HLD file into a tree of JPetTSlot objects"));
    });

  manager.registerTask([](){
      return new JPetTaskLoader("tslot_raw", "raw_sig",
                new ModuleB("Module: assemble signals",
                       "Assemble raw PMT signals from time window data"));
    });


  /* TO DO: add modules to create more sophisticated signals type */

  // manager.registerTask([](){
  //     return new JPetTaskLoader("raw_sig", "reco_sig",
  //               new ModuleC1("Module: reconstruct signals",
  //                      "Create reconstructed signals based on raw signals points"));
  //   });



  /*=====  WARNING  ======*/
  /* Module below is olny tempoary. For now it only sets time of signal arrival
   * as time at first treshold of JPetRawSignal */

  manager.registerTask([](){
      return new JPetTaskLoader("raw_sig" /* will be changed to reco_sig */ , "phys_sig",
                new ModuleC2("Module: assemble physical signals",
                       "Add physical properties of signals based on reconstructed signals"));
    });


  manager.registerTask([](){
      return new JPetTaskLoader("phys_sig", "hits",
                new ModuleD("Module: create signals",
                       "Create hits from physical signals from both ends of scintilators"));
    });


  manager.run();
}
