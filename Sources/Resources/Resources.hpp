#pragma once

#include "Engine/Engine.hpp"
#include "Helpers/ThreadPool.hpp"
#include "Files/Node.hpp"
#include "Resource.hpp"

namespace acid {
/**
 * @brief Module used for managing resources. Resources are held alive as long as they are in use,
 * a existing resource is queried by node value.
 */
class ACID_EXPORT Resources : public Module::Registrar<Resources, Module::Stage::Post> {
public:
	Resources();

	void Update() override;

	std::shared_ptr<Resource> Find(const std::type_index &typeIndex, const Node &node) const;

	template<typename T>
	std::shared_ptr<T> Find(const Node &node) const {
		if (m_resources.find(typeid(T)) == m_resources.end())
			return nullptr;
		
		for (const auto &[key, resource] : m_resources.at(typeid(T))) {
			if (key == node)
				return std::dynamic_pointer_cast<T>(resource);
		}
		
		return nullptr;
	}
	
	void Add(const Node &node, const std::shared_ptr<Resource> &resource);
	void Remove(const std::shared_ptr<Resource> &resource);

	/**
	 * Gets the resource loader thread pool.
	 * @return The resource loader thread pool.
	 */
	ThreadPool &GetThreadPool() { return m_threadPool; }

private:
	std::unordered_map<std::type_index, std::map<Node, std::shared_ptr<Resource>>> m_resources;
	ElapsedTime m_elapsedPurge;

	ThreadPool m_threadPool;
};
}
