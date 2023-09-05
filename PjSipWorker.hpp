#pragma once

#include <memory>
#include <QDebug>

#include <pjsua2.hpp>
#include <iostream>
#include <pj/file_access.h>

class MyEndpoint : public pj::Endpoint
{
public:
    MyEndpoint() : Endpoint() {};
    virtual pj_status_t onCredAuth(pj::OnCredAuthParam &prm)
    {
        PJ_UNUSED_ARG(prm);
        std::cout << "*** Callback onCredAuth called ***" << std::endl;
        /* Return PJ_ENOTSUP to use
         * pjsip_auth_create_aka_response()/<b>libmilenage</b> (default),
         * if PJSIP_HAS_DIGEST_AKA_AUTH is defined.
         */
        return PJ_ENOTSUP;
    }
};

class MyAccount;

class MyAudioMediaPort: public pj::AudioMediaPort
{
    virtual void onFrameRequested(pj::MediaFrame &frame)
    {
        // Give the input frame here
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        // frame.buf.assign(frame.size, 'c');
    }

    virtual void onFrameReceived(pj::MediaFrame &frame)
    {
        // Process the incoming frame here
    }
};

class MyCall : public pj::Call
{
private:
    MyAccount *myAcc;
    pj::AudioMediaPlayer *wav_player;
    pj::AudioMediaPort *med_port;

public:
    MyCall(pj::Account &acc, int call_id = PJSUA_INVALID_ID)
        : pj::Call(acc, call_id)
    {
        wav_player = NULL;
        med_port = NULL;
        myAcc = (MyAccount *)&acc;
    }

    ~MyCall()
    {
        if (wav_player)
            delete wav_player;
        if (med_port)
            delete med_port;
    }

    virtual void onCallState(pj::OnCallStateParam &prm);
    virtual void onCallTransferRequest(pj::OnCallTransferRequestParam &prm);
    virtual void onCallReplaceRequest(pj::OnCallReplaceRequestParam &prm);
    virtual void onCallMediaState(pj::OnCallMediaStateParam &prm);
};

class MyBuddy : public pj::Buddy
{
public:
    MyBuddy() = default;
    ~MyBuddy() = default;

    virtual void onBuddyState()
    {
        pj::BuddyInfo bi = getInfo();
        std::cout << "Buddy " << bi.uri << " is " << bi.presStatus.statusText << std::endl;
    }

    virtual void onBuddyEvSubState(pj::OnBuddyEvSubStateParam &prm){
        pj::BuddyInfo bi = getInfo();
        std::cout << "Subscribe state: " << bi.subStateName << " Subscribe code: " << bi.subTermCode
                  << " Subscribe term reason: " << bi.subTermReason << std::endl;
    }
};

class MyAccount : public pj::Account
{
public:
    std::vector<pj::Call*> calls;
    std::vector<std::unique_ptr<MyBuddy>> buddys;

public:
    MyAccount()
    {}

    ~MyAccount()
    {
        std::cout << "*** Account is being deleted: No of calls="
                  << calls.size() << std::endl;

        for (std::vector<pj::Call*>::iterator it = calls.begin();
             it != calls.end(); )
        {
            delete (*it);
            it = calls.erase(it);
        }

        buddys.clear();
    }

    void removeCall(pj::Call *call)
    {
        for (std::vector<pj::Call*>::iterator it = calls.begin();
             it != calls.end(); ++it)
        {
            if (*it == call) {
                calls.erase(it);
                break;
            }
        }
    }

    virtual void onRegState(pj::OnRegStateParam &prm)
    {
        pj::AccountInfo ai = getInfo();
        std::cout << (ai.regIsActive? "*** Register: code=" : "*** Unregister: code=")
                  << prm.code << std::endl;
    }

    virtual void onIncomingCall(pj::OnIncomingCallParam &iprm)
    {
        pj::Call *call = new MyCall(*this, iprm.callId);
        pj::CallInfo ci = call->getInfo();
        pj::CallOpParam prm;

        std::cout << "*** Incoming Call: " <<  ci.remoteUri << " ["
                  << ci.stateText << "]" << std::endl;

        calls.push_back(call);
        prm.statusCode = (pjsip_status_code)200;
        call->answer(prm);
    }
};

class PjSipWorker{
public:
    PjSipWorker(){
        qDebug() << "PjSipWorker constructor!";

        try {
            ep.libCreate();
        }
        catch (pj::Error& err){
            std::cout << "Exception: " << err.info() << std::endl;
        }
    }

    ~PjSipWorker(){
        qDebug() << "PjSipWorker destructor!";

        ep.hangupAllCalls();
        pj_thread_sleep(4000);

        try {
            ep.libDestroy();
        }
        catch(pj::Error &err) {
            std::cout << "Exception: " << err.info() << std::endl;
        }
    }

    bool libGetState(){
        return ep.libGetState() == PJSUA_STATE_CREATED;
    }

    void startCallTests();
private:
    MyEndpoint ep;
    std::unique_ptr<MyAccount> acc;
};
