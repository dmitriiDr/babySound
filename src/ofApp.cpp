#include "ofApp.h"
#include <cmath>

// sin sample wave

float calc_sin(float A, float f) {
    return A * sin(2 * M_PI * f);
}

float calc_square(float A, float f) {
    float sineWave = sin(2 * M_PI * f);
    
    if (sineWave >= 0) {
        return A;
    } else {
        return -A;
    }
}

float calc_square_F(float A, float f, int harmonic = 10) {
	float sum = 0.0f;
	for (int k = 1; k <= harmonic; k += 2) {
		sum += sin(2 * M_PI * (k) * f) / (k);
		if (k == harmonic) {
			sum += sin(2 * M_PI * (k + A) * f) / (k + A);
		}
	}
	return (4 * A / M_PI) * sum;
}

float calc_saw(float A, float f, int harmonic = 10) {
	float sum = 0.0f;
	for (int k = 1; k <= harmonic; k ++) {
		sum += pow(-1, (k)) * sin(2 * M_PI * (k) * f) / (k);
		if (k == harmonic) {
			sum += pow(-1, (k)) * sin(2 * M_PI * (k+A) * f) / (k+A);
		}
	}

	return (2 * A / M_PI) * sum;
}

float calc_saw_reverse(float A, float f, int harmonic = 10) {
	float sum = 0.0f;
	for (int k = 1; k <= harmonic; k++) {
		sum += sin(2 * M_PI * (k + A) * f) / (k + A);
	}

	return (-2 * A / M_PI) * sum;
}

float show_harmonic(int harmonic, float A) {
	return harmonic;
}

// float calcul_carre(float A, float f, float t, float brillance) {

// }

// method to create

// void ofApp::cbAudioProcess(float* outputBuffer, int bufferSize, int nChannels) {
//     for (int i = 0; i < bufferSize; i++) {

//         float sample = calc_sin(volume, freq, phase);
//         phase += (freq / (float)sampleRate) * TWO_PI;
//         if (phase > TWO_PI) {
//             phase -= TWO_PI;
//         }

//         outputBuffer[i * nChannels] = sample * volume;
//         outputBuffer[i * nChannels + 1] = sample * volume;
//     }
// }


//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(34, 34, 34);
	
	int bufferSize		= 512;
	sampleRate 			= 44100;
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.5f;
	bNoise 				= false;
	int waveType 		= 0;

	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	
	soundStream.printDeviceList();

	ofSoundStreamSettings settings;

	// if you want to set the device id to be different than the default:
	//
	//	auto devices = soundStream.getDeviceList();
	//	settings.setOutDevice(devices[3]);

	// you can also get devices for an specific api:
	//
	//	auto devices = soundStream.getDeviceList(ofSoundDevice::Api::PULSE);
	//	settings.setOutDevice(devices[0]);

	// or get the default device for an specific api:
	//
	// settings.api = ofSoundDevice::Api::PULSE;

	// or by name:
	//
	//	auto devices = soundStream.getMatchingDevices("default");
	//	if(!devices.empty()){
	//		settings.setOutDevice(devices[0]);
	//	}

#ifdef TARGET_LINUX
	// Latest linux versions default to the HDMI output
	// this usually fixes that. Also check the list of available
	// devices if sound doesn't work
	auto devices = soundStream.getMatchingDevices("default");
	if(!devices.empty()){
		settings.setOutDevice(devices[0]);
	}
#endif

	settings.setOutListener(this);
	settings.sampleRate = sampleRate;
	settings.numOutputChannels = 2;
	settings.numInputChannels = 0;
	settings.bufferSize = bufferSize;
	soundStream.setup(settings);

	// on OSX: if you want to use ofSoundPlayer together with ofSoundStream you need to synchronize buffersizes.
	// use ofFmodSetBuffersize(bufferSize) to set the buffersize in fmodx prior to loading a file.
}


//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(225);
	ofDrawBitmapString("BabySOUND", 32, 32);
	ofDrawBitmapString("press 's' to unpause the audio\npress 'e' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 150, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Left Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (unsigned int i = 0; i < lAudio.size(); i++){
				float x =  ofMap(i, 0, lAudio.size(), 0, 900, true);
				ofVertex(x, 100 -lAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	// draw the right channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 350, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Right Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (unsigned int i = 0; i < rAudio.size(); i++){
				float x =  ofMap(i, 0, rAudio.size(), 0, 900, true);
				ofVertex(x, 100 -rAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();
	
		
	ofSetColor(225);
	string reportString = "harmonic: ("+ofToString(show_harmonic(harmonic, volume), 2)+") modify with -/+ keys\nform: ("+ofToString(form, 2)+") modify with f/r\nsynthesis: ";
	if( !bNoise ){
		reportString += "sine wave (" + ofToString(freq, 2) + "hz) modify with mouse y";
	}else{
		reportString += "noise";	
	}
	ofDrawBitmapString(reportString, 32, 579);

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == '_' || key == '-') {
		harmonic = std::max(harmonic - 1, 0);
		//harmonic = MAX(volume, 0);
	}
	else if (key == '=' || key == '+') {
		harmonic = std::min(harmonic + 1, 20);
		//volume = MIN(volume, 1);
	}

	if (key == 's') {
		soundStream.start();
	}

	if (key == 'e') {
		soundStream.stop();
	}

	if (key == '1') waveType = 0;
	if (key == '2') waveType = 1;
	if (key == '3') waveType = 2;
	if (key == '4') waveType = 3;
	if (key == '5') waveType = 4;

	/*if (key == 'f') {
		fKeyHeld = true;
	}*/

	if (key == 'f') {
		form = std::min(form + 0.1f, 1.0f);
	}
	if (key == 'r') {
		form = std::max(form - 0.1f, 0.0f);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    // if (key == '1') waveType = 0;  // Sine
    // if (key == '2') waveType = 1;  // Square
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	int width = ofGetWidth();
	volume = (float)x / (float)width;
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	freq = 2000.0f * heightPct;
	phaseAdderTarget = (freq / (float) sampleRate) * TWO_PI;
}


//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	int width = ofGetWidth();
	volume = (float)x / (float)width;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	bNoise = true;
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bNoise = false;
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
void ofApp::audioOut(ofSoundBuffer& buffer) {
	pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI) {
		phase -= TWO_PI;
	}

	if (bNoise == true) {
		// ---------------------- noise --------------
		for (size_t i = 0; i < buffer.getNumFrames(); i++) {
			lAudio[i] = buffer[i * buffer.getNumChannels()] = ofRandom(0, 1) * leftScale;
			rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = ofRandom(0, 1) * rightScale;
		}
	}
	else {
		phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
		for (size_t i = 0; i < buffer.getNumFrames(); i++) {

			float sample;

			// uncomment if you want to use separate wave forms...................................

			/*if (waveType == 0) {
				sample = calc_sin(volume, phase);
			}
			else if (waveType == 1) {
				sample = calc_square(volume, phase);
			}
			else if (waveType == 2) {
				sample = calc_square_F(volume, phase, harmonic);
			}
			else if (waveType == 3) {
				sample = calc_saw(volume, phase, harmonic);
			}
			else if (waveType == 4) {
				sample = calc_saw_reverse(volume, phase, harmonic);
			}*/

			//...................................................................................

			if (form == 0.0f) {
				sample = calc_sin(volume, phase);
			}
			else if (form > 0.0f && form < 0.5f) {
				// Blend sine and square
				float square = calc_square_F(volume, phase, harmonic);
				float sine = calc_sin(volume, phase);
				sample = sine * (1.0f - form * 2.0f) + square * (form * 2.0f);
			}
			else if (form == 0.5f) {
				sample = calc_square_F(volume, phase, harmonic);
			}
			else if (form > 0.5f && form < 1.0f) {
				// Blend square and sawtooth
				float square = calc_square_F(volume, phase, harmonic);
				float saw = calc_saw(volume, phase, harmonic);
				sample = square * (2.0f * (1.0f - form)) + saw * (2.0f * (form - 0.5f));
			}
			else if (form == 1.0f) {
				sample = calc_saw(volume, phase, harmonic);
			}

			phase += phaseAdder;
			// float sample = calc_sin(volume, phase);
			// phase += (freq / (float)sampleRate) * TWO_PI;
			lAudio[i] = buffer[i * buffer.getNumChannels()] = sample * leftScale;
			rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = sample * rightScale;
		}
	}

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
