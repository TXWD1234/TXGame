#include "utility_dump.hpp"

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
				// TODO: Optional warning logging here about missing reference
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
};