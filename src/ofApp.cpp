
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   Lee Rogers
//  Date: 11/19/2024


#include "ofApp.h"
#include "Util.h"
#include "glm/gtx/intersect.hpp"

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup() {
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
	ofSetWindowShape(1280, 768);
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
	// moon.loadModel("geo/moon-houdini.obj");
	moon.setScaleNormalization(false);

	// Load lander
	lander.loadModel("geo/ufo.obj");
	lander.setScaleNormalization(false);
	bLanderLoaded = true;
	lander.setPosition(0, 50, 0);
	landerPos = glm::vec3(0, 50, 0);
	
	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(timingInfo.setup("Timing Info", true));
	gui.setPosition(15, ofGetHeight() - gui.getHeight() - 15);
	bHide = false;

	//  Create Octree for testing.
	//

	float start = ofGetElapsedTimeMillis();
	octree.create(moon.getMesh(0), 20);
	float end = ofGetElapsedTimeMillis();

	if (timingInfo)
		cout << "Octree build time: " << end - start << " milliseconds" << endl;
	cout << "Number of Verts: " << moon.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

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
	thrustWhoosh.load("sounds/thrust.mp3");

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

	// Set landing area
	landing = glm::vec3(-100, 32, 30);
	landingRadius = 20;

	// Create particle forces
	turbForce = new TurbulenceForce(ofVec3f(-10, -10, -10), ofVec3f(10, 10, 10));
	gravityForce = new GravityForce(ofVec3f(0, -2, 0));
	radialForce = new ImpulseRadialForce(10);
	cyclicForce = new CyclicForce(10);

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

	landingRingEmitter.setVelocity(ofVec3f(0, 3, 0));
	landingRingEmitter.setEmitterType(CircularEmitter);
	landingRingEmitter.setGroupSize(50);
	landingRingEmitter.setRate(30);
	landingRingEmitter.setRandomLife(true);
	landingRingEmitter.setLifespanRange(ofVec2f(0, 1));
	landingRingEmitter.setParticleRadius(15);
	landingRingEmitter.setCircularEmitterRadius(landingRadius);
	landingRingEmitter.setPosition(landing);

	landingRingEmitter.start();

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
}

// load vertex buffer in preparation for rendering
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
			ofVec3f targetCamPos = ofVec3f(landerPos.x, landerPos.y + 10, landerPos.z + 10);
			cam.setPosition(currentCamPos.interpolate(targetCamPos, 0.1)); // Smooth transition
			cam.lookAt(landerPos + glm::vec3(0, 0, 45));
		} else if (tPov) {
			ofVec3f currentCamPos = cam.getPosition();
			ofVec3f targetCamPos = ofVec3f(landerPos.x, landerPos.y + 20, landerPos.z + 45);
			cam.setPosition(currentCamPos.interpolate(targetCamPos, 0.1)); // Smooth transition
			cam.lookAt(lander.getPosition());
		}

		// Measure distance -----------------------------------------------------------------------------
		glm::vec3 origin = lander.getPosition();
		glm::vec3 dir = glm::vec3(0, -1, 0);
		glm::normalize(dir);
		Ray ray = Ray(Vector3(origin.x, origin.y, origin.z), Vector3(dir.x, dir.y, dir.z));
		TreeNode node;

		pointSelected = octree.intersect(ray, octree.root, node);

		Vector3 center = node.box.center();
		distance = glm::distance(lander.getPosition(), glm::vec3(center.x(), center.y(), center.z()));

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
		if (!gameOver && !gameComplete) {
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
			}
			if (keysPressed.count(OF_KEY_LEFT))
				landerAngularForce += 100;
			if (keysPressed.count(OF_KEY_RIGHT))
				landerAngularForce -= 100;
			if (keysPressed.count(OF_KEY_UP))
				landerForce += glm::vec3(glm::rotate(glm::mat4(1.0), glm::radians(landerRot), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1)) * 10;
			if (keysPressed.count(OF_KEY_DOWN))
				landerForce -= glm::vec3(glm::rotate(glm::mat4(1.0), glm::radians(landerRot), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1)) * 10;

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

		// Collision -----------------------------------------------------------------------------
		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);

		if (colBoxList.size() >= 10) {
			// Explosion (return, end game if true)
			if (glm::length(landerVelocity) > 3) {
				// Start explosion particle effect
				if (!explosion)
					explosionEmitter.start();
					landerBoom.play();
				explosionStart = ofGetElapsedTimef();

				// Apply game over, explode lander in random direction
				gameOver = true;
				landerForce += glm::vec3((ofRandom(-1, 1) > 0 ? 1 : -1) * ofRandom(1000, 5000)
					, ofRandom(1000, 5000), (ofRandom(-1, 1) > 0 ? 1 : -1) * ofRandom(1000, 5000));
				return;
			} else if (glm::length(landerVelocity) < 0.5 && !gameOver && !gameComplete) {
				glm::vec3 pos = glm::vec3(landerPos.x, 0, landerPos.z);
				glm::vec3 dest = glm::vec3(landing.x, 0, landing.z);

				// Apply game complete if lander within radius
				if (glm::length(dest - pos) < landingRadius) {
					gameComplete = true;
					landingStart = ofGetElapsedTimef();
				}
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
				}
				explosion = false;

				// Stop game if explosion is done playing and game is over
				if (ofGetElapsedTimef() - explosionStart > 3)
					gameState = false;
			} else if (gameComplete) {
				if (ofGetElapsedTimef() - landingStart > 3)
					gameState = false;
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
	//ofBackground(ofColor::black);

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
	

	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		moon.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		moon.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}

			if (bLanderSelected) {

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));



	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		moon.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}


	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
	}
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		// ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]); // Set position at point
		Vector3 center = selectedNode.box.center();
		ofVec3f p = glm::vec3(center.x(), center.y(), center.z()); // Set position at center of box
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}

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

	ofSetColor(133, 224, 133); // Landing ring color
	landingRingVbo.draw(GL_POINTS, 0, (int)landingRingEmitter.sys->particles.size());

	particleTex.unbind();

	cam.end();
	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();

	glDepthMask(true);

	// Draw UI last to place on top -----------------------------------------------------------------------------
	glDepthMask(false);
	if (!bHide) gui.draw();
	ofSetColor(ofColor::white);

	if (gameState && !gameOver && !gameComplete) {
		// Draw top left info
		ofDrawBitmapString("Altitude: " + ofToString(distance, 2), 15, 15);
		ofDrawBitmapString("Velocity: " + ofToString(landerVelocity.x, 2) + " " + ofToString(landerVelocity.y, 2) + " " + ofToString(landerVelocity.z, 2), 15, 30);
		ofDrawBitmapString((fuelLevel > 0) ? "Fuel: " + ofToString(fuelLevel, 2) : "Fuel: EMPTY!", 15, 45);

		// Draw bottom right info
		ofBitmapFont font = ofBitmapFont();
		string text = "x: Lander Light";
		int width = font.getBoundingBox(text, 0, 0).getWidth(); // Get width of longest text
		ofDrawBitmapString("Space: Thrust", ofGetWidth() - width - 15, ofGetHeight() - 60);
		ofDrawBitmapString("Arrows: Move", ofGetWidth() - width - 15, ofGetHeight() - 45);
		ofDrawBitmapString("z: Cycle Views", ofGetWidth() - width - 15, ofGetHeight() - 30);
		ofDrawBitmapString("x: Lander Light", ofGetWidth() - width - 15, ofGetHeight() - 15);
	} else if (gameState && gameOver) {
		ofBitmapFont font = ofBitmapFont();
		string text = "Ship Exploded. Game Over!";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2 - height / 2);
	} else if (gameState && gameComplete) {
		ofBitmapFont font = ofBitmapFont();
		string text = "Landed Successfully. Game Complete!";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2 - height / 2);
	} else {
		ofBitmapFont font = ofBitmapFont();
		string text = "Press Enter to Begin";
		int width = font.getBoundingBox(text, 0, 0).getWidth();
		int height = font.getBoundingBox(text, 0, 0).getHeight();
		ofDrawBitmapString(text, ofGetWidth() / 2 - width / 2, ofGetHeight() / 2 - height / 2);
	}
	glDepthMask(true);
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {
	keysPressed.insert(key);

	switch (key) {
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
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
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
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
		if (view > 1)
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
			/*case 2:
				fPov = false;
				tPov = false;
				aPov = true;
				cam.setPosition(lander.getPosition().x, lander.getPosition().y + 50, lander.getPosition().z);
				cam.lookAt(lander.getPosition());
				break;*/
		}
		break;
	case 'x':
		bLanderLight = !bLanderLight; // Toggle lander light
		break;
	case OF_KEY_RETURN:
		// Reset game values
		if (!gameState) {
			// Game state
			gameState = true;
			gameOver = false;
			gameComplete = false;

			// Fuel
			fuelLevel = 120;
			initialFuel = 120;
			usedFuel = 0;
		}
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
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

	if (button == OF_MOUSE_BUTTON_RIGHT) {
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
}

bool ofApp::raySelectWithOctree(ofVec3f& pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	float start = ofGetElapsedTimeMillis();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	float end = ofGetElapsedTimeMillis();

	Vector3 center = selectedNode.box.center();
	cout << "Position" << center.x() << ", " << center.y() << ", " << center.z() << endl;

	if (timingInfo)
		cout << "Ray intersection time: " << end - start << " milliseconds" << endl;

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

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (lander.loadModel(dragInfo.files[0])) {
		lander.setScaleNormalization(false);
		//		lander.setScale(.1, .1, .1);
			//	lander.setPosition(point.x, point.y, point.z);
		lander.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		cout << "Mesh Count: " << lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


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