#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(false);
    ofSetFrameRate(200);
    
    this->portReceiveClients = 6001;
    this->portReceiveBroadcast = 6000;
    this->portOutputClients = 6001;
    this->maxClientAge = 15;
    this->broadcastIP = "255.255.255.255";
    
    this->forcedBroadcastPrefix = "192.168.2.";
    this->forcedBroadcastLow = 1;
    this->forcedBroadcastHigh = 254;
    
    this->doStandardBroadcast=false;
    this->doRetransmission=false;
    
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
        senderBroadcast.setup(this->broadcastIP, this->portOutputClients);
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
            
            int numBroadcastTag = XML.getNumTags("BROADCASTIP");
            if(numBroadcastTag>0){
                this->broadcastIP = XML.getAttribute("BROADCASTIP","ip","255.255.255.255");
            }
            else{
                cout << "No BroadcastIP settings defined!" << endl;
                loadOK = false;
                throw std::exception();
            }
            
            int numForcedBroadcastTag = XML.getNumTags("FORCED_BROADCAST");
            if(numForcedBroadcastTag>0){
                this->forcedBroadcastPrefix = XML.getAttribute("FORCED_BROADCAST","ipPrefix","192.168.2.");
                this->forcedBroadcastLow = ofToInt(XML.getAttribute("FORCED_BROADCAST","minPostfix","1"));
                this->forcedBroadcastHigh = ofToInt(XML.getAttribute("FORCED_BROADCAST","maxPostfix","254"));
            }
            else{
                cout << "No BroadcastIP settings defined!" << endl;
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
    
    ofSetColor(0);
    
    float spacing = 100.0f;
    float initialYTitle = 50.0f;
    float initialY = 170.0f;
    float initialX = 50.0f;
    float lineSpacing = 20.0f;
    float columnSpacing = 150.0f;
    
    float bottomY = 360.0f;
    float bottomX = 50.0f;
    
    float maxItemsPerCol = 20.0f;
    
    float initialXClientList = 50.0f;
    float initialYClientList = 250.0f;
    
    ofSetColor(0);
    
    this->textRenderer->drawString("/// Awkward Consequence - OSC CommManager ///", initialX, initialYTitle);
    this->textRendererSmall->drawString("This application receives OSC messages to be croadcasted on UDP port: " + ofToString(this->portReceiveBroadcast), initialX, initialYTitle + 2 * lineSpacing );
    this->textRendererSmall->drawString("And sends them to the registered clients on UDP port: " + ofToString(this->portOutputClients), initialX, initialYTitle + 3 * lineSpacing);
    this->textRendererSmall->drawString("Clients should send /heartbeat OSC message to port:" + ofToString(this->portReceiveClients), initialX, initialYTitle + 4 * lineSpacing);
    this->textRendererSmall->drawString("Client max age: " + ofToString(this->maxClientAge) + " seconds ", initialX, initialYTitle + 5 * lineSpacing);
    this->textRendererSmall->drawString("Registered clients: " + ofToString(this->clients.size()) , initialX, initialYTitle + 6 * lineSpacing);
    
    this->textRendererSmall->drawString("Do Retransmission: " + ofToString(this->doRetransmission) , initialX, initialYTitle + 7 * lineSpacing);
    this->textRendererSmall->drawString("Do Broadcast: " + ofToString(this->doStandardBroadcast) , initialX, initialYTitle + 8 * lineSpacing);
    
    
    this->textRendererSmall->drawString("FPS: " + ofToString(ofGetFrameRate()) , initialX, initialYTitle + 9 * lineSpacing);
    
    if (loadOK){
        
        
        int cont = 0;
        for( std::map<string, AWK_Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
            //ofLogNotice(ofToString(cont/maxItemsPerCol));
            //ofLogNotice(ofToString(initialXClientList + (columnSpacing* truncf((cont/maxItemsPerCol)))));
            ofPushMatrix();
            ofTranslate(initialXClientList + (columnSpacing* truncf((cont/maxItemsPerCol))), initialYClientList + (cont%(int)maxItemsPerCol) * lineSpacing);
            it->second->draw();
            ofPopMatrix();
            cont+=1;
        }
        
        ofSetColor(0);
    
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
    switch (key) {
        case 'b':
        {
            this->doStandardBroadcast = !doStandardBroadcast;
            break;
        }
        case 'r':
        {
            this->doRetransmission = !this->doRetransmission;
            break;
        }
        case 'l':
        {
            // loading missing clients
            
            loadAllClients();
            
            break;
        }
        case 'q':
        {
            this->maxClientAge += 1;
            
            break;
        }
        case 'a':
        {
            this->maxClientAge -= 1;
            
            break;
        }

    }

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
                
                if(doRetransmission){
                
                    for( std::map<string, AWK_Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
                       
                        it->second->addOSCInput(m);
                        
                    }
                    
                }
                
                if(doStandardBroadcast){
                    this->addOSCBroadcastMessage(m);
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
    
    if(this->doRetransmission){
        for( std::map<string, AWK_Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
            it->second->sendMessages();
        }
    }
    if(this->doStandardBroadcast){
        this->sendBroadcastMessages();
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

void ofApp::addOSCBroadcastMessage(ofxOscMessage* m){
    
    ofxOscMessage * mCopy = new ofxOscMessage(*(m));
    this->broadcastMessageBuffer.push(mCopy);
    
}

void ofApp::sendBroadcastMessages(){
    //we send all available messages
    while(broadcastMessageBuffer.size()>0){
        ofxOscMessage* newMessage = this->broadcastMessageBuffer.front();
        
        senderBroadcast.sendMessage(*(newMessage));
        
        delete newMessage;
        this->broadcastMessageBuffer.pop();
    }
    
}

void ofApp::clearBroadcastMessages(){
    //we send all available messages
    while(broadcastMessageBuffer.size()>0){
        ofxOscMessage* newMessage = this->broadcastMessageBuffer.front();
        delete newMessage;
        this->broadcastMessageBuffer.pop();
    }
    
}

void ofApp::loadAllClients(){
    
    for (int i=this->forcedBroadcastLow; i<= this->forcedBroadcastHigh; i++){
        
        string senderIp = this->forcedBroadcastPrefix + ofToString(i);
        
        //ofLogNotice(ofToString(senderIp));
        
        std::map<string, AWK_Client*>::const_iterator it = clients.find(senderIp);
        
        if(it!=clients.end()){
            //client found
            it->second->gotHeartBeat();
        }
        else{
            //we have a new client
            AWK_Client* newClient = new AWK_Client();
            newClient->setup(senderIp, "", this->portOutputClients, this->textRendererSmall);
            clients.insert(std::pair<string, AWK_Client*>(senderIp, newClient));
        }
    }

}




