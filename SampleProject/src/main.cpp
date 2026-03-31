#include "Project.hpp"
#include <concepts>

class Application {
private:
	struct UpdateFunc {
		Application* ptr;
		inline void operator()() {
			ptr->update();
		}
	};
	struct RenderFunc {
		Application* ptr;
		inline void operator()() {
			ptr->render();
		}
	};
	tx::RE::Framework<re::Mode::FixTickRate | re::Mode::PrintFrameRate, UpdateFunc, RenderFunc> framework;

	bool initGLFW() {
		std::cout << "Initializing GLFW...\n";
		if (!glfwInit()) {
			std::cerr << "[FatalError]: Failed to init GLFW\n";
			return false;
		}
		return true;
	}
	bool initGLAD() {
		std::cout << "Initializing GLAD...\n";
		if (!gl::init((void*)glfwGetProcAddress)) {
			std::cerr << "[FatalError]: Failed to init GLAD\n";
			return false;
		}
		return true;
	}

	bool m_valid = 0;

public:
	Application() {
		if (!initGLFW()) return;
		framework = decltype(framework){
			UpdateFunc{ this },
			RenderFunc{ this }
		};
		if (!framework.valid()) return;
		if (!initGLAD()) return;
		if (!init()) return;
		m_valid = 1;
	}
	~Application() {
		glfwTerminate();
	}

	void run() { this->framework.run(); }
	bool valid() const { return m_valid; }

private:
	void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			switch (key) {
			}
		}
	}

	re::RE re;
	re::RSP rr;
	tx::Animation anim{ 3 };

private:
	bool init() {
		tx::glfwSetKeyCallback<Application, &Application::onKeyEvent>(framework.getWindow(), this);
		tx::glBasicSettings();
		//stbi_set_flip_vertically_on_load(true);
		//glfwSwapInterval(0); // turn off vsync

		re.init();
		rr = re.createSectionProxy(re::readShaderSource("vertex.vert"), re::readShaderSource("fragment.frag"));


		// int width, height, channels;
		// std::vector<tx::u8*> data;
		// data.reserve(11);

		// for (int i = 1; i <= 12; i++) {
		// 	if (i == 4) continue; // watch who ever is reading this code be so confusing...
		// 	std::ostringstream oss;
		// 	oss << "/home/TX_Jerry/Desktop/mpv-shot00" << std::setw(2) << std::setfill('0') << i << ".jpg";
		// 	std::string path = oss.str();
		// 	data.push_back(stbi_load(path.c_str(), &width, &height, &channels, 4));
		// 	if (!data.back()) {
		// 		std::cerr << "[Error]: stb_image failed to load image in frame: " << i << endl;
		// 		return 0;
		// 	}
		// }
		// tx::u32 length = width * height * 4;

		// for (int i = 0; i < data.size(); i++) {
		// 	anim.addFrame(
		// 	    re.addTexture({ width, height }, std::bitSpan(data[i], length)));
		// 	if (data[i]) stbi_image_free(data[i]);
		// }

		return 1;
	}

	tx::u32 frameCounter = 0;
	tx::u32 imageCount = 15;


	const float scaleIncreaseMult = 1.067f;
	const float scaleDecreaseMult = 0.9f;
	float scale = 0.1f, currentMult = scaleIncreaseMult;
	float degree = 0.0f, rotationSpeed = tx::ONE_DEGREE * -5.0f, degreeMax = 2 * tx::PI;
	tx::Rainbow colorEngine = tx::Rainbow(36);

	tx::u64 tickCounter = 0;
	void update() {
		// anim frame
		if (!(tickCounter % 3)) {
			frameCounter++;
			if (frameCounter >= 11) {
				frameCounter = 0;
			}
		}
		// scale
		scale *= currentMult;
		if (scale >= 2.0f) {
			currentMult = scaleDecreaseMult;
		} else if (scale <= 0.5f) {
			currentMult = scaleIncreaseMult;
		}
		// rotation
		degree += rotationSpeed;
		if (degree >= degreeMax) degree -= degreeMax;

		tickCounter++;
	}
	void render() {
		rr.drawSprite(tx::Origin, 0, 0.0f, tx::vec2{ 1.0f, 1.0f }, 0, colorEngine.getNextColor().compress());
		re.draw();
	}
};

int main() {

	std::vector<int> ids = { 3, 1, 4, 2 };
	std::vector<std::string> val1 = { "Three", "One", "Four", "Two" };
	std::vector<double> val2 = { 3.3, 1.1, 4.4, 2.2 };

	// Sort by ID ascending
	tx::multi_sort(ids.begin(), ids.end(), std::less<>(), val1.begin(), val2.begin());

	for (size_t i = 0; i < ids.size(); ++i) {
		std::cout << ids[i] << ": " << val1[i] << ", " << val2[i] << "\n";
	}
	// Output:
	// 1: One, 1.1
	// 2: Two, 2.2
	// 3: Three, 3.3
	// 4: Four, 4.4







	return 0;
	std::cout << "Initializing Application...\n";
	Application app;
	if (!app.valid()) {
		std::cerr << "[FatalError]: Failed to init Application\n";
		return 1;
	}

	std::cout << "[Status]: Main Loop Starts\n";
	app.run();

	std::cout << "[Status]: Terminating...\n";
	return 0;
}