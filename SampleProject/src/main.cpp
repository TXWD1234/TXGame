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
	Platformer engine;

private:
	bool init() {
		tx::glfwSetKeyCallback<Application, &Application::onKeyEvent>(framework.getWindow(), this);
		tx::glBasicSettings();
		//stbi_set_flip_vertically_on_load(true);
		glfwSwapInterval(0); // turn off vsync

		re.init();
		rr = re.createSectionProxy(re::readShaderSource("vertex.vert"), re::readShaderSource("fragment.frag"));

		engine = Platformer([&](Platformer::Initer& initer) {
			initer.addBox(tx::BottomLeft, 2.0f, 2.0f, false);

			// Parkour Map!
			// 1. Starting platform
			initer.addHorizontal(-1.0f, -0.6f, -0.8f, true);

			// 2. First jump
			initer.addHorizontal(-0.4f, 0.0f, -0.65f, true);
			initer.addHorizontal(-0.4f, 0.0f, -0.65f, false);

			// 3. Second jump
			initer.addHorizontal(0.3f, 0.7f, -0.5f, true);

			// 4. Wrap around a wall
			//initer.addVertical(-0.5f, 0.0f, 0.7f, false); // Block right
			initer.addHorizontal(0.8f, 1.0f, -0.3f, true); // Safe spot

			// 5. Jump back left
			initer.addHorizontal(0.2f, 0.5f, -0.1f, true);
			initer.addHorizontal(-0.4f, 0.0f, 0.1f, true);

			// 6. Tiny precision jump
			initer.addHorizontal(-0.8f, -0.7f, 0.3f, true);

			// 7. Goal platform with a back wall
			initer.addHorizontal(-0.2f, 0.8f, 0.5f, true);
			initer.addVertical(0.5f, 0.8f, 0.8f, false);
		});

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

	tx::Rect player{ { -0.9f, -0.7f }, 0.1f, 0.1f };
	tx::vec2 playerVelocity;
	float playerSpeed = 0.02f, playerJumpSpeed = 0.05f;
	const float Gravity = 0.005f;

	tx::u64 tickCounter = 0;
	void update() {
		// change of velocity
		if (!engine.objectOnFloor(player)) {
			playerVelocity.y -= Gravity;
		} else {
			playerVelocity.y = 0.0f;
			if (glfwGetKey(framework.getWindow(), GLFW_KEY_UP)) {
				// jump
				playerVelocity.y = playerJumpSpeed;
			}
		}

		// current movement

		tx::vec2 movement = playerVelocity;

		if (glfwGetKey(framework.getWindow(), GLFW_KEY_LEFT)) {
			movement.x = -playerSpeed;
		}
		if (glfwGetKey(framework.getWindow(), GLFW_KEY_RIGHT)) {
			movement.x = playerSpeed;
		}

		engine.objectMoveConstrainted(player, movement);

		// "The floor is lava" - reset if fallen off the map
		if (player.bottom() < -1.1f) {
			player.setPos(-0.9f, -0.7f); // Reset to start
			playerVelocity.y = 0.0f;
		}
	}
	void render() {
		rr.drawSprite(player.center(), 0, 0.0f, player.dimension(), tx::MikuColor.compress());
		engine.drawDebugLines([&](tx::vec2 start, tx::vec2 end) {
			rr.drawLine(start, end);
		});
		re.draw();
	}
};

int main() {
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