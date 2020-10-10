#include <Corrade/Containers/Array.h>
#include <Corrade/Utility/Resource.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Trade/MeshData3D.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/Phong.h>

using namespace Magnum;
using namespace Magnum::Math::Literals;

typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;
typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;

class ColoredDrawable: public SceneGraph::Drawable3D {
    public:
        explicit ColoredDrawable(Object3D& object, GL::Mesh& mesh,
            Shaders::Phong& shader, const Color4& color,
            SceneGraph::DrawableGroup3D* drawables = nullptr);

		void setSelected(bool selected) { _selected = selected; }
    private:
		virtual void draw(const Matrix4& transformationMatrix,
			SceneGraph::Camera3D& camera);

		bool _selected;
        GL::Mesh& _mesh;
        Shaders::Phong& _shader;
        Color4 _color;

};

class FirstScene: public Platform::Application {
    public:
        explicit FirstScene(const Arguments& arguments);

    private:
        void drawEvent() override;
		/*void mousePressEvent(MouseEvent& event) override;
		void mouseMoveEvent(MouseMoveEvent& event) override;
		void mouseReleaseEvent(MouseEvent& event) override;*/

        Scene3D _scene;
        Object3D* _cameraObject;
        SceneGraph::Camera3D* _camera;
        SceneGraph::DrawableGroup3D _drawables;

        GL::Mesh _sphere;
        Shaders::Phong _shader;

		enum { ObjectCount = 3 };
		ColoredDrawable* _objects[ObjectCount];

		//GL::Framebuffer _framebuffer;

		//Vector2i _previousMousePosition, _mousePressPosition;
};


FirstScene::FirstScene(const Arguments& arguments): Platform::Application{arguments} {
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

    _cameraObject = new Object3D{&_scene};
    (*_cameraObject)
        .translate(Vector3::zAxis(8.0f))
        .rotateX(-35.0_degf)
        .rotateY(40.0_degf);
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    (*_camera)
        .setProjectionMatrix(Matrix4::perspectiveProjection(
            35.0_degf, 1.3333f, 0.01f, 100.0f));

	_sphere = MeshTools::compile(Primitives::uvSphereSolid(16, 32));
	(*(_objects[0]= new ColoredDrawable{ *object, _sphere, _shader,
		0xde3038_rgbf, &_drawables };)
		.translate(Vector3::xAxis(6))
		.scale(Vector3(.1, .1, .1));
    new ColoredDrawable{*object, _sphere, _shader,
        0xde3038_rgbf, &_drawables};

	auto* object2 = new Object3D{ &_scene };
	(*object2)
		.translate(Vector3::xAxis(-6))
		.scale(Vector3(.1, .1, .1));
	new ColoredDrawable{ *object2, _sphere, _shader,
		0xde3038_rgbf, &_drawables };

	auto* object3 = new Object3D{ &_scene };
	(*object3)
		.translate(Vector3::yAxis(3))
		.scale(Vector3(.1, .1, .1));
	new ColoredDrawable{ *object3, _sphere, _shader,
		0xde3038_rgbf, &_drawables };
}

ColoredDrawable::ColoredDrawable(Object3D& object, GL::Mesh& mesh, Shaders::Phong& shader, const Color4& color, SceneGraph::DrawableGroup3D* drawables) :  SceneGraph::Drawable3D{ object, drawables }, _mesh{ mesh }, _shader{ shader }, _color{ color }, _selected{false} {}

void ColoredDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) {
    _shader
        .setDiffuseColor(_color*(_selected ? 2.0f : 1.0f))
        .setLightPosition(camera.cameraMatrix()
            .transformPoint({2.0f, 3.0f, -1.0f}))
        .setTransformationMatrix(transformationMatrix)
        .setNormalMatrix(transformationMatrix.rotationScaling())
        .setProjectionMatrix(camera.projectionMatrix())
		.setAmbientColor(_selected ? _color * 0.3f : Color3{});

    _mesh.draw(_shader);
}

void FirstScene::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    _camera->draw(_drawables);

    swapBuffers();
}

//void FirstScene::mousePressEvent(MouseEvent& event) {
//	if (event.button() != MouseEvent::Button::Left) return;
//
//	_previousMousePosition = _mousePressPosition = event.position();
//	event.setAccepted();
//}
//
//void FirstScene::mouseMoveEvent(MouseMoveEvent& event) {
//	if (!(event.buttons() & MouseMoveEvent::Button::Left)) return;
//
//	const Vector2 delta = 3.0f*
//		Vector2{ event.position() - _previousMousePosition } /
//		Vector2{ GL::defaultFramebuffer.viewport().size() };
//
//	(*_cameraObject)
//		.rotate(Rad{ -delta.y() }, _cameraObject->transformation().right().normalized())
//		.rotateY(Rad{ -delta.x() });
//
//	_previousMousePosition = event.position();
//	event.setAccepted();
//	redraw();
//}

//void FirstScene::mouseReleaseEvent(MouseEvent& event) {
//	if (event.button() != MouseEvent::Button::Left || _mousePressPosition != event.position()) return;
//
//	/* Read object ID at given click position (framebuffer has Y up while windowing system Y down) */
//	_framebuffer.mapForRead(GL::Framebuffer::ColorAttachment{ 1 });
//	Image2D data = _framebuffer.read(
//		Range2Di::fromSize({ event.position().x(), _framebuffer.viewport().sizeY() - event.position().y() - 1 }, { 1, 1 }),
//		{ PixelFormat::R8UI });
//
//	/* Highlight object under mouse and deselect all other */
//	for (auto* o : _objects) o->setSelected(false);
//	UnsignedByte id = data.data<UnsignedByte>()[0];
//	if (id > 0 && id < ObjectCount + 1)
//		_objects[id - 1]->setSelected(true);
//
//	event.setAccepted();
//	redraw();
//}

MAGNUM_APPLICATION_MAIN(FirstScene)
