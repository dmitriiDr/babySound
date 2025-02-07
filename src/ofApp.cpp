#include "ofApp.h"
#include <cmath>
#include <vector>

// sin sample wave

float calc_sin(float A, float f) {
    return A * sin(f);
}

float calc_square(float A, float f) {
    float sineWave = sin(2 * f);
    
    if (sineWave >= 0) {
        return A;
    } else {
        return -A;
    }
}

float calc_square_F(float A, float f, int harmonic = 10) {
	float sum = 0.0f;
	for (int k = 1; k <= harmonic; k += 2) {
		sum += sin((k) * f) / (k);
		if (k == harmonic) {
			sum += sin((k + A) * f) / (k + A);
		}
	}
	return (4 * A / M_PI) * sum;
}

float calc_saw(float A, float f, int harmonic = 10) {
	float sum = 0.0f;
	for (int k = 1; k <= harmonic; k ++) {
		sum += pow(-1, (k)) * sin((k) * f) / (k);
		if (k == harmonic) {
			sum += pow(-1, (k)) * sin((k+A) * f) / (k+A);
		}
	}

	return (2 * A / M_PI) * sum;
}

float calc_saw_reverse(float A, float f, int harmonic = 10) {
	float sum = 0.0f;
	for (int k = 1; k <= harmonic; k++) {
		sum += sin(2 * (k + A) * f) / (k + A);
	}

	return (-2 * A / M_PI) * sum;
}

float show_harmonic(int harmonic, float A) {
	return harmonic;
}
// FILTER
void ofApp::Filter(float cutoff, float f_sampling, int Q, int typeFilter, float gainDB) {

	a.resize(3);
	b.resize(3);

	double norm;
	double K = tan(M_PI * cutoff / f_sampling);
	double V = pow(10, gainDB / 20.0);
	norm = 1 / (1.0 + K / double (Q) + K * K);


	// bq_type_lowpass

	// norm = 1 / (1 + K / Q + K * K);
	// a[0] = K * K * norm;
	// a[1] = 2 * a[0];
	// a[2] = a[0];

	// b[1] = 2 * (K * K - 1) * norm;
	// b[2] = (1 - K / Q + K * K) * norm;

	// bq_type_highpass

	if (typeFilter == 0) {

		a[0] = 1 * norm;
		a[1] = -2 * a[0];
		a[2] = a[0];
		b[1] = 2 * (K * K - 1) * norm;
		b[2] = (1 - K / Q + K * K) * norm;

	}

	// bq_type_lowpass

	else if (typeFilter == 1) {

        a[0] = K * K * norm;
        a[1] = 2 * a[0];
        a[2] = a[0];

        b[1] = 2 * (K * K - 1) * norm;
        b[2] = (1.0f - K / double (Q) + K * K) * norm;
    }

	// Band-Pass filter

	else if (typeFilter == 2) {

		a[0] = K / double(Q) * norm;
		a[1] = 0;
		a[2] = -a[0];

		b[1] = 2 * (K * K - 1) * norm;
		b[2] = (1.0f - K / double(Q) + K * K) * norm;

	}

	// Notch filter
	else if (typeFilter == 3) {

		a[0] = (1 + K * K) * norm;
		a[1] = 2 * (K * K - 1) * norm;
		a[2] = a[0];

		b[1] = a[1];
		b[2] = (1.0f - K / double(Q) + K * K) * norm;

	}

	else if (typeFilter == 4) {


	}


	// b[0] /= a[0];
	// b[1] /= a[0];
	// b[2] /= a[0];
	// a[1] /= a[0];
	// a[2] /= a[0];

	// a[0] = 1;

}


void ofApp::apply_filter(std::vector<float>& input, std::vector<float>& output, float gain, float& z1, float& z2) {
    int n = input.size();
    output.resize(n, 0.0f);

    for (int i = 0; i < n; i++) {
        float in = input[i];  // Current input sample

        // Direct Form I Transposed (Biquad) Calculation
        float out = a[0] * in + z1;
        z1 = a[1] * in - b[1] * out + z2;
        z2 = a[2] * in - b[2] * out;

        output[i] = out * gain;
    }
}


//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(34, 34, 34);
	
	bufferSize			= 1024;
	sampleRate 			= 44100; //= 44100;
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.5f;
	bNoise 				= false;
	int waveType 		= 0;
	static float z1_L = 0.0f, z2_L = 0.0f;  // Left
	static float z1_R = 0.0f, z2_R = 0.0f;  // Right

	Filter(cutoff, sampleRate, 1, typeFilter, gainDB);


	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	lAudioFiltered.assign(bufferSize, 0.0);
	rAudioFiltered.assign(bufferSize, 0.0);
	
	soundStream.printDeviceList();

	ofSoundStreamSettings settings;

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
		float minVal = -1.0f, maxVal = 1.0f;
    	float centerY = 100.0f;
					
			ofBeginShape();
			for (unsigned int i = 0; i < lAudio.size(); i++){
				float x =  ofMap(i, 0, lAudio.size(), 0, 900, true);
				// ofVertex(x, 100 -lAudio[i]*180.0f);
				float y = ofClamp(lAudio[i], minVal, maxVal) * 90.0f;  // Scale by 90 instead of 180
				ofVertex(x, centerY - y);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();


	calcul_dft(rAudio, bufferSize,  norm, myfreq, real_part, im_part);
// draw the DFT channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 350, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("DFT Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (unsigned int i = 0; i < norm.size(); i++){
				float x =  ofMap(i, 0, norm.size(), 0, 900, true);
				ofVertex(x, 100 -norm[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	string filterName;
    switch (typeFilter) {
        case 0: filterName = "High-pass"; break;
        case 1: filterName = "Low-pass"; break;
        case 2: filterName = "Band-pass"; break;
        case 3: filterName = "Notch"; break;
		case 4: filterName = "No Filter"; break;
        default: filterName = "Unknown"; break;
    }
	
	
		
	ofSetColor(225);
	string reportString = 
        "harmonic: (" + ofToString(show_harmonic(harmonic, volume), 2) + ") modify with -/+ keys\n"
        "form: (" + ofToString(form, 2) + ") modify with f/r\n"
        "synthesis: " + (bNoise ? "noise" : "wave (" + ofToString(freq, 2) + " Hz) modify with mouse y") + "\n"
        "Filter Type: " + filterName + " (Change with 'A'/'D')\n"
        "Cutoff Frequency: " + ofToString(cutoff, 2) + " Hz (Adjust with '[' and ']')";

    ofDrawBitmapString(reportString, 32, 579);


	// string reportString = "harmonic: ("+ofToString(show_harmonic(harmonic, volume), 2)+") modify with -/+ keys\nform: ("+ofToString(form, 2)+") modify with f/r\nsynthesis: ";
	// if( !bNoise ){
	// 	reportString += "sine wave (" + ofToString(freq, 2) + "hz) modify with mouse y";
	// }else{
	// 	reportString += "noise";	
	// }
	// ofDrawBitmapString(reportString, 32, 579);

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == '_' || key == '-') {
		harmonic = std::max(harmonic - 1, 0);
		//harmonic = MAX(volume, 0);
	}
	else if (key == '=' || key == '+') {
		harmonic = std::min(harmonic + 1, 50);
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

	if (key == 'f') {
		form = std::min(form + 0.1f, 1.0f);
	}
	if (key == 'r') {
		form = std::max(form - 0.1f, 0.0f);
	}

	if (key == 'd' || key == 'D') {
        typeFilter = (typeFilter + 1) % 5;  // Cycle filter types
    }
    if (key == 'a' || key == 'A') {
        typeFilter = (typeFilter - 1 + 5) % 5;  // Cycle backward
    }

	    if (key == '[') {
        cutoff = std::max(100.0f, cutoff - 100);  //  frequencies
    }
    if (key == ']') {
        cutoff = std::min(10000.0f, cutoff + 100);  // Set max limit
    }

	if (typeFilter != 4) { // Only update filter if it's not "No Filter"
        Filter(cutoff, sampleRate, 1, typeFilter, gainDB);
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

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

// --------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& buffer) {
	pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;
	float gainDB = 6.0f;

	static float prevCutoff = -1;
    static int prevFilterType = -1;
	if (typeFilter != 4 && (prevCutoff != cutoff || prevFilterType != typeFilter)) {
        Filter(cutoff, sampleRate, 1, typeFilter, gainDB);
        prevCutoff = cutoff;
        prevFilterType = typeFilter;
    }

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI) {
		phase -= TWO_PI;
	}
	while (phase < 0) {
		phase += TWO_PI;
	}
	// if (phase > TWO_PI) phase -= TWO_PI;
	// if (phase < 0) phase += TWO_PI;


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

			// if (waveType == 0) {
			// 	sample = calc_sin(volume, phase);
			// }
			// else if (waveType == 1) {
			// 	sample = calc_square(volume, phase);
			// }
			// else if (waveType == 2) {
			// 	sample = calc_square_F(volume, phase, harmonic);
			// }
			// else if (waveType == 3) {
			// 	sample = calc_saw(volume, phase, harmonic);
			// }
			// else if (waveType == 4) {
			// 	sample = calc_saw_reverse(volume, phase, harmonic);
			// }

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

			float previousSample = sample;
			sample = previousSample * 0.95 + sample * 0.05;

			phase += (freq / (float)sampleRate) * TWO_PI;


			// UNCOMMENT FOR FILTER
			lAudio[i] = sample * leftScale;
			rAudio[i] = sample * rightScale;

		}

		if (typeFilter != 4) { // Apply filtering

        apply_filter(lAudio, lAudioFiltered, pow(10, fabs(gainDB) / 20.0), z1_L, z2_L);
        apply_filter(rAudio, rAudioFiltered, pow(10, fabs(gainDB) / 20.0), z1_R, z2_R);

    	} else { // No Filter

        lAudioFiltered = lAudio;
        rAudioFiltered = rAudio;

    	}

		for (size_t i = 0; i < buffer.getNumFrames(); i++) {

			lAudio[i] = buffer[i * buffer.getNumChannels()] = std::max(-1.0f, std::min(1.0f, lAudioFiltered[i]));
			rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = std::max(-1.0f, std::min(1.0f, rAudioFiltered[i]));
		}

		// COMMENT FOR FILTER

		// lAudio[i] = buffer[i * buffer.getNumChannels()] = sample * leftScale;
		// rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = sample * rightScale;
	}

}


//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}





void ofApp::calcul_dft(vector <float> & audio, int bufferSize,  vector <float> & norm, vector <float> & myfreq, vector <float> & real_part, vector <float> & im_part){ 
	float n=bufferSize;
	float delta_norm=0;

	real_part.assign(bufferSize, 0.0);
	im_part.assign(bufferSize, 0.0);
	norm.assign(bufferSize, 0.0);
	myfreq.assign(bufferSize, 0.0);

	for (int k=0; k < n; k++){
		for (int m=0; m < n; m++){
				real_part[k]= real_part[k] + audio[m]*cos(TWO_PI* m*k/n);
				im_part[k] = im_part[k] + audio[m]*sin(TWO_PI* m*k/n);

			}
		norm[k] = sqrt(pow(real_part[k],2)+ pow(im_part[k],2));
		myfreq[k] = k/(sampleRate*n);
		if (norm[k] > delta_norm){
			delta_norm = norm[k];


		}
		
		}
	for (int k=0; k < n; k++){
		norm[k] = (norm[k]/delta_norm)-0.5;
		//cout<<norm[k]<<' ';

	} 
	//	cout<<endl<<endl<<endl;

	/*for (int m=0; m < n; m++){
		cout<<audio[m]<<' ';
	}*/
} 
