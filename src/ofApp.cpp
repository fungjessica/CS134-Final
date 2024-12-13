
//--------------------------------------------------------------
//  UFO and the Grey Cheese
//
//  Student Names: Lee Rogers, Jessica Fung

#include "ofApp.h"
#include "Util.h"
#include "glm/gtx/intersect.hpp"

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup() {
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
	ofSetWindowShape(1280, 768);
	ofSetFrameRate(60);
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	// Load terrain
	moon.loadModel("geo/moonTerrain_size2.obj");
	moon.setScaleNormalization(false);

	// Load lander
	lander.loadModel("geo/ufo.obj");
	lander.setScaleNormalization(false);
	bLanderLoaded = true;
	lander.setPosition(0, 50, 0);
	landerPos = glm::vec3(0, 50, 0);

	//  Create Octree
	//
	octree.create(moon.getMesh(0), 20);

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}
	
	background.load("images/background.png");
	sky.set(1000, 100);
	sky.mapTexCoordsFromTexture(background.getTexture());

	landerBoom.load("sounds/explosion.mp3");
	landerBoom.setVolume(0.5);
	thrustWhoosh.load("sounds/thrust.mp3");
	thrustWhoosh.setVolume(0.5);
	thrustWhoosh.setLoop(true);

	// load the shader
	//
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif

	// Setup fuel
	fuelLevel = 120; // Fuel level (120 seconds, 2 minutes)
	initialFuel = 120;
	usedFuel = 0;

	// Initalize game state
	gameState = false;
	gameOver = false;

	// Set landing areas
	landing = glm::vec3(-100, 75, 30);
	landing2 = glm::vec3(110, 26, 120);
	landing3 = glm::vec3(30, 30, -146);
	landingRadius = 20;

	// Create particle forces
	turbForce = new TurbulenceForce(ofVec3f(-10, -10, -10), ofVec3f(10, 10, 10));
	gravityForce = new GravityForce(ofVec3f(0, -2, 0));
	radialForce = new ImpulseRadialForce(10);

	// Initalize emitters
	thrustEmitter.sys->addForce(turbForce);
	thrustEmitter.sys->addForce(gravityForce);
	thrustEmitter.sys->addForce(radialForce);

	thrustEmitter.setVelocity(ofVec3f(0, -10, 0));
	thrustEmitter.setEmitterType(CircularEmitter);
	thrustEmitter.setGroupSize(20);
	thrustEmitter.setRate(50);
	thrustEmitter.setRandomLife(true);
	thrustEmitter.setLifespanRange(ofVec2f(0, 0.5));
	thrustEmitter.setParticleRadius(10);
	thrustEmitter.setCircularEmitterRadius(1);

	thrust = false;

	explosionEmitter.sys->addForce(turbForce);
	explosionEmitter.sys->addForce(gravityForce);
	explosionEmitter.sys->addForce(radialForce);

	explosionEmitter.setVelocity(ofVec3f(0, 30, 0));
	explosionEmitter.setEmitterType(RadialEmitter);
	explosionEmitter.setGroupSize(500);
	explosionEmitter.setOneShot(true);
	explosionEmitter.setRandomLife(true);
	explosionEmitter.setLifespanRange(ofVec2f(2, 4));
	explosionEmitter.setParticleRadius(20);

	explosion = false;

	vortexRingEmitter.sys->addForce(turbForce);
	vortexRingEmitter.sys->addForce(gravityForce);
	vortexRingEmitter.sys->addForce(radialForce);

	vortexRingEmitter.setVelocity(ofVec3f(0, -10, 0));
	vortexRingEmitter.setEmitterType(CircularRadialEmitter);
	vortexRingEmitter.setGroupSize(20);
	vortexRingEmitter.setRate(25);
	vortexRingEmitter.setRandomLife(true);
	vortexRingEmitter.setLifespanRange(ofVec2f(0, 0.5));
	vortexRingEmitter.setParticleRadius(10);

	vortexRing = false;

	landingRingEmitter.sys->addForce(turbForce);
	landingRingEmitter.sys->addForce(gravityForce);
	landingRingEmitter.sys->addForce(radialForce);

	landingRingEmitter2.sys->addForce(turbForce);
	landingRingEmitter2.sys->addForce(gravityForce);
	landingRingEmitter2.sys->addForce(radialForce);

	landingRingEmitter3.sys->addForce(turbForce);
	landingRingEmitter3.sys->addForce(gravityForce);
	landingRingEmitter3.sys->addForce(radialForce);

	setupLandingRingEmitter(landingRingEmitter, landing, ofVec3f(0, 3, 0));
	setupLandingRingEmitter(landingRingEmitter2, landing2, ofVec3f(0, 3, 0));
	setupLandingRingEmitter(landingRingEmitter3, landing3, ofVec3f(0, 3, 0));

	landingRingEmitter.start();
	landingRingEmitter2.start();
	landingRingEmitter3.start();

	//spotlights for the 3 landing zones
	ofEnableLighting();

	//lighting for entire terrain
	spotlight1.setDiffuseColor(ofFloatColor(1, 1, 1));
	spotlight1.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0));
	spotlight1.setPosition(0, 900, 0);
	spotlight1.setSpotlight();
	spotlight1.setSpotlightCutOff(100);
	spotlight1.setSpotConcentration(5);
	spotlight1.lookAt(glm::vec3(0, 0, 0));
	spotlight1.enable();

	spotlight2.setDiffuseColor(ofFloatColor(1.0, 0.8, 0.8) * 3.0);
	spotlight2.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0) * 2.0);
	spotlight2.setPosition(-100, 100, 30);
	spotlight2.setSpotlight();
	spotlight2.setSpotlightCutOff(45);
	spotlight2.setSpotConcentration(15);
	spotlight2.lookAt(glm::vec3(-100, 0, 30)); 
	spotlight2.enable();

	spotlight3.setDiffuseColor(ofFloatColor(0.8, 1.0, 0.8) * 3.0);
	spotlight3.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0) * 2.0);
	spotlight3.setPosition(110, 60, 120);
	spotlight3.setSpotlight();
	spotlight3.setSpotlightCutOff(45);
	spotlight3.setSpotConcentration(15);
	spotlight3.lookAt(glm::vec3(110, 0, 120)); 
	spotlight3.enable();

	spotlight4.setDiffuseColor(ofFloatColor(0.8, 0.8, 1.0) * 5.0);
	spotlight4.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0) * 2.0);
	spotlight4.setPosition(30, 70, -145);
	spotlight4.setSpotlight();
	spotlight4.setSpotlightCutOff(55);
	spotlight4.setSpotConcentration(20); 
	spotlight4.lookAt(glm::vec3(30, 0, -145)); 
	spotlight4.enable();

	// Lander light
	landerLight.setDiffuseColor(ofColor::white);
	landerLight.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0) * 2.0);
	landerLight.setPosition(landerPos);
	landerLight.setSpotlight();
	landerLight.setSpotlightCutOff(55);
	landerLight.setSpotConcentration(20);
	landerLight.lookAt(glm::vec3(0, 20, 0));
	landerLight.enable();

	// Enable lander light by default
	bLanderLight = true;

	view = 1;
	tPov = true;
	glm::vec3 landerPos = lander.getPosition();
	cam.setPosition(landerPos.x, landerPos.y + 20, landerPos.z + 45);
	cam.lookAt(landerPos);
}

// Check if in spotlight
//
// Jessica Fung
//
bool ofApp::inSpotlight(ofLight& spotlight, glm::vec3 landerPos, float angle, float radius) {
	glm::vec3 spotlightPos = spotlight.getPosition();
	glm::vec3 spotlightDir = glm::normalize(spotlight.getLookAtDir());
	glm::vec3 toLander = glm::normalize(landerPos - spotlightPos);

	float distance = glm::length(glm::vec3(toLander.x, toLander.y, toLander.z));
	if (distance > radius) {
		return false;
	}
	float delta = glm::degrees(glm::acos(glm::dot(spotlightDir, glm::normalize(toLander))));
	return delta <= angle;
}

// Check collision position
//
// Jessica Fung
//
void ofApp::checkCollisionPosition(glm::vec3 landerPos) {
	float spotlightRadius = 20.0f;
	inLight = false;
	gameEnd = false;

	if (inSpotlight(spotlight2, landerPos, 45, spotlightRadius)) {
		cout << "lander in spotlight 2" << endl;
		inLight = true;
	}
	if (inSpotlight(spotlight3, landerPos, 45, spotlightRadius)) {
		cout << "lander in spotlight 3" << endl;
		inLight = true;
	}
	if (inSpotlight(spotlight4, landerPos, 45, spotlightRadius)) {
		cout << "lander in spotlight 4" << endl;
		inLight = true;
	}

	if (inLight) {
		gameComplete = true;
	} else {
		gameEnd = true;
		cout << "not in light" << endl;
	}
}

// Set up landing ring emitter with default values
//
// Jessica Fung
//
void ofApp::setupLandingRingEmitter(ParticleEmitter& emitter, glm::vec3 pos, glm::vec3 velocity) {
	velocity = glm::vec3(0, 3, 0);

	emitter.setVelocity(velocity);
	emitter.setEmitterType(CircularEmitter);
	emitter.setGroupSize(50);
	emitter.setRate(30);
	emitter.setRandomLife(true);
	emitter.setLifespanRange(ofVec2f(0, 1));
	emitter.setParticleRadius(15);
	emitter.setCircularEmitterRadius(landingRadius);
	emitter.setPosition(pos);
}

// load vertex buffer in preparation for rendering
//
// Lee Rogers
//
void ofApp::loadThrustVbo() {
	if (thrustEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < thrustEmitter.sys->particles.size(); i++) {
		points.push_back(thrustEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(thrustEmitter.particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	thrustVbo.clear();
	thrustVbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	thrustVbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// load vertex buffer in preparation for rendering
//
// Lee Rogers
//
void ofApp::loadExplosionVbo() {
	if (explosionEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < explosionEmitter.sys->particles.size(); i++) {
		points.push_back(explosionEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(explosionEmitter.particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	explosionVbo.clear();
	explosionVbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	explosionVbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// load vertex buffer in preparation for rendering
//
// Lee Rogers
//
void ofApp::loadVortexRingVbo() {
	if (vortexRingEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < vortexRingEmitter.sys->particles.size(); i++) {
		points.push_back(vortexRingEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(vortexRingEmitter.particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vortexRingVbo.clear();
	vortexRingVbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vortexRingVbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// load vertex buffer in preparation for rendering
//
// Lee Rogers
//
void ofApp::loadLandingRingVbo() {
	if (landingRingEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < landingRingEmitter.sys->particles.size(); i++) {
		points.push_back(landingRingEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(landingRingEmitter.particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	landingRingVbo.clear();
	landingRingVbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	landingRingVbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// load vertex buffer in preparation for rendering
//
// Lee Rogers
//
void ofApp::loadLandingRingVbo2() {
	if (landingRingEmitter2.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < landingRingEmitter2.sys->particles.size(); i++) {
		points.push_back(landingRingEmitter2.sys->particles[i].position);
		sizes.push_back(ofVec3f(landingRingEmitter2.particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	landingRingVbo2.clear();
	landingRingVbo2.setVertexData(&points[0], total, GL_STATIC_DRAW);
	landingRingVbo2.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// load vertex buffer in preparation for rendering
//
// Lee Rogers
//
void ofApp::loadLandingRingVbo3() {
	if (landingRingEmitter3.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < landingRingEmitter3.sys->particles.size(); i++) {
		points.push_back(landingRingEmitter3.sys->particles[i].position);
		sizes.push_back(ofVec3f(landingRingEmitter3.particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	landingRingVbo3.clear();
	landingRingVbo3.setVertexData(&points[0], total, GL_STATIC_DRAW);
	landingRingVbo3.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// Stop game
//
// Lee Rogers
//
void ofApp::stopGame() {
	gameState = false;

	// Reset lander position/rotation
	lander.setPosition(0, 50, 0);
	lander.setRotation(0, 0, 0, 1, 0);
	landerPos = glm::vec3(0, 50, 0);
	landerRot = 0;
	landerVelocity = glm::vec3(0, 0, 0);

	// Set camera
	cam.setPosition(lander.getPosition().x, lander.getPosition().y + 20, lander.getPosition().z + 45);
	cam.lookAt(lander.getPosition());
	fPov = false;
	aPov = false;
	tPov = true;

	// Remove particle effects
	for (int i = thrustEmitter.sys->particles.size() - 1; i > -1; i--)
		thrustEmitter.sys->remove(i);
	for (int i = explosionEmitter.sys->particles.size() - 1; i > -1; i--)
		explosionEmitter.sys->remove(i);
	for (int i = vortexRingEmitter.sys->particles.size() - 1; i > -1; i--)
		vortexRingEmitter.sys->remove(i);
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	if (gameState) {
		// Update positions
		landerPos = lander.getPosition();
		thrustEmitter.position = landerPos;
		explosionEmitter.position = landerPos;
		landerLight.setPosition(landerPos);
		
		// Call update
		thrustEmitter.update();
		explosionEmitter.update();
		vortexRingEmitter.update();
		landingRingEmitter.update();
		landingRingEmitter2.update();
		landingRingEmitter3.update();
		lander.update();

		// Point light in direction of lander movement
		landerLight.lookAt(lander.getPosition() 
			+ glm::vec3(glm::rotate(glm::mat4(1.0), glm::radians(landerRot), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1)) * 50
			+ glm::vec3(0, -50, 0) * 2);

		//update camera povs
		ofVec3f currentCamPos = cam.getPosition();
		ofVec3f targetCamPos;
		if (fPov) {
			ofVec3f currentCamPos = cam.getPosition();
			ofVec3f targetCamPos = ofVec3f(landerPos.x, landerPos.y + 10, landerPos.z);
			cam.setPosition(currentCamPos.interpolate(targetCamPos, 0.1));
			cam.lookAt(landerPos + glm::vec3(0, 10, 0)
				+ glm::vec3(glm::rotate(glm::mat4(1.0), glm::radians(landerRot), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1)) * 10);
		}
		else if (tPov) {
			ofVec3f currentCamPos = cam.getPosition();
			ofVec3f targetCamPos = ofVec3f(landerPos.x, landerPos.y + 20, landerPos.z + 45);
			cam.setPosition(currentCamPos.interpolate(targetCamPos, 0.1));
			cam.lookAt(lander.getPosition());
		}
		else if (aPov) {
			ofVec3f currentCamPos = cam.getPosition();
			ofVec3f targetCamPos = ofVec3f(landerPos.x, landerPos.y + 50, landerPos.z);
			cam.setPosition(currentCamPos.interpolate(targetCamPos, 0.1));
			cam.lookAt(lander.getPosition());
		}

		// Measure distance -----------------------------------------------------------------------------
		if (bAGL) {
			glm::vec3 origin = lander.getPosition();
			glm::vec3 dir = glm::vec3(0, -1, 0);
			glm::normalize(dir);
			Ray ray = Ray(Vector3(origin.x, origin.y, origin.z), Vector3(dir.x, dir.y, dir.z));
			TreeNode node;

			pointSelected = octree.intersect(ray, octree.root, node);

			Vector3 center = node.box.center();
			distance = glm::distance(lander.getPosition(), glm::vec3(center.x(), center.y(), center.z()));
		}

		// Update vortex ring
		if (distance < 5) {
			if (!vortexRing)
				vortexRingEmitter.start();
			vortexRing = true;
			vortexRingEmitter.position = lander.getPosition() + glm::vec3(0, -distance, 0);
		}
		else {
			if (vortexRing) {
				vortexRingEmitter.stop();
				vortexRingEmitter.sys->reset();
			}
			vortexRing = false;
		}

		// Lander physics simulation -----------------------------------------------------------------------------
		float framerate = ofGetFrameRate();
		float dt = (framerate > 0) ? 1.0 / framerate : 0;

		//move up/down (linear)
		landerPos += landerVelocity * dt;
		landerAcceleration = (1 / landerMass) * landerForce;
		landerVelocity += landerAcceleration * dt;
		landerVelocity *= landerDamping;

		//rotate (angular)
		landerRot += landerAngularVelocity * dt;
		landerAngularAcceleration = (1 / landerMass) * landerAngularForce;
		landerAngularVelocity += landerAngularAcceleration * dt;
		landerAngularVelocity *= landerDamping;

		// Zero out forces
		landerForce = glm::vec3(0, 0, 0);
		landerAngularForce = 0;

		// Update lander pos/rot
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		lander.setRotation(0, landerRot, 0, 1, 0);

		// Add forces -----------------------------------------------------------------------------
		landerForce += gravityForce->get() * landerMass; // Gravity

		// Handle key presses -----------------------------------------------------------------------------
		if (!gameOver && !gameComplete && !gameEnd) {
			if (keysPressed.count(' ') && fuelLevel > 0) {
				landerForce += glm::vec3(0, 10, 0);

				// Start thrust particle effect
				if (!thrust) {
					thrustEmitter.start();
					thrustWhoosh.play();
				}
				thrust = true;

				// Turbulence effect
				landerForce += glm::vec3(ofRandom(turbForce->getMin().x, turbForce->getMax().x)
					, ofRandom(turbForce->getMin().y, turbForce->getMax().y)
					, ofRandom(turbForce->getMin().y, turbForce->getMax().y));
			}
			else {
				// End thrust particle effect
				if (thrust) {
					thrustEmitter.stop();
					thrustEmitter.sys->reset();
				}
				thrust = false;
				thrustWhoosh.stop();
			}

			if (fuelLevel > 0) {
				if (keysPressed.count(OF_KEY_LEFT))
					landerAngularForce += 100;
				if (keysPressed.count(OF_KEY_RIGHT))
					landerAngularForce -= 100;
				if (keysPressed.count(OF_KEY_UP))
					landerForce += glm::vec3(glm::rotate(glm::mat4(1.0), glm::radians(landerRot), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1)) * 10;
				if (keysPressed.count(OF_KEY_DOWN))
					landerForce -= glm::vec3(glm::rotate(glm::mat4(1.0), glm::radians(landerRot), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1)) * 10;
			}

			// Track fuel consumption
			if ((keysPressed.count(' ') || keysPressed.count(OF_KEY_LEFT) || keysPressed.count(OF_KEY_RIGHT)
				|| keysPressed.count(OF_KEY_UP) || keysPressed.count(OF_KEY_DOWN)) && fuelLevel > 0) {
				if (!fuel) {
					// Mark fuel start
					fuelStart = ofGetElapsedTimef();
				}
				fuel = true;

				// Update fuel level
				usedFuel = ofGetElapsedTimef() - fuelStart;
				fuelLevel = initialFuel - usedFuel;
			}
			else {
				if (fuel) {
					// Update fuel level
					initialFuel -= usedFuel;
				}
				fuel = false;
			}
		}
		else {
			// End thrust effect if still playing
			if (thrust) {
				thrustEmitter.stop();
				thrustEmitter.sys->reset();
			}
			thrust = false;
			thrustWhoosh.stop();
		}

		// Collision -----------------------------------------------------------------------------
		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);

		if (colBoxList.size() >= 5) {
			// Explosion (return, end game if true)
			if (glm::length(landerVelocity) > 3 && !gameOver && !gameComplete && !gameEnd) {
				// Start explosion particle effect
				if (!explosion)
					explosionEmitter.start();
					landerBoom.play();
				explosionStart = ofGetElapsedTimef();

				// Apply game over, explode lander in random upward direction
				gameOver = true;
				landerForce += glm::vec3((ofRandom(-1, 1) > 0 ? 1 : -1) * 3000, 3000, (ofRandom(-1, 1) > 0 ? 1 : -1) * 3000);
				return;
			} else if (glm::length(landerVelocity) < 3 && !gameOver && !gameComplete && !gameEnd) {
				landingStart = ofGetElapsedTimef();
				checkCollisionPosition(lander.getPosition());
			}

			// Resolution
			glm::vec3 p2 = glm::vec3(bounds.center().x(), bounds.center().y(), bounds.center().z()); // Center of lander box

			// Find closest box center
			glm::vec3 p1;
			float min = FLT_MAX;
			for (Box box : colBoxList) {
				Vector3 center = box.center();
				glm::vec3 point = glm::vec3(center.x(), center.y(), center.z());
				float distance = glm::length(p2 - point);
				if (distance < min) {
					min = distance;
					p1 = point;
				}
			}

			float e = 0.1; // Restitution (0-1)
			glm::vec3 n = glm::normalize(p2 - p1); // Normal
			glm::vec3 p = (e + 1) * (-glm::dot(landerVelocity, n)) * n; // Impulse force (assume mass 1, inf)

			// Apply impulse directly to velocity (adjusts velocity, not forces)
			landerVelocity = p;
		}
		else {
			if (gameOver) {
				// End explosion particle effect
				if (explosion) {
					explosionEmitter.stop();
					explosionEmitter.sys->reset();
					explosion = false;
				}
				// Stop game if explosion is done playing and game is over
				if (ofGetElapsedTimef() - explosionStart > 3)
					stopGame();
			} else if (gameComplete) {
				if (ofGetElapsedTimef() - landingStart > 3)
					stopGame();
			}
			else if (gameEnd) {
				if (ofGetElapsedTimef() - landingStart > 3)
					stopGame();
			}
		}
	}
	
}

//--------------------------------------------------------------
void ofApp::draw() {
	loadThrustVbo();
	loadExplosionVbo();
	loadVortexRingVbo();
	loadLandingRingVbo();
	loadLandingRingVbo2();
	loadLandingRingVbo3();

	// Handle lander light toggle
	if (bLanderLight)
		landerLight.enable();
	else
		landerLight.disable();

	cam.begin();

	ofEnableDepthTest();  
	background.getTexture().bind();  
	sky.draw();
	background.getTexture().unbind();

	ofPushMatrix();
	
	ofEnableLighting();              // shaded mode
	moon.drawFaces();
	if (bLanderLoaded)
		lander.drawFaces();
	ofDisableLighting();

	ofPopMatrix();
	cam.end();

	// Draw Particles -----------------------------------------------------------------------------
	glDepthMask(false);

	// this makes everything look glowy :)
	//
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

	shader.begin();
	cam.begin();

	particleTex.bind();

	ofSetColor(230, 153, 255); // Thrust color
	thrustVbo.draw(GL_POINTS, 0, (int)thrustEmitter.sys->particles.size());

	ofSetColor(255, 166, 77); // Explosion color
	explosionVbo.draw(GL_POINTS, 0, (int)explosionEmitter.sys->particles.size());

	ofSetColor(230, 230, 230); // Vortex ring color
	vortexRingVbo.draw(GL_POINTS, 0, (int)vortexRingEmitter.sys->particles.size());

	//landing ring color
	ofSetColor(133, 224, 133);
	landingRingVbo.draw(GL_POINTS, 0, (int)landingRingEmitter.sys->particles.size());
	ofSetColor(133, 224, 133); 
	landingRingVbo2.draw(GL_POINTS, 0, (int)landingRingEmitter2.sys->particles.size());
	ofSetColor(133, 224, 133);
	landingRingVbo3.draw(GL_POINTS, 0, (int)landingRingEmitter3.sys->particles.size());

	particleTex.unbind();

	cam.end();
	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();

	glDepthMask(true);

	// Draw UI last to place on top -----------------------------------------------------------------------------
	glDepthMask(false);
	ofSetColor(ofColor::white);

	if (gameState && !gameOver && !gameComplete && !gameEnd) {
		// Draw top left info
		ofDrawBitmapString("Velocity: " + ofToString(landerVelocity.x, 2) + " " + ofToString(landerVelocity.y, 2) + " " + ofToString(landerVelocity.z, 2), 15 * 2, 15 * 2);
		ofDrawBitmapString((fuelLevel > 0) ? "Fuel: " + ofToString(fuelLevel, 2) : "Fuel: EMPTY!", 15 * 2, 30 * 2);
		if (bAGL)
			ofDrawBitmapString("Altitude: " + ofToString(distance, 2), 15 * 2, 45 * 2);

		// Draw bottom right info
		ofBitmapFont font = ofBitmapFont();
		string text = "x: Lander Light";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		ofDrawBitmapString("Space: Thrust", ofGetWidth() - width - 15 * 2, ofGetHeight() - 75 * 2);
		ofDrawBitmapString("Arrows: Move", ofGetWidth() - width - 15 * 2, ofGetHeight() - 60 * 2);
		ofDrawBitmapString("z: Cycle Views", ofGetWidth() - width - 15 * 2, ofGetHeight() - 45 * 2);
		ofDrawBitmapString("x: Lander Light", ofGetWidth() - width - 15 * 2, ofGetHeight() - 30 * 2);
		ofDrawBitmapString("n: AGL", ofGetWidth() - width - 15 * 2, ofGetHeight() - 15 * 2);
	}
	else if (gameState && gameOver && !gameComplete && !gameEnd) {
		ofBitmapFont font = ofBitmapFont();
		string text = "Ship Exploded. Game Over!";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2 - height / 2);
	}
	else if (gameState && gameComplete && !gameOver && !gameEnd) {
		ofBitmapFont font = ofBitmapFont();
		string text = "Landed Successfully. Game Complete!";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2 - height / 2);
	}
	else if (gameState && gameEnd && !gameOver && !gameComplete) {
		ofBitmapFont font = ofBitmapFont();
		string text = "Lander did not land in a spotlight! Try again.";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2 - height / 2);
	}
	else if (gameInstructions) {
		ofBitmapFont font = ofBitmapFont();
		string text = "Game Controls";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2.5 - height / 2);
		string goal = "Goal: Land within the 3 spotlights and keep your velocity under 3!";
		ofDrawBitmapString(goal, ofGetWidth() / 2.5 - width / 2, ofGetHeight() / 2.5 - height / 2 + 20);
		string space = "Space to use thruster";
		ofDrawBitmapString(space, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2.5 - height / 2 + 40);
		string xLight = "x to turn lander light on/off";
		ofDrawBitmapString(xLight, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2.5 - height / 2 + 60);
		string zCam = "z to swap between 3 camera views";
		ofDrawBitmapString(zCam, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2.5 - height / 2 + 80);
		string cMouse = "c to move the camera";
		ofDrawBitmapString(cMouse, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2.5 - height / 2 + 100);
		string rightClick = "Right click to set your own camera view";
		ofDrawBitmapString(rightClick, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2.5 - height / 2 + 120);
		string start = "Press Enter to start";
		ofDrawBitmapString(start, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2.5 - height / 2 + 140);
	}
	else if (!gameState) {
		startScreen = true;
		ofBitmapFont font = ofBitmapFont();
		string title = "UFO and the Grey Cheese";
		string text = "Press Enter to Begin or P to see game controls";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(title, ofGetWidth() / 1.75 - width / 2, ofGetHeight() / 2 - height / 2);
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2 - height / 2 + 20);
	}
	
	glDepthMask(true);
}

void ofApp::keyPressed(int key) {
	keysPressed.insert(key);

	switch (key) {
	case 'B':
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'r':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case 'z':
		view++;
		if (view > 2)
			view = 0;

		switch (view) {
		case 0:
			fPov = true;
			tPov = false;
			aPov = false;
			cam.setPosition(lander.getPosition().x, lander.getPosition().y + 10, lander.getPosition().z + 10);
			cam.lookAt(lander.getPosition() + glm::vec3(0, 0, 45));
			break;
		case 1:
			fPov = false;
			aPov = false;
			tPov = true;
			cam.setPosition(lander.getPosition().x, lander.getPosition().y + 20, lander.getPosition().z + 45);
			cam.lookAt(lander.getPosition());
			break;
		case 2:
			fPov = false;
			tPov = false;
			aPov = true;
			cam.setPosition(lander.getPosition().x, lander.getPosition().y + 50, lander.getPosition().z);
			cam.lookAt(lander.getPosition());
			break;
		}
		break;
	case 'x':
		bLanderLight = !bLanderLight; // Toggle lander light
		break;
	case OF_KEY_RETURN:
		
		// Reset game values
		if (!gameState || gameInstructions) {
			// Game state
			gameState = true;
			gameOver = false;
			gameComplete = false;
			gameEnd = false;
			gameInstructions = false;
			startScreen = false;

			// Fuel
			fuelLevel = 120;
			initialFuel = 120;
			usedFuel = 0;
		}
		break;
	case 'p':
		if (startScreen) {
			gameInstructions = true;
			startScreen = false;
			cout << "instructions screen" << endl;
		}
		break;
	case 'n':
		bAGL = !bAGL;
		break;
	default:
		break;
	}
}

void ofApp::keyReleased(int key) {
	keysPressed.erase(key);

	switch (key) {

	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {


}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	glm::vec3 origin = cam.getPosition();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded && gameState == false) {

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}

	//retarget camera view
	if (button == OF_MOUSE_BUTTON_RIGHT) {
		setCameraTarget();
	}
}

bool ofApp::raySelectWithOctree(ofVec3f& pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	Vector3 center = selectedNode.box.center();

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;

		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);


		/*if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}*/


	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}

// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {
	glm::vec3 origin = cam.getPosition();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

	glm::vec3 target;

	if (bLanderLoaded) {
		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();
		Box landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = landerBounds.intersect(
			Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);

		if (hit) {
			target = lander.getPosition();
		}
	}
	if (target == glm::vec3(0, 0, 0)) {
		float distance;
		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), distance);
		if (hit) {
			target = origin + distance * mouseDir;
		}
	}
	if (target != glm::vec3(0, 0, 0)) {
		cam.lookAt(target);
		cPov = true;
		fPov = tPov = aPov = false;
	}
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{ 5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
}

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}