#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "AWK_Client.hpp"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void loadFromXML();
        void receiveHeartBeats();
        void receiveBroadcasts();
        void sendDataToClients();
        void killDeadClients();
        void addOSCBroadcastMessage(ofxOscMessage* m);
        void sendBroadcastMessages();
        void clearBroadcastMessages();
        void loadAllClients();
    
    private:
        ofxXmlSettings XML;
        ofTrueTypeFont* textRenderer;
        ofTrueTypeFont* textRendererSmall;
        ofImage* logo;
    
        ofxOscReceiver oscRecieverClients;
        ofxOscReceiver broadcaster;
    
        ofxOscSender	senderBroadcast;
        ofxOscSender	senderForcedBroadcast;
    
        //ofxOscSender	senderTEST;
    
        std::map<string, AWK_Client*> clients;
    
        int portReceiveClients;
        int portReceiveBroadcast;
        int portOutputClients;
    
        string broadcastIP;
    
        string forcedBroadcastPrefix;
        int forcedBroadcastLow;
        int forcedBroadcastHigh;
    
        int maxClientAge;
    
        bool loadOK;
        bool doStandardBroadcast;
        bool doRetransmission;
    
        queue<ofxOscMessage*> broadcastMessageBuffer;
    
    

};
