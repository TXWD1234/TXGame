#include "utility_dump.hpp"

namespace txgame {

struct ConfigObj {
	std::string_view regName;
	const tx::JsonObject* content;
	tx::u32 handlerIndex = 0;
};
struct ConfigDir {
	std::string_view regName;
	tx::KVMap<std::string_view, ConfigDir> m_dirs;
	tx::KVMap<std::string_view, ConfigObj> m_objs;
};

class ConfigContent;
// pure virtual class
class ConfigHandlerBase {
public:
	virtual ~ConfigHandlerBase() = default;
	virtual void handleConfig(const tx::JsonObject&, std::string_view, const ConfigDir&, const ConfigContent&) = 0;
	virtual std::string_view getRegName() = 0;
	virtual tx::u32 getObjectIndex(std::string_view) = 0;
};

// Transparent lookup utilities for unordered_map
struct HashStringView {
	using is_transparent = void;
	size_t operator()(std::string_view s) const {
		return std::hash<std::string_view>{}(s);
	}
};
struct EqualStringView {
	using is_transparent = void;
	bool operator()(std::string_view a, std::string_view b) const {
		return a == b;
	}
};


class ConfigManager;
class ConfigContent {
	friend class ConfigManager;

public:
	void addHandler(ConfigHandlerBase& handler) {
		if (!handlerIndexMap.count(handler.getRegName())) {
			handlerIndexMap.insert({ std::string(handler.getRegName()), static_cast<tx::u32>(handlers.size()) });
			handlers.push_back(&handler);
		}
	}

	void addSource(const tx::JsonObject& source) {
		root.merge(source, tx::JsonMergeReplace{});
	}
	void addSource(tx::JsonObject&& source) {
		root.merge(std::move(source), tx::JsonMergeReplace{});
	}

private:
	std::vector<ConfigHandlerBase*> handlers;
	std::unordered_map<std::string, tx::u32, HashStringView, EqualStringView> handlerIndexMap;
	tx::JsonObject root;

	// functions for ConfigManager

	tx::u32 getHandlerIndex(std::string_view name) const {
		auto it = handlerIndexMap.find(name);
		if (it == handlerIndexMap.end()) return tx::InvalidU32;
		return it->second;
	}
};

struct ConfigObjectHandle {
	ConfigHandlerBase* handler = nullptr;
	std::string_view regName;
};

class ConfigManager {
private:
	// constants
	inline constexpr static const char* TypeIndicator = "txgame.type";


public:
	static void configure(const ConfigContent& content) {
		ConfigDir root;
		constructStructure_impl(root, content.root, content);
		foreachObj_impl(root, [&](const ConfigObj& obj) {
			content.handlers[obj.handlerIndex]->handleConfig(*obj.content, obj.regName, root, content);
		});
	}
	template <std::invocable<ConfigContent&> Func>
	static void configure(Func&& f) {
		ConfigContent content;
		f(content);
		configure(content);
	}

	// utility functions

	static ConfigObjectHandle findObject(std::string_view path, const ConfigContent& content, const ConfigDir& root) {
		const ConfigDir* parent = &root;
		size_t previous = 0;
		size_t current = path.find('/');
		while (current != std::string::npos) {
			auto it = parent->m_dirs.find(path.substr(previous, current - previous));
			if (it == parent->m_dirs.end()) return ConfigObjectHandle{ nullptr, "" };

			parent = &it->v();

			previous = current + 1;
			current = path.find('/', previous);
		}
		current = path.size();

		auto it = parent->m_objs.find(path.substr(previous, current - previous));
		if (it == parent->m_objs.end()) return ConfigObjectHandle{ nullptr, "" };
		return ConfigObjectHandle{ content.handlers[it->v().handlerIndex], it->v().regName };
	}


private:
	static void constructStructure_impl(ConfigDir& root, const tx::JsonObject& json, const ConfigContent& content) {
		for (const tx::JsonPair& i : json) {
			if (!i.v().is<tx::JsonObject>()) continue;
			const tx::JsonObject& entry = i.v().get<tx::JsonObject>();
			auto it = entry.find(TypeIndicator);
			if (it != entry.end()) { // is a object
				const tx::JsonValue& typeVal = it->v();
				if (typeVal.is<std::string>()) {
					tx::u32 handlerIndex = content.getHandlerIndex(typeVal.get<std::string>());
					if (handlerIndex != tx::InvalidU32) {
						root.m_objs.insertMulti(i.k(), ConfigObj{ i.k(), &entry, handlerIndex });
					}
					// optionally log a warning here if handlerIdx is InvalidU32 <-------------------------------------
				}
			} else { // is a dir
				tx::KVMapHandle handle = root.m_dirs.insertMulti(i.k());
				constructStructure_impl(handle.get(), entry, content);
			}
		}
		root.m_dirs.validate();
		root.m_objs.validate();
	}
	template <std::invocable<const ConfigObj&> Func>
	static void foreachObj_impl(const ConfigDir& dir, Func&& f) {
		dir.m_dirs.foreach ([&](const ConfigDir& i) {
			foreachObj_impl(i, f);
		});
		dir.m_objs.foreach ([&](const ConfigObj& i) {
			f(i);
		});
	}
};


class ConfigHandler_Art;
class Config_Art {
	friend class ConfigHandler_Art;

public:
	using Handler_t = ConfigHandler_Art;

	// user specific entries
	struct Object {
		// all members here will be user specific
		std::string down, up;
	};

public:
	Object& operator[](tx::u32 index) { return entries[index]; }
	const Object& operator[](tx::u32 index) const { return entries[index]; }

private:
	std::vector<Object> entries;
};


class ConfigHandler_Art : public ConfigHandlerBase {
public:
	ConfigHandler_Art(Config_Art& config) : m_config(&config) {}

	// ConfigManager (init time) oriented virtual functions

	void handleConfig(const tx::JsonObject& config, std::string_view regName, const ConfigDir& rootDir, const ConfigContent& content) override {
		// hard coded, user specific code
		regNameIndexMap.insert({ regName, static_cast<tx::u32>(m_config->entries.size()) });
		m_config->entries.emplace_back();
		auto downIt = config.find("down");
		if (downIt != config.end() && downIt->v().is<std::string>()) {
			m_config->entries.back().down = downIt->v().get<std::string>();
		}
		auto upIt = config.find("up");
		if (upIt != config.end() && upIt->v().is<std::string>()) {
			m_config->entries.back().up = upIt->v().get<std::string>();
		}
	}
	std::string_view getRegName() override {
		return "Art";
	}

	// user (other handlers) oriented API functions

	tx::u32 getObjectIndex(std::string_view regName) override {
		auto it = regNameIndexMap.find(regName);
		if (it == regNameIndexMap.end()) return tx::InvalidU32;
		return it->second;
	}

private:
	Config_Art* m_config;
	std::unordered_map<std::string_view, tx::u32> regNameIndexMap;
};




class ConfigHandler_Unit;
class Config_Unit {
	friend class ConfigHandler_Unit;

public:
	using Handler_t = ConfigHandler_Unit;

	// user specific entries
	struct Object {
		// all members here will be user specific
		tx::u32 art;
	};

public:
	Object& operator[](tx::u32 index) { return entries[index]; }
	const Object& operator[](tx::u32 index) const { return entries[index]; }

private:
	std::vector<Object> entries;
};


class ConfigHandler_Unit : public ConfigHandlerBase {
public:
	ConfigHandler_Unit(Config_Unit& config) : m_config(&config) {}

	// ConfigManager (init time) oriented virtual functions

	void handleConfig(const tx::JsonObject& config, std::string_view regName, const ConfigDir& rootDir, const ConfigContent& content) override {
		// hard coded, user specific code
		regNameIndexMap.insert({ regName, static_cast<tx::u32>(m_config->entries.size()) });
		m_config->entries.emplace_back();
		auto artIt = config.find("art");
		if (artIt != config.end() && artIt->v().is<std::string>()) {
			ConfigObjectHandle obj = ConfigManager::findObject(artIt->v().get<std::string>(), content, rootDir);
			if (obj.handler) {
				m_config->entries.back().art = obj.handler->getObjectIndex(obj.regName);
			} else {
				// TODO: Optional warning logging here about missing reference <----------------------------------------------------------------------
			}
		}
	}
	std::string_view getRegName() override {
		return "Unit";
	}

	// user (other handlers) oriented API functions

	tx::u32 getObjectIndex(std::string_view regName) override {
		auto it = regNameIndexMap.find(regName);
		if (it == regNameIndexMap.end()) return tx::InvalidU32;
		return it->second;
	}

private:
	Config_Unit* m_config;
	std::unordered_map<std::string_view, tx::u32> regNameIndexMap;
};

struct ConfigClass {
	Config_Art art;
	Config_Unit unit;

	struct ConfigIniter {
		friend class ConfigClass;

		template <class... Args>
		void addSource(Args&&... args) {
			content->addSource(args...);
		}

	private:
		ConfigContent* content;
		ConfigIniter(ConfigContent& in) : content(&in) {}
	};
	template <std::invocable<ConfigIniter&> Func>
	ConfigClass(Func&& f) {
		Config_Art::Handler_t handler_art{ art };
		Config_Unit::Handler_t handler_unit{ unit };
		ConfigContent content;
		ConfigIniter initer{ content };
		f(initer);
		content.addHandler(handler_art);
		content.addHandler(handler_unit);
		ConfigManager::configure(content);
	}
	ConfigClass() = default;
};
using ConfigIniter = ConfigClass::ConfigIniter;


// be aware of the double-deletion problem, and the dangling reference of the deleted handles
class HandleSystem {
public:
	tx::u32 addHandle() {
		if (!m_availableHandleBuffer.empty()) return pop_back();
		return m_handleMax++;
	}
	void deleteHandle(tx::u32 handle) {
		if (handle < m_handleMax) m_availableHandleBuffer.push_back(handle);
	}

	tx::u32 count() { return m_handleMax - static_cast<tx::u32>(m_availableHandleBuffer.size()); }

private:
	std::vector<tx::u32> m_availableHandleBuffer;
	tx::u32 m_handleMax = 0; // aka the next handle

	tx::u32 pop_back() {
		tx::u32 back = m_availableHandleBuffer.back();
		m_availableHandleBuffer.pop_back();
		return back;
	}
};
template <class T>
class HandleContainer {
public:
	tx::u32 addHandle(const T& value) {
		tx::u32 handle = m_hs.addHandle();
		resizeToFit(handle);
		m_handleValues[handle] = value;
		return handle;
	}
	tx::u32 addHandle(T&& value) {
		tx::u32 handle = m_hs.addHandle();
		resizeToFit(handle);
		m_handleValues[handle] = std::move(value);
		return handle;
	}

	void deleteHandle(tx::u32 handle) {
		if (handle < m_handleValues.size()) m_handleValues[handle] = T{};
		m_hs.deleteHandle(handle);
	}

	T& operator[](tx::u32 handle) { return m_handleValues[handle]; }
	const T& operator[](tx::u32 handle) const { return m_handleValues[handle]; }

private:
	HandleSystem m_hs;
	std::vector<T> m_handleValues;

	void resizeToFit(tx::u32 index) {
		if (index >= m_handleValues.size())
			m_handleValues.resize(index + 1);
	}
};















// animman ***************************************************************************

struct AnimationId {
	tx::u32 index;
};
using AnimId = AnimationId;

class AnimationManager {
	// Terminology:
	// play = playing animation
private:
	using id = tx::RenderEngine::TextureId;
	using u32 = tx::u32;
	using u16 = tx::u16;
	using u8 = tx::u8;

	struct AnimMeta_impl {
		u32 offset;
		u16 len;
		u8 frameInterval, repeat;
	};
	struct AnimCounter_impl {
		u16 counter;
		u16 counterMax;
	};
	struct AnimPlayMeta_impl {
		std::vector<u32> id;
		std::vector<AnimCounter_impl> frameCounter;
		std::vector<AnimCounter_impl> intervalCounter;
	};

public:
	AnimationManager() {
	}


	// @param frameRate means the rate of update, aka the interval of frame update, pass 0 means update every frame.
	template <std::invocable<std::span<id>> Func>
	AnimationId addAnimation(u8 frameRate, u16 length, bool repeat, Func&& f) {
		m_animations.push_back({ static_cast<u32>(m_frames.size()), length, frameRate, static_cast<u8>(repeat) });
		u32 oldSize = static_cast<u32>(m_frames.size());
		m_frames.resize(m_frames.size() + length);
		f(std::span<id>{ m_frames.begin() + oldSize, m_frames.end() });
		return { static_cast<u32>(m_animations.size() - 1) };
	}

	void reserveFrames(u32 count) { m_frames.reserve(count); }


	void update() {
		// Update non-repeating animations
		for (u32 i = 0; i < m_nonRepeatPlays.intervalCounter.size();) {
			auto& intervalCounter = m_nonRepeatPlays.intervalCounter[i];
			intervalCounter.counter++;
			if (intervalCounter.counter >= intervalCounter.counterMax) { // update frame counter
				intervalCounter.counter = 0; // reset interval counter
				auto& frameCounter = m_nonRepeatPlays.frameCounter[i];
				frameCounter.counter++;
				if (frameCounter.counter >= frameCounter.counterMax) { // end of one run of the anim
					removeAnimPlay_impl(m_nonRepeatPlays, i);
					continue; // Skip incrementing so we process the swapped-in element
				}
			}
			i++;
		}

		// Update repeating animations
		for (u32 i = 0; i < m_repeatPlays.intervalCounter.size(); i++) {
			auto& intervalCounter = m_repeatPlays.intervalCounter[i];
			intervalCounter.counter++;
			if (intervalCounter.counter >= intervalCounter.counterMax) { // update frame counter
				intervalCounter.counter = 0; // reset interval counter
				auto& frameCounter = m_repeatPlays.frameCounter[i];
				frameCounter.counter++;
				if (frameCounter.counter >= frameCounter.counterMax) { // end of one run of the anim
					frameCounter.counter = 0; // reset frame counter
				}
			}
		}
	}

	// @brief add a playing animation into the playing animation buffer (aka add a playing anim instance)
	// @return the id of the animPlay. the highest bit of the id is `isRepeat`
	u32 play(AnimationId animId) {
		if (m_animations[animId.index].repeat) {
			insertAnimPlay_impl(m_repeatPlays, animId.index);

			return static_cast<u32>(m_repeatPlays.id.size() - 1) | 0x80000000;
		} else {
			insertAnimPlay_impl(m_nonRepeatPlays, animId.index);
			return static_cast<u32>(m_nonRepeatPlays.id.size() - 1);
		}
	}
	void stop(u32 index) {
		if (index & 0x80000000) {
			removeAnimPlay_impl(m_repeatPlays, index & ~0x80000000);
		} else {
			removeAnimPlay_impl(m_nonRepeatPlays, index);
		}
	}
	void stopAll(AnimationId id) {
		for (u32 i = 0; i < m_nonRepeatPlays.id.size();) {
			if (m_nonRepeatPlays.id[i] == id.index)
				removeAnimPlay_impl(m_nonRepeatPlays, i);
			else
				i++;
		}
		for (u32 i = 0; i < m_repeatPlays.id.size();) {
			if (m_repeatPlays.id[i] == id.index)
				removeAnimPlay_impl(m_repeatPlays, i);
			else
				i++;
		}
	}

	// Get the current TextureId for a actively playing animation instance
	id getFrame(u32 playIndex) const {
		const AnimPlayMeta_impl& buffer = (playIndex & 0x80000000) ? m_repeatPlays : m_nonRepeatPlays;
		playIndex &= ~0x80000000;

		u32 animId = buffer.id[playIndex];
		u32 frameOffset = m_animations[animId].offset;
		u16 currentFrame = buffer.frameCounter[playIndex].counter;
		return m_frames[frameOffset + currentFrame];
	}







private:
	std::vector<id> m_frames;
	std::vector<AnimMeta_impl> m_animations;
	AnimPlayMeta_impl m_repeatPlays;
	AnimPlayMeta_impl m_nonRepeatPlays;
	std::vector<u32> m_repeatIndexMap; // maps the returned handle of a play with the physical index of the the play


	void insertAnimPlay_impl(AnimPlayMeta_impl& buffer, u32 id) {
		buffer.id.push_back(id);
		buffer.frameCounter.push_back({ 0, m_animations[id].len });
		buffer.intervalCounter.push_back({ 0, m_animations[id].frameInterval });
	}

	template <class T>
	void removeElement_impl(std::vector<T>& arr, u32 index) {
		arr[index] = std::move(arr.back());
		arr.pop_back();
	}
	void removeAnimPlay_impl(AnimPlayMeta_impl& buffer, u32 index) {
		removeElement_impl(buffer.frameCounter, index);
		removeElement_impl(buffer.id, index);
		removeElement_impl(buffer.intervalCounter, index);
	}
};



class AssetManager {
public:
private:
	re::RenderEngine* re;
};

} // namespace txgame


class Game {
public:
	Game() = default;
	void init(tx::u32 sideLen) {
		nodes.reinit(sideLen);
		re.init();
		rr = re.createSectionProxy(re::readShaderSource("vertex.vert"), re::readShaderSource("fragment.frag"));
	}

	void update() {
	}
	void render() {
		drawLines();
		re.draw();
	}


private:
	struct Node {
		tx::u32 health = 0;
		tx::u32 level = 0;
		tx::u32 occupyed = 0;
		tx::u32 owner = 0;
		std::array<tx::u32, 3> adjacent;
	};

	enum class GeneralState {
		Normal,
		Select
	};


private:
	// game
	tx::GridSystem<Node> nodes;
	float GridSize;

	// render
	re::RenderEngine re;
	re::RSP rr;


	void drawLines() {
		float increment = 2.0f / nodes.getWidth();
		float current = -1.0f;
		for (int i = 0; i < nodes.getWidth(); i++) {
			current += increment;
			rr.drawLineHorizontal(current);
			rr.drawLineVertical(current);
		}
	}
	void drawGrid(Node& obj, tx::vec2 pos) {
	}
	void drawGrids() {
	}
};