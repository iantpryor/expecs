#pragma once
#include "entity_manager.h"

#include <ranges>
#include <queue>
#include <vector>
#include <unordered_map>
#include <cassert>

namespace expecs
{
	class ComponentPoolBase
	{
	public:
		ComponentPoolBase() = default;
		virtual ~ComponentPoolBase() = default;

		virtual void entityDestroyed(Entity entity) = 0;
		virtual void removeComponent(Entity entity) = 0;
		virtual bool hasComponent(Entity entity) = 0;
	};

	template<typename T>
	class ComponentPool : public ComponentPoolBase
	{
	public:
		ComponentPool()
		{
			_componentData.reserve(MAX_ENTITIES);
			_entityToIndexMap.reserve(MAX_ENTITIES);
			_indexToEntityMap.reserve(MAX_ENTITIES);
		}
		~ComponentPool() = default;

		T& getComponent(Entity entity)
		{
			auto it = _entityToIndexMap.find(entity);
			return _componentData.at(it->second);
		}

		const std::vector<Entity> getEntitiesWithComponent()
		{
			auto keyView = std::views::keys(_entityToIndexMap);
			return std::vector<Entity>{ keyView.begin(), keyView.end() };
		}

		bool hasComponent(Entity entity)
		{
			return _entityToIndexMap.contains(entity);
		}

		T& addComponent(Entity entity, const T& component)
		{
			size_t newIndex = _componentData.size();
			_componentData.push_back(component);

			_entityToIndexMap[entity] = newIndex;
			_indexToEntityMap[newIndex] = entity;

			return _componentData[newIndex];
		}

		void removeComponent(Entity entity) override
		{
			auto it = _entityToIndexMap.find(entity);

			size_t indexToRemove = it->second;
			size_t lastIndex = _componentData.size() - 1;

			if (indexToRemove != lastIndex) 
			{
				// Move last element to the position of removed element
				Entity lastEntity = _indexToEntityMap[lastIndex];

				_componentData[indexToRemove] = std::move(_componentData[lastIndex]);
				_indexToEntityMap[indexToRemove] = lastEntity;
				_entityToIndexMap[lastEntity] = indexToRemove;
			}

			// Remove the last element
			_componentData.pop_back();
			_entityToIndexMap.erase(entity);
			_indexToEntityMap.erase(lastIndex);
		}

		void entityDestroyed(Entity entity) override
		{
			if (_entityToIndexMap.find(entity) != _entityToIndexMap.end())
			{
				removeComponent(entity);
			}
		}

	private:
		std::vector<T> _componentData = {};
		std::unordered_map<Entity, size_t> _entityToIndexMap = {};
		std::unordered_map<size_t, Entity> _indexToEntityMap = {};
	};
}
