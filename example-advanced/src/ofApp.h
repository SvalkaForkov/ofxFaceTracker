#pragma once

#include "ofMain.h"
#include "ofxCv.h"
using namespace ofxCv;
using namespace cv;

#include "ofxFaceTracker.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	ofVideoGrabber cam;
	ofxFaceTracker tracker;
	glm::vec2 position;
	float scale;
	glm::vec3 orientation;
	glm::mat4 rotationMatrix;
	
	Mat translation, rotation;
	glm::mat4 pose;
};
