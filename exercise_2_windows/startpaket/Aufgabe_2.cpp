#include "webcam-head-tracker.hpp"
#define _USE_MATH_DEFINES
#include <cstdio>
#include <cmath>
#include <iostream>

 
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/MeshData3D.h>



#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include<chrono>
#include<thread>



using namespace Magnum;
using namespace Magnum::Math::Literals;

typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;
typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;



std::shared_ptr<WebcamHeadTracker> tracker;

// Adapt this function to calculate a projection matrix that adapts to a users position
void calculate_perspektive() {

}








class FirstScene: public Platform::Application {
    public:
        explicit FirstScene(const Arguments& arguments);

    private:
        void drawEvent() override;

        Scene3D _scene;
        Object3D* _cameraObject;
        SceneGraph::Camera3D* _camera;
        SceneGraph::DrawableGroup3D _drawables;

        GL::Mesh _cube;
        Shaders::Phong _shader;
		glm::mat4 projMat;
		glm::mat4 viewMat;
		std::chrono::time_point<std::chrono::high_resolution_clock> a = std::chrono::high_resolution_clock::now();
		};

class ColoredDrawable: public SceneGraph::Drawable3D {
    public:
        explicit ColoredDrawable(Object3D& object, GL::Mesh& mesh,
            Shaders::Phong& shader, const Color4& color,
            SceneGraph::DrawableGroup3D* drawables = nullptr);

    private:
        void draw(const Matrix4& transformationMatrix,
            SceneGraph::Camera3D& camera) override;

        GL::Mesh& _mesh;
        Shaders::Phong& _shader;
        Color4 _color;

};
//Constructor of FirstScene
FirstScene::FirstScene(const Arguments& arguments): 

Platform::Application{arguments,
//Make window fullscreen
Configuration{}.setWindowFlags(Configuration::WindowFlag::Fullscreen)
} 
{

	// Enable OpenGL depth test
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	

	// Create an object in the scene graph for the camera
    _cameraObject = new Object3D{&_scene};

	// Create a camera (add it to the camera-object in the scenegraph)
    _camera = new SceneGraph::Camera3D{*_cameraObject};

	// Create a cube and prepare it for rendering
    _cube = MeshTools::compile(Primitives::cubeSolid());
	// Create an object in the scenegraph for the cube
    auto* object = new Object3D{&_scene};
	// Create a drawable object that gets the mesh of the cube and is added to the object in the scenegraph for the cube
    new ColoredDrawable{*object, _cube, _shader,
        0xff9400_rgbf, &_drawables};
	// Move the cube object into the scene (minus z axis)
	(*object).translate(Vector3::zAxis(-5.1f));
	// Scale the cube to have a side length of 10 x 10 x 10 (default size is 2 x 2 x 2: from -1 to +1 on each axis)
	(*object).scaleLocal({5.f,5.f,5.f});
}

// Constructor of ColoredDrawable
ColoredDrawable::ColoredDrawable(Object3D& object, GL::Mesh& mesh, Shaders::Phong& shader, const Color4& color, SceneGraph::DrawableGroup3D* drawables): 
	SceneGraph::Drawable3D{object, drawables}, _mesh{mesh}, _shader{shader}, _color{color} {}


// Draw function of ColoredDrawable. This function is called if a camera object draws a drawableGroup which holds this object
void ColoredDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) {
	
    _shader
        .setDiffuseColor(_color)
        .setLightPosition(Vector3(0.0f,0.0f,0.0f))
        .setTransformationMatrix(transformationMatrix)
        .setNormalMatrix(transformationMatrix.rotationScaling())
        .setProjectionMatrix(camera.projectionMatrix());
    _mesh.draw(_shader);
}


// Draw function: This function is called by the event manager to draw the frame.
void FirstScene::drawEvent() {
	GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);
	
	if (tracker->isReady()) {
		tracker->getNewFrame();
		bool gotPose = tracker->computeHeadPose();
		if (gotPose) {
			
			float pos[3];
			tracker->getHeadPosition(pos);
			//Convert position into cm
			pos[0] *= 100.0f;
			pos[1] *= 100.0f;
			pos[2] *= 100.0f;

			// Calculate a projection matrix
			calculate_perspektive();
					
		}		
	}
	// In case you want to transform the pose of the camera use the following function:
	//_cameraObject->setTransformation();
	
	// For setting the projection matrix, use the following function (it expects a Magnum matrix (4x4 Matrix))
	//_camera->setProjectionMatrix();
	
	// Draw all objects in the drawable group
	_camera->draw(_drawables);

	// Swap front and back buffer
	swapBuffers();
	
	//Tell the event manager to redraw (draw every frame)
	redraw();
}



int main(int argc, char** argv) { 

	// Create a WebcamHeadTracker object 
	tracker = std::make_shared<WebcamHeadTracker>(WebcamHeadTracker::Filter_Double_Exponential);
	
	// Initialise the webcam
	if (!tracker->initWebcam()) {
		std::cout << "No usable webcam found" << std::endl;
		::exit(0);
	}
	
	// Initialise the pose estimator
	if (!tracker->initPoseEstimator()) {
		std::cout << "Cannot initialize pose esimator:\n haarcascade_frontalface_alt.xml and shape_predictor_68_face_landmarks.dat\n are not where they were when libwebcamheadtracker was built" << std::endl;
		::exit(0);
	}
	
	// Create an object of FirstScene and execute it (implements an event manager)
	FirstScene app({argc, argv}); 
	return app.exec(); 
}



