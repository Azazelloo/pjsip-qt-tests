#include "PjSipWorker.hpp"

namespace{
void makeOutgoingAudioCall(MyAccount* acc){
    // Make outgoing call
    pj::Call *call = new MyCall(*acc);
    acc->calls.push_back(call);
    pj::CallOpParam prm(true);
    prm.opt.audioCount = 1;
    prm.opt.videoCount = 0;
    call->makeCall("sip:192.168.143.182", prm);
}

void recordToWAVFile(){
    std::cout << "Start recording to WAV file in 2 seconds..." << std::endl;
    pj_thread_sleep(2000);

    pj::AudioMediaRecorder wav_writer;
    pj::AudioMedia& mic_media = pj::Endpoint::instance().audDevManager().getCaptureDevMedia();
    wav_writer.createRecorder("test_file.wav");
    mic_media.startTransmit(wav_writer);

    pj_thread_sleep(5000);
    std::cout << "Stop record to WAV file..." << std::endl;
    mic_media.stopTransmit(wav_writer);
}

void playingWAVFile(){
    pj::AudioMediaPlayer* player {new pj::AudioMediaPlayer}; //memory leak for tests
    pj::AudioMedia& speaker_media = pj::Endpoint::instance().audDevManager().getPlaybackDevMedia();
    try{
        player->createPlayer("test_file.wav", PJMEDIA_FILE_NO_LOOP);
        player->startTransmit(speaker_media);
//        pj_thread_sleep(1000);
    } catch(pj::Error& err){
        std::cout << "Play WAV file error: " << err.info() << std::endl;
    }
}

void addBuddy(MyAccount* acc){
    pj::BuddyConfig cfg;
    cfg.uri = "sip:192.168.143.182";

    auto buddy = std::make_unique<MyBuddy>();
    try {
        buddy->create(*acc, cfg);
        buddy->subscribePresence(true);
        acc->buddys.push_back(std::move(buddy));
    } catch(pj::Error& err) {
        std::cout << "Error buddy create!" << std::endl;
    }
}
}

void PjSipWorker::startCallTests(){
    //init
    try{
        pj::EpConfig ep_cfg;
        ep_cfg.logConfig.level = 4;
        ep.libInit(ep_cfg);
    } catch(pj::Error& err) {
        std::cout << "Initialization error: " << err.info() << std::endl;
    }

    //сетевые настройки
    pj::TransportConfig tcfg;
    tcfg.port = 5060;
    ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

    // Start library
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

    // Add account
    pj::AccountConfig acc_cfg;
    acc_cfg.idUri = "sip:192.168.143.181";

    acc = std::make_unique<MyAccount>();
    try {
        acc->create(acc_cfg);
    } catch (...) {
        std::cout << "Adding account failed" << std::endl;
    }

    pj_thread_sleep(2000);

    int test = 2; //0 - audio call; 1 - record to WAV; 2 - presense and instant messaging;

    switch(test){
    case 0:
        makeOutgoingAudioCall(acc.get());
        break;
    case 1:
        recordToWAVFile();
        playingWAVFile();
        break;
    case 2:
        addBuddy(acc.get());
        break;
    default:
        std::cout << "Invalid test number!" << std::endl;
    }
}

void MyCall::onCallState(pj::OnCallStateParam &prm)
{
    PJ_UNUSED_ARG(prm);

    pj::CallInfo ci = getInfo();
    std::cout << "*** Call: " <<  ci.remoteUri << " [" << ci.stateText
              << "]" << std::endl;

    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        //myAcc->removeCall(this);
        /* Delete the call */
        //delete this;
    }
}

void MyCall::onCallMediaState(pj::OnCallMediaStateParam &prm)
{
    pj::CallInfo ci = getInfo();

    for (unsigned i = 0; i < ci.media.size(); i++) {
        if (ci.media[i].type==PJMEDIA_TYPE_AUDIO && getMedia(i)) {
            pj::AudioMedia *aud_med = static_cast<pj::AudioMedia*>(getMedia(i));

            // Connect the call audio media to sound device
            pj::AudDevManager& mgr = pj::Endpoint::instance().audDevManager();
            aud_med->startTransmit(mgr.getPlaybackDevMedia());
            mgr.getCaptureDevMedia().startTransmit(*aud_med);
        }
    }
}

void MyCall::onCallTransferRequest(pj::OnCallTransferRequestParam &prm)
{
    /* Create new Call for call transfer */
    prm.newCall = new MyCall(*myAcc);
}

void MyCall::onCallReplaceRequest(pj::OnCallReplaceRequestParam &prm)
{
    /* Create new Call for call replace */
    prm.newCall = new MyCall(*myAcc);
}
