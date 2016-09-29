#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
	ofSetVerticalSync(true);
	cam.initGrabber(640, 480);
	camTracker.setup();
	imgTracker.setup();
	
	img.load("face.jpg");
	imgTracker.update(toCv(img));
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
	}
}

void ofApp::draw() {
	ofSetColor(255);
	cam.draw(0, 0);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
	
	if(camTracker.getFound()) {
		camTracker.draw();
	}
	
	ofTranslate(0, 480);
	
	if(imgTracker.getFound()) {
		ofMesh faceMesh = imgTracker.getImageMesh();
		
		ofxDelaunay delaunay;
		
		// add main face points
		for(int i = 0; i < faceMesh.getNumVertices(); i++) {
			delaunay.addPoint(faceMesh.getVertex(i));
		}
		
		// add boundary face points
		float scaleFactor = 1.6;
		ofPolyline outline = imgTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE);
		glm::vec2 position = imgTracker.getPosition();
		for(int i = 0; i < outline.size(); i++) {
			glm::vec2 point((outline[i] - position) * scaleFactor + position);
			delaunay.addPoint(glm::vec3(point.x, point.y, 0));
		}

		// add the image corners
		int w = img.getWidth(), h = img.getHeight();
		delaunay.addPoint(glm::vec3(0, 0, 0));
		delaunay.addPoint(glm::vec3(w, 0, 0));
		delaunay.addPoint(glm::vec3(w, h, 0));
		delaunay.addPoint(glm::vec3(0, h, 0));
		
		delaunay.triangulate();
		ofMesh triangulated = delaunay.triangleMesh;
		triangulated.drawWireframe();
		
		// find mapping between triangulated mesh and original
		vector<int> delaunayToFinal(triangulated.getNumVertices(), -1);
		vector<int> finalToDelaunay;
		int reindexed = 0;
		for(int i = 0; i < faceMesh.getNumVertices(); i++) {
			float minDistance = 0;
			int best = 0;
			for(int j = 0; j < triangulated.getNumVertices(); j++) {
				float distance = glm::distance(triangulated.getVertex(j), faceMesh.getVertex(i));
				if(j == 0 || distance < minDistance) {
					minDistance = distance;
					best = j;
				}
			}
			delaunayToFinal[best] = reindexed++;
			finalToDelaunay.push_back(best);
		}
		for(int i = 0; i < delaunayToFinal.size(); i++) {
			if(delaunayToFinal[i] == -1) {
				delaunayToFinal[i] = reindexed++;
				finalToDelaunay.push_back(i);
			}
		}
		
		// construct new mesh that has tex coords, vertices, etc.
		ofMesh finalMesh;
		finalMesh.setMode(OF_PRIMITIVE_TRIANGLES);	
		for(int i = 0; i < delaunayToFinal.size(); i++) {
			int index = finalToDelaunay[i];
			glm::vec3 vrtx3 = triangulated.getVertex(index);
			finalMesh.addVertex(vrtx3);
			finalMesh.addTexCoord(glm::vec2(vrtx3.x, vrtx3.y));
		}
		for(int i = 0; i < triangulated.getNumIndices(); i++) {
			finalMesh.addIndex(delaunayToFinal[triangulated.getIndex(i)]);
		}
		
		// modify mesh
		if(camTracker.getFound()) {
			glm::vec2 imgPosition = imgTracker.getPosition();
			glm::vec2 camPosition = camTracker.getPosition();
			float imgScale = imgTracker.getScale();
			float camScale = camTracker.getScale();
			ofMesh reference = camTracker.getImageMesh();
			for(int i = 0; i < reference.getNumVertices(); i++) {
				glm::vec3 point3 = reference.getVertices()[i];
				glm::vec2 point = glm::vec2(point3.x, point3.y);
				point -= camPosition;
				point /= camScale;
				point *= imgScale;
				point += imgPosition;
				finalMesh.getVertices()[i] = glm::vec3(point.x, point.y, 0);
			}
		}
		 
		img.bind();
		finalMesh.drawFaces();
		img.unbind();
	}
}
