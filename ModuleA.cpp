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
 *  @file ModuleA.cpp
 */
#include <JPetUnpacker/Unpacker2/EventIII.h>
#include <JPetWriter/JPetWriter.h>
#include "TGraph.h"
#include "ModuleA.h"

ModuleA::ModuleA(const char * name, const char * description)
:JPetTask(name, description),fCurrEventNumber(0){}

void ModuleA::init(const JPetTaskInterface::Options& opts){
    getStatistics().createHistogram( new TH1F("ChannelsPerEvt","Channels fired in one event", 401, -0.5, 400.5) );
}

ModuleA::~ModuleA(){}

void ModuleA::exec(){
    if(auto evt = reinterpret_cast</*const*/ EventIII*const>(getEvent())){

        int ntdc = evt->GetTotalNTDCChannels();


        auto tdcHits = evt->GetTDCChannelsArray();

        JPetTimeWindow tslot;

        tslot.setIndex(fCurrEventNumber);


        for (int i = 0; i < ntdc; ++i) {
            auto tdcChannel = dynamic_cast</*const*/ TDCChannel*const>(tdcHits->At(i));
            auto tomb_number =  tdcChannel->GetChannel();
            if (tomb_number % 65 == 0) { // skip trigger signals from TRB
                continue;
            }
            JPetTOMBChannel& tomb_channel = getParamBank().getTOMBChannel(tomb_number);

            for(int j = tdcChannel->GetHitsNum()-1; j < tdcChannel->GetHitsNum(); ++j){
                JPetSigCh sigChTmpLead, sigChTmpTrail;
                sigChTmpLead.setDAQch(tomb_number);
                sigChTmpTrail.setDAQch(tomb_number);
                sigChTmpLead.setType(JPetSigCh::Leading);
                sigChTmpTrail.setType(JPetSigCh::Trailing);
                sigChTmpLead.setThresholdNumber(tomb_channel.getLocalChannelNumber());
                sigChTmpTrail.setThresholdNumber(tomb_channel.getLocalChannelNumber());
                sigChTmpLead.setPM(tomb_channel.getPM());
                sigChTmpLead.setFEB(tomb_channel.getFEB());
                sigChTmpLead.setTRB(tomb_channel.getTRB());
                sigChTmpLead.setTOMBChannel(tomb_channel);
                sigChTmpTrail.setPM(tomb_channel.getPM());
                sigChTmpTrail.setFEB(tomb_channel.getFEB());
                sigChTmpTrail.setTRB(tomb_channel.getTRB());
                sigChTmpTrail.setTOMBChannel(tomb_channel);
                sigChTmpLead.setThreshold(tomb_channel.getThreshold());
                sigChTmpTrail.setThreshold(tomb_channel.getThreshold());
                // check for empty TDC times
                if( tdcChannel->GetLeadTime(j) == -100000 )continue;
                if( tdcChannel->GetTrailTime(j) == -100000 )continue;
                // finally, set the times in ps [raw times are in ns]
                sigChTmpLead.setValue(tdcChannel->GetLeadTime(j) * 1000.);
                sigChTmpTrail.setValue(tdcChannel->GetTrailTime(j) * 1000.);
                tslot.addCh(sigChTmpLead);
                tslot.addCh(sigChTmpTrail);
            }
        }

        saveTimeWindow(tslot);
        fCurrEventNumber++;

    }
}

void ModuleA::terminate(){}

void ModuleA::saveTimeWindow( JPetTimeWindow slot){
    assert(fWriter);
    fWriter->write(slot);
}

void ModuleA::setWriter(JPetWriter* writer){
    fWriter=writer;
}

void ModuleA::setParamManager(JPetParamManager* paramManager) {
    fParamManager = paramManager;
}

const JPetParamBank& ModuleA::getParamBank()const{
    assert(fParamManager);
    return fParamManager->getParamBank();
}
