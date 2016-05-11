//
//  AWK_Client.cpp
//  AWK_Comm
//
//  Created by Christian Clark on 5/11/16.
//
//

#include "AWK_Client.hpp"


AWK_Client::AWK_Client(){
    this->ip = "";
    this->extrainfo = "";
    this->port = 0;
    this->setupCompleted = false;
    this->lastProcessedCommand = NULL;
    this->timeSinceLastHeartBeat = ofGetElapsedTimef();
}

AWK_Client::~AWK_Client(){
    while(messageBuffer.size()>0){
        ofxOscMessage* newMessage = this->messageBuffer.front();
        delete newMessage;
        this->messageBuffer.pop();
    }
}

void AWK_Client::setup(string _ip, string _extrainfo, int _port,ofTrueTypeFont* _textRenderer){
    this->ip = _ip;
    this->extrainfo = _extrainfo;
    this->port = _port;
    this->textRenderer = _textRenderer;
    
    sender.setup(ip, port);
    
    this->setupCompleted = true;
}

void AWK_Client::shutDown(){
    //seems we dont have to do anything here.
}

void AWK_Client::draw(){
    this->textRenderer->drawString(this->ip + " - Age: " + ofToString(ofGetElapsedTimef()-this->timeSinceLastHeartBeat).substr(0,4) , 0, 0);
}

void AWK_Client::addOSCInput(ofxOscMessage* m){
    
    ofxOscMessage * mCopy = new ofxOscMessage(*(m));
    this->messageBuffer.push(mCopy);
    
}

void AWK_Client::sendMessages(){
    //we send all available messages
    while(messageBuffer.size()>0){
        ofxOscMessage* newMessage = this->messageBuffer.front();
        
        sender.sendMessage(*(newMessage));
        
        delete newMessage;
        this->messageBuffer.pop();
    }

}

void AWK_Client::gotHeartBeat(){
    this->timeSinceLastHeartBeat = ofGetElapsedTimef();
}

bool AWK_Client::isInitialized(){
    return this->setupCompleted;
}

float AWK_Client::getTimeOfLastHeartBeat(){
    return this->timeSinceLastHeartBeat;
}


