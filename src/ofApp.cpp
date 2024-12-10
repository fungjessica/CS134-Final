
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
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
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
	//bLanderLoaded = true;
	lander.setPosition(0, 100, 0);
	landerPos = glm::vec3(0, 100, 0);

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

	// load the shader
	//
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif

	// Create particle forces
	turbForce = new TurbulenceForce(ofVec3f(-3,-3, -3), ofVec3f(3, 3, 3));
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

	thrust = false;

	explosionEmitter.sys->addForce(turbForce);
	explosionEmitter.sys->addForce(gravityForce);
	explosionEmitter.sys->addForce(radialForce);

	explosionEmitter.setVelocity(ofVec3f(0, -10, 0));
	explosionEmitter.setEmitterType(RadialEmitter);
	explosionEmitter.setGroupSize(500);
	explosionEmitter.setOneShot(true);
	explosionEmitter.setRandomLife(true);
	explosionEmitter.setLifespanRange(ofVec2f(0, 0.5));
	explosionEmitter.setParticleRadius(10);

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

	// Setup fuel
	fuel = 120; // Fuel level (120 seconds, 2 minutes)
	initialFuel = 120;
	usedFuel = 0;

	//spotlights for the 3 landing zones
	ofEnableLighting();

	//lighting for entire terrain
	spotlight1.setDiffuseColor(ofFloatColor(0.9, 0.9, 0.9));  
	spotlight1.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0)); 
	spotlight1.setPosition(0, 500, 0);                     
	spotlight1.setSpotlight();
	spotlight1.setSpotlightCutOff(90);                       
	spotlight1.setSpotConcentration(5);                      
	spotlight1.lookAt(glm::vec3(0, 0, 0));
	spotlight1.enable();

	spotlight2.setDiffuseColor(ofFloatColor(1.0, 0.8, 0.8) * 3.0); 
	spotlight2.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0) * 2.0);
	spotlight2.setPosition(50, 100, 50);
	spotlight2.setSpotlight();
	spotlight2.setSpotlightCutOff(45);
	spotlight2.setSpotConcentration(15);
	spotlight2.lookAt(glm::vec3(0, 0, 0));
	spotlight2.enable();

	spotlight3.setDiffuseColor(ofFloatColor(0.8, 1.0, 0.8) * 3.0);
	spotlight3.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0) * 2.0);
	spotlight3.setPosition(-50, 30, 50);
	spotlight3.setSpotlight();
	spotlight3.setSpotlightCutOff(45);
	spotlight3.setSpotConcentration(15);
	spotlight3.lookAt(glm::vec3(-100, 0, 0));
	spotlight3.enable();

	spotlight4.setDiffuseColor(ofFloatColor(1.0, 0.9, 0.8) * 3.0);  
	spotlight4.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0) * 2.0);
	spotlight4.setPosition(100, 60, -50);                      
	spotlight4.setSpotlight();
	spotlight4.setSpotlightCutOff(45);                        
	spotlight4.setSpotConcentration(15);                     
	spotlight4.lookAt(glm::vec3(50, 0, 150));                    
	spotlight4.enable();
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

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	bLanderLoaded = true;

	thrustEmitter.position = landerPos;
	explosionEmitter.position = landerPos;
	thrustEmitter.update();
	explosionEmitter.update();
	vortexRingEmitter.update();

	// Measure distance -----------------------------------------------------------------------------
	glm::vec3 rayPoint = lander.getPosition();
	glm::vec3 rayDir = glm::vec3(0, -1, 0);
	glm::normalize(rayDir);
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z), Vector3(rayDir.x, rayDir.y, rayDir.z));
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
	} else {
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
	landerForce += glm::vec3(0, -2, 0); // Gravity

	// Handle key presses -----------------------------------------------------------------------------
	if (keysPressed.count(' ')) {
		landerForce += glm::vec3(0, 10, 0);

		// Start thrust particle effect
		if (!thrust) {
			thrustEmitter.start();

			// Mark fuel start
			fuelStart = ofGetElapsedTimef();
		}
		thrust = true;

		// Update fuel level
		usedFuel = ofGetElapsedTimef() - fuelStart;
		fuel = initialFuel - usedFuel;
	} else {
		// End thrust particle effect
		if (thrust) {
			thrustEmitter.stop();
			thrustEmitter.sys->reset();

			// Update fuel level
			initialFuel -= usedFuel;
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

	// Collision -----------------------------------------------------------------------------
	ofVec3f min = lander.getSceneMin() + lander.getPosition();
	ofVec3f max = lander.getSceneMax() + lander.getPosition();
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	colBoxList.clear();
	octree.intersect(bounds, octree.root, colBoxList);

	if (colBoxList.size() >= 10) {
		// Explosion
		if (glm::length(landerVelocity) > 3) {
			// Start explosion particle effect
			if (!explosion)
				explosionEmitter.start();
		} else {
			// End explosion particle effect
			if (explosion) {
				explosionEmitter.stop();
				explosionEmitter.sys->reset();
			}
			explosion = false;
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
}
//--------------------------------------------------------------
void ofApp::draw() {
	loadThrustVbo();
	loadExplosionVbo();
	loadVortexRingVbo();
	ofBackground(ofColor::black);

	cam.begin();
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

	ofSetColor(255, 166, 77); // Thrust color
	thrustVbo.draw(GL_POINTS, 0, (int)thrustEmitter.sys->particles.size());

	ofSetColor(255, 77, 77); // Explosion color
	explosionVbo.draw(GL_POINTS, 0, (int)explosionEmitter.sys->particles.size());

	ofSetColor(230, 230, 230); // Vortex ring color
	vortexRingVbo.draw(GL_POINTS, 0, (int)vortexRingEmitter.sys->particles.size());

	particleTex.unbind();

	cam.end();
	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();
	
	glDepthMask(true);

	// Draw GUI last to place on top -----------------------------------------------------------------------------
	glDepthMask(false);
	if (!bHide) gui.draw();
	ofSetColor(ofColor::white);
	ofDrawBitmapString("Altitude: " + to_string(distance), 15, 15);
	ofDrawBitmapString("Velocity: " + to_string(landerVelocity.x) + " " + to_string(landerVelocity.y) + " " + to_string(landerVelocity.z), 15, 30);
	ofDrawBitmapString("Fuel: " + to_string(fuel), 15, 45);
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
		if (view > 2)
			view = 0;

		switch (view) {
		case 0:
			cam.setPosition(lander.getPosition().x, lander.getPosition().y+10, lander.getPosition().z + 10); 
			cam.lookAt(lander.getPosition() + glm::vec3(0, 0, 45));
			break;
		case 1:
			cam.setPosition(lander.getPosition().x, lander.getPosition().y + 20, lander.getPosition().z + 45); 
			cam.lookAt(lander.getPosition());
			break;
		case 2:
			cam.setPosition(lander.getPosition().x, lander.getPosition().y + 50, lander.getPosition().z); 
			cam.lookAt(lander.getPosition());
			break;
		}
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
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
			cout << "lander clicked" << endl;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	float start = ofGetElapsedTimeMillis();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	float end = ofGetElapsedTimeMillis();

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
		cout << "drag???" << endl;

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
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

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

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
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
