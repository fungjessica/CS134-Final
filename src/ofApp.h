#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "Particle.h"
#include "ParticleEmitter.h"

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
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p , glm::vec3 n);

		void loadThrustVbo();
		void loadExplosionVbo();
		void loadVortexRingVbo();

		ofEasyCam cam;
		ofxAssimpModelLoader moon, lander;
		ofLight light, spotlight1, spotlight2, spotlight3, spotlight4;
		Box boundingBox, landerBounds;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;


		ofxIntSlider numLevels;
		ofxPanel gui;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = true;
		bool bDisplayBBoxes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;

		// Timing
		ofxToggle timingInfo;

		// Lander variables
		glm::vec3 landerPos = glm::vec3(0, 0, 0);
		float landerRot = 0;

		glm::vec3 landerVelocity = glm::vec3(0, 0, 0);
		glm::vec3 landerAcceleration = glm::vec3(0, 0, 0);
		glm::vec3 landerForce = glm::vec3(0, 0, 0);
		float landerMass = 1.0;
		float landerDamping = .99;
		float landerAngularVelocity = 0;
		float landerAngularAcceleration = 0;
		float landerAngularForce = 0;

		// Forces
		TurbulenceForce* turbForce;
		GravityForce* gravityForce;
		ImpulseRadialForce* radialForce;
		CyclicForce* cyclicForce;

		std::set<int> keysPressed;

		float distance;

		// Particle effects
		ParticleEmitter thrustEmitter;
		ParticleEmitter explosionEmitter;
		ParticleEmitter vortexRingEmitter;

		bool thrust;
		bool explosion;
		bool vortexRing;

		// textures
		//
		ofTexture  particleTex;

		// shaders
		//
		ofVbo thrustVbo;
		ofVbo explosionVbo;
		ofVbo vortexRingVbo;
		ofShader shader;

		// Fuel
		float fuel, initialFuel, usedFuel, fuelStart;

		// Camera view state
		int view;
};
