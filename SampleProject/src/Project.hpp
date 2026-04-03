#include "utility_dump.hpp"

struct ConfigObj {
	std::string_view regName;
	const tx::JsonObject* content;
	tx::u32 handlerIndex = 0;
};
struct ConfigDir {
	std::string_view regName;
	tx::KVMap<std::string, ConfigDir> m_dirs;
	tx::KVMap<std::string, ConfigObj> m_objs;
};

// pure virtual class
class ConfigHandlerBase {
public:
	virtual ~ConfigHandlerBase() = default;
	virtual void handleConfig(const tx::JsonObject&, std::string_view regName, const ConfigDir& rootDir) = 0;
	virtual std::string_view getRegName() = 0;
};


class ConfigManager {
public:
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

private:
private:
	// constants
	inline constexpr static const char* TypeIndicator = "txgame.type";


public:
	static void configure(const ConfigContent& content) {
		ConfigDir root;
		constructStructure_impl(root, content.root, content);
		foreachObj_impl(root, [&](const ConfigObj& obj) {
			content.handlers[obj.handlerIndex]->handleConfig(*obj.content, obj.regName, root);
		});
	}
	template <std::invocable<ConfigContent&> Func>
	static void configure(Func&& f) {
		ConfigContent content;
		f(content);
		configure(content);
	}

private:
	static void constructStructure_impl(ConfigDir& root, const tx::JsonObject& json, const ConfigContent& content) {
		for (const tx::JsonPair& i : json) {
			if (!i.v().is<tx::JsonObject>()) continue;
			const tx::JsonObject& entry = i.v().get<tx::JsonObject>();
			if (entry.exist(TypeIndicator)) { // is a object
				const tx::JsonValue& typeVal = entry.get(TypeIndicator);
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