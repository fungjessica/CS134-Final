#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "Particle.h"
#include "ParticleEmitter.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void gotMessage(ofMessage msg);
	void initLightingAndMaterials();
	void savePicture();
	void toggleWireframeMode();
	void togglePointsDisplay();
	void toggleSelectTerrain();
	void setCameraTarget();
	bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point);
	bool raySelectWithOctree(ofVec3f& pointRet);
	glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p, glm::vec3 n);
	bool inSpotlight(ofLight& spotlight, glm::vec3 landerPos, float angle, float radius);

	void setupLandingRingEmitter(ParticleEmitter& emitter, glm::vec3 pos, glm::vec3 velocity);
	void loadThrustVbo();
	void loadExplosionVbo();
	void loadVortexRingVbo();
	void loadLandingRingVbo();
	void loadLandingRingVbo2();
	void loadLandingRingVbo3();
	void checkCollisionPosition(glm::vec3 landerPos);
	bool inLight;

	ofEasyCam cam;
	ofxAssimpModelLoader moon, lander;
	ofLight light, spotlight1, spotlight2, spotlight3, spotlight4, landerLight;
	Box boundingBox, landerBounds;
	Box testBox;
	vector<Box> colBoxList;
	bool bLanderSelected = false;
	Octree octree;
	TreeNode selectedNode;
	glm::vec3 mouseDownPos, mouseLastPos;
	bool bInDrag = false;

	ofImage background;
	ofSpherePrimitive sky;
	ofSoundPlayer landerBoom, thrustWhoosh;

	bool bAltKeyDown;
	bool bCtrlKeyDown;
	bool bWireframe;
	bool bDisplayPoints;
	bool bPointSelected;
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

	std::set<int> keysPressed;

	float distance;

	// Particle effects
	ParticleEmitter thrustEmitter;
	ParticleEmitter explosionEmitter;
	ParticleEmitter vortexRingEmitter;
	ParticleEmitter landingRingEmitter, landingRingEmitter2, landingRingEmitter3;

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
	ofVbo landingRingVbo;
	ofVbo landingRingVbo2;
	ofVbo landingRingVbo3;
	ofShader shader;

	// Fuel
	float fuelLevel, initialFuel, usedFuel, fuelStart;
	bool fuel;

	// Camera view state
	int view;
	bool fPov = false;
	bool tPov = false;
	bool aPov = false;
	bool cPov = false;

	// Game state
	bool gameState;
	bool gameOver;
	bool gameComplete;
	bool gameEnd;

	// Track explosion time
	float explosionStart;

	// Lander toggle
	bool bLanderLight;

	// Landing positions
	glm::vec3 landing, landing2, landing3;
	float landingRadius;

	// Track time of landing completion
	float landingStart;

	const int groupSize = 50;
	const int emitRate = 30;
	const float particleRadius = 15.0f;

	bool gameInstructions = false;
	bool startScreen = true;

	// AGL toggle
	bool bAGL = true;
};