#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		// void cbAudioProcess(float* outputBuffer, int bufferSize, int nChannels);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void audioOut(ofSoundBuffer & buffer);
		void BPF(float low_cutoff, float high_cutoff, float f_sampling, int order, std::vector<float>& a, std::vector<float>& b);
		void apply_filter(const std::vector<float> input, const std::vector<float> output, const std::vector<float> a, const std::vector<float> b);
		
		
		ofSoundStream soundStream;

		float 	pan;
		int		sampleRate;
		bool 	bNoise;
		float 	volume=0.5f;
		float   freq = 440.0f;

		vector <float> lAudio;
		vector <float> rAudio;
		
		//------------------- for the simple sine wave synthesis
		float 	targetFrequency;
		float 	phase = 0.0f;
		float 	phaseAdder;
		float 	phaseAdderTarget;
		int 	waveType = 0;
		int		harmonic = 10;
		float	form = 0.0f;
		//bool	fKeyHeld = false;

};
