//
//  AWK_Client.hpp
//  AWK_Comm
//
//  Created by Christian Clark on 5/11/16.
//
//

#ifndef AWK_Client_hpp
#define AWK_Client_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxOsc.h"

class AWK_Client{
    
public:
    
    AWK_Client();
    ~AWK_Client();
    
    void setup(string _ip, string _extrainfo, int _port,ofTrueTypeFont* _textRenderer);
    
    void shutDown();
    
    void update();
    void draw();
    
    void addOSCInput(ofxOscMessage* m);
    void sendMessages();
    void gotHeartBeat();
    bool isInitialized();
    float getTimeOfLastHeartBeat();
    
private:
    
    ofxOscSender	sender;
    
    int port;
    
    string ip;
    string extrainfo;
    
    bool setupCompleted;
    
    queue<ofxOscMessage*> messageBuffer;
    
    ofxOscMessage* lastProcessedCommand;
    
    ofTrueTypeFont* textRenderer;
    
    float timeSinceLastHeartBeat;
    
};


#endif /* AWK_Client_hpp */
