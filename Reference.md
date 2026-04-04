# **TXGame Reference**

# TXGraphics (`tx::RenderEngine`)
**Example Usage:**
```c++
re::RE re;
re::RSP rr;

bool init() {
	re.init();
	rr = re.createSectionProxy(re::readShaderSource("vertex.vert"), re::readShaderSource("fragment.frag"));
	return 1;
}
```

# Platformer
**Example Usage:**
```c++
gm::Platformer platform;

bool init() {
	platform = gm::Platformer([&](gm::Platformer::Initer& initer) {
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
	return 1;
}
```



# Config management system

**Example Usage:**
```c++
re::TextureId down, up;
gm::ConfigClass config;

bool init() {
	config = gm::ConfigClass([&](gm::ConfigIniter& initer) {
		initer.addSource(tx::parseJson(tx::readWholeFileText(tx::getExeDir() / "config/art.json")));
		initer.addSource(tx::parseJson(tx::readWholeFileText(tx::getExeDir() / "config/rules.json")));
	});
	down = re::REAddTexture(config.art[config.unit[0].art].down, re);
	up = re::REAddTexture(config.art[config.unit[0].art].up, re);
	return 1;
}
```
Json:
```json
// art.json
{
	"testing":{
		"purposes":{
			"Player": {
				"txgame.type": "Art",
				"down": "/home/TX_Jerry/Downloads/Yafan Sprites/slip_1_copy.jpg",
				"up": "/home/TX_Jerry/Downloads/Yafan Sprites/slip_4_copy.jpg"
			}
		}
	}
}
// rules.json
{
	"Player": {
		"txgame.type": "Unit",
		"art": "testing/purposes/Player"
	}
}
```