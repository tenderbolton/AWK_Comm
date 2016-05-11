#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    this->portReceiveClients = 6001;
    this->portReceiveBroadcast = 6000;
    this->portOutputClients = 6001;
    this->maxClientAge = 15;
    
    this->textRenderer = new ofTrueTypeFont();
    this->textRendererSmall = new ofTrueTypeFont();
    this->textRenderer->loadFont("verdana.ttf", 12);
    this->textRendererSmall->loadFont("verdana.ttf", 8);
    
    this->logo = new ofImage();
    logo->loadImage("AWKLogo.png");
    
    loadOK = false;
    
    loadFromXML();
    
    if(loadOK){
        oscRecieverClients.setup(portReceiveClients);
        broadcaster.setup(portReceiveBroadcast);
    }

}

void ofApp::loadFromXML(){
    
    loadOK = true;
    
    if( XML.loadFile("appSettings.xml") ){
        
        int numMainSettingsTag = XML.getNumTags("MAIN_SETTINGS");
        
        if(numMainSettingsTag==1){
            XML.pushTag("MAIN_SETTINGS");
            
            int numInputTag = XML.getNumTags("OSC_INPUT_CLIENTS");
            if(numInputTag>0){
                this->portReceiveClients = ofToInt(XML.getAttribute("OSC_INPUT_CLIENTS","port","6001"));
            }
            else{
                cout << "No osc input clients defined!" << endl;
                loadOK = false;
                throw std::exception();
            }
            
            int numInputBroadcastTag = XML.getNumTags("OSC_INPUT_BROADCAST");
            if(numInputBroadcastTag>0){
                this->portReceiveBroadcast = ofToInt(XML.getAttribute("OSC_INPUT_BROADCAST","port","6000"));
            }
            else{
                cout << "No osc input boadcasts defined!" << endl;
                loadOK = false;
                throw std::exception();
            }
            
            int numOutputTag = XML.getNumTags("OSC_OUTPUT_CLIENTS");
            if(numOutputTag>0){
                this->portOutputClients = ofToInt(XML.getAttribute("OSC_OUTPUT_CLIENTS","port","6001"));
            }
            else{
                cout << "No osc output clients defined!" << endl;
                loadOK = false;
                throw std::exception();
            }
            
            int numClientsTag = XML.getNumTags("CLIENTS");
            if(numOutputTag>0){
                this->maxClientAge = ofToInt(XML.getAttribute("CLIENTS","maxAge","15"));
            }
            else{
                cout << "No clients settings defined!" << endl;
                loadOK = false;
                throw std::exception();
            }


            XML.popTag();
        }
        else{
            cout << "No main settings defined!" << endl;
            loadOK = false;
            throw std::exception();
        }
    }
    else{
        cout << "File could not be loaded!" << endl;
        loadOK = false;
        throw std::exception();
    }
    
}


//--------------------------------------------------------------
void ofApp::update(){
    receiveHeartBeats();
    receiveBroadcasts();
    sendDataToClients();
    killDeadClients();
}

//--------------------------------------------------------------
void ofApp::draw(){
    float spacing = 100.0f;
    float initialYTitle = 50.0f;
    float initialY = 170.0f;
    float initialX = 50.0f;
    float lineSpacing = 20.0f;
    float columnSpacing = 60.0f;
    
    float bottomY = 360.0f;
    float bottomX = 50.0f;
    
    float maxItemsPerCol = 20.0f;
    
    ofSetColor(0);
    
    this->textRenderer->drawString("/// Awkward Consequence - OSC CommManager ///", initialX, initialYTitle);
    this->textRendererSmall->drawString("This application receives OSC messages to be croadcasted on UDP port: " + ofToString(this->portReceiveBroadcast), initialX, initialYTitle + 2 * lineSpacing );
    this->textRendererSmall->drawString("And sends them to the registered clients on UDP port: " + ofToString(this->portOutputClients), initialX, initialYTitle + 3 * lineSpacing);
    this->textRendererSmall->drawString("Clients should send /heartbeat OSC message to port:" + ofToString(this->portReceiveClients), initialX, initialYTitle + 4 * lineSpacing);
    this->textRendererSmall->drawString("Client max age: " + ofToString(this->maxClientAge) + " seconds ", initialX, initialYTitle + 5 * lineSpacing);
    this->textRendererSmall->drawString("Registered clients: " + ofToString(this->clients.size()) , initialX, initialYTitle + 6 * lineSpacing);
    this->textRendererSmall->drawString("FPS: " + ofToString(ofGetFrameRate()) , initialX, initialYTitle + 7 * lineSpacing);
    
    if (loadOK){
        
        ofPushMatrix();
        int cont = 0;
        for( std::map<string, AWK_Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
            ofTranslate(initialX + columnSpacing*(cont/maxItemsPerCol), initialYTitle + (9+cont%(int)maxItemsPerCol) * lineSpacing);
            it->second->draw();
            cont+=1;
        }
        
        ofPopMatrix();
        
    }
    else{
        this->textRenderer->drawString("Loading error. Please check XML and run again.", initialX, initialY);
        this->textRenderer->drawString(" :( ", initialX, initialY + 2 * lineSpacing);
        
    }
    
    ofSetColor(255);
    this->logo->draw(ofGetWidth() - 120, ofGetHeight() - 120);
    ofSetColor(0);
    this->textRendererSmall->drawString("http://awkwardconsequence.com", bottomX, ofGetHeight() - 40);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::receiveHeartBeats(){
    while(oscRecieverClients.hasWaitingMessages()){
        // get the next message
        ofxOscMessage* m = new ofxOscMessage();
        oscRecieverClients.getNextMessage(m);
        
        // process messages
        
        if(m->getAddress() == "/heartbeat"){
            // arguments: extendedinfo (string)
            try {
                
                string extendedinfo = m->getArgAsString(0);
                string senderIp = m->getRemoteIp();
                
                std::map<string, AWK_Client*>::const_iterator it = clients.find(senderIp);
                
                if(it!=clients.end()){
                    //client found
                    it->second->gotHeartBeat();
                }
                else{
                    //we have a new client
                    AWK_Client* newClient = new AWK_Client();
                    newClient->setup(senderIp, extendedinfo, this->portOutputClients, this->textRendererSmall);
                    clients.insert(std::pair<string, AWK_Client*>(senderIp, newClient));
                }
                
                //deleting message
                delete m;
                
            }
            catch(std::exception const& e){
                cout << "Error while processing OSC command" << endl;
                delete m;
            }
        }
        else{
            delete m;
        }
    }
}

void ofApp::receiveBroadcasts(){
    
    while(broadcaster.hasWaitingMessages()){
        // get the next message
        ofxOscMessage* m = new ofxOscMessage();
        broadcaster.getNextMessage(m);
        
        // process messages
        
        if(m->getAddress() == "/showdata" || m->getAddress() == "/heartbeat"){
            try {
                
                for( std::map<string, AWK_Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
                   
                    it->second->addOSCInput(m);
                    
                }
                
                //deleting message
                delete m;
                
            }
            catch(std::exception const& e){
                cout << "Error while processing OSC command" << endl;
                delete m;
            }
        }
        else{
            delete m;
        }
    }
}

void ofApp::sendDataToClients(){
    for( std::map<string, AWK_Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
        it->second->sendMessages();
    }
}

void ofApp::killDeadClients(){
    
    std::vector<string> clientsToDelete;
    
    float currentTime = ofGetElapsedTimef();
    
    for( std::map<string, AWK_Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
        if(currentTime - it->second->getTimeOfLastHeartBeat()>this->maxClientAge){
            clientsToDelete.push_back(it->first);
        }
    }
    
    for(int i = 0; i<clientsToDelete.size(); i++){
        std::map<string, AWK_Client*>::const_iterator it = clients.find(clientsToDelete[i]);
        
        if(it!=clients.end()){
            AWK_Client* cli = it->second;
            delete cli;
            clients.erase(it);
        }
        
    }
    
}

