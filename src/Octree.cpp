
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Simple Octree Implementation 11/10/2020
// 
//  Copyright (c) by Kevin M. Smith
//  Copying or use without permission is prohibited by law. 
//


#include "Octree.h"
 


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}

// getMeshFacesInBox:  return an array of indices to Faces in mesh that are contained 
//                      inside the Box.  Return count of faces found;
//
int Octree::getMeshFacesInBox(const ofMesh & mesh, const vector<int>& faces,
	Box & box, vector<int> & facesRtn)
{
	int count = 0;
	for (int i = 0; i < faces.size(); i++) {
		ofMeshFace face = mesh.getFace(faces[i]);
		ofVec3f v[3];
		v[0] = face.getVertex(0);
		v[1] = face.getVertex(1);
		v[2] = face.getVertex(2);
		Vector3 p[3];
		p[0] = Vector3(v[0].x, v[0].y, v[0].z);
		p[1] = Vector3(v[1].x, v[1].y, v[1].z);
		p[2] = Vector3(v[2].x, v[2].y, v[2].z);
		if (box.inside(p,3)) {
			count++;
			facesRtn.push_back(faces[i]);
		}
	}
	return count;
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & geo, int numLevels) {
	// initialize octree structure
	//
	mesh = geo;
	int level = 0;
	root.box = meshBounds(mesh);
	
	if (!bUseFaces) {
		for (int i = 0; i < mesh.getNumVertices(); i++) {
			root.points.push_back(i);
		}
	}
	else {
		// need to load face vertices here
		//
		for (int i = 0; i < mesh.getNumIndices()/3; i++) {
			root.points.push_back(i);
		}
	}

	// recursively buid octree
	//
	level++;
    subdivide(mesh, root, numLevels, level);
}


//
// subdivide:  recursive function to perform octree subdivision on a mesh
//
//  subdivide(node) algorithm:
//     1) subdivide box in node into 8 equal side boxes - see helper function subDivideBox8().
//     2) For each child box
//            sort point data into each box  (see helper function getMeshFacesInBox())
//        if a child box contains at list 1 point
//            add child to tree
//            if child is not a leaf node (contains more than 1 point)
//               recursively call subdivide(child)
//         
//      
             
void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;

	// subdvide algorithm implemented here
	vector<Box> boxList;
	subDivideBox8(node.box, boxList);

	for (const auto& childBox : boxList) {
		TreeNode childNode;
		childNode.box = childBox;

		vector<int> pointsInBox;
		getMeshPointsInBox(mesh, node.points, childNode.box, pointsInBox);

		if (!pointsInBox.empty()) {
			childNode.points = pointsInBox;
			node.children.push_back(childNode);

			if (pointsInBox.size() > 1) {

				subdivide(mesh, node.children.back(), numLevels, level+1); 
			}
		}
	}

	
}

// Implement functions below for Homework project
//

bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
	bool intersects = false;
	//return intersects;
	float t0 = 0.0f;
	float t1 = FLT_MAX;
	float minDistance = FLT_MAX;
	int closestPointIndex = -1;
	glm::vec3 rayOrigin(ray.origin.x(), ray.origin.y(), ray.origin.z());  

	if (!node.box.intersect(ray, t0, t1)) {
		return false;
	}


	if (node.children.empty()) {
		nodeRtn = node;
		

		for (int point : node.points) {
			glm::vec3 p = mesh.getVertex(point);

			float distance = glm::distance(rayOrigin, p);
			if (distance < minDistance) {
				minDistance = distance;
				closestPointIndex = point;
			}
		}
		if (closestPointIndex != -1) {
			closestPoint = mesh.getVertex(closestPointIndex);
			//std::cout << "closest Point: " << closestPoint << std::endl;
			//ofDrawSphere(closestPoint, 10);
		}

		return true;
	}

	for (auto& child : node.children) {
		if (intersect(ray, child, nodeRtn)) {
			intersects = true;
		}
	}
	return intersects;
}

bool Octree::intersect(const Box &box, TreeNode & node, vector<Box> & boxListRtn) {
	bool intersects = false;

	const auto& nodeMin = node.box.parameters[0];
	const auto& nodeMax = node.box.parameters[1];
	const auto& boxMin = box.parameters[0];
	const auto& boxMax = box.parameters[1];

	if (nodeMax.x() < boxMin.x() || nodeMin.x() > boxMax.x()) return false;
	if (nodeMax.y() < boxMin.y() || nodeMin.y() > boxMax.y()) return false;
	if (nodeMax.z() < boxMin.z() || nodeMin.z() > boxMax.z()) return false;

	intersects = true;

	if (node.children.empty()) {
		boxListRtn.push_back(node.box);
	}
	else {
		for (auto& child : node.children) {
			if (intersect(box, child, boxListRtn)) {
				intersects = true;
			}
		}
	}
	return intersects;
}

void Octree::draw(TreeNode & node, int numLevels, int level) {
	if (level > numLevels) return;

	if (colors) {
		ofSetColor((*colors)[level % colors->size()]);
	}

	drawBox(node.box);

	for (auto& child : node.children) {
		draw(child, numLevels, level + 1);
	}
}


// Optional
//
void Octree::drawLeafNodes(TreeNode & node) {


}




