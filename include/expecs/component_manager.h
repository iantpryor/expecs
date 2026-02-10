#pragma once
#include "component_pool.h"

#include <memory>
#include <unordered_map>
#include <typeindex>

namespace expecs
{
	using ComponentType = uint8_t;
	constexpr uint8_t MAX_COMPONENTS = 32;

	class ComponentManager
	{
	public:
		ComponentManager() = default;
		~ComponentManager() = default;

		template<typename T>
		Signature registerComponentType()
		{
			_componentPools.push_back(std::make_unique<ComponentPool<T>>());

			_typeMap[std::type_index(typeid(T))] = _currentComponentType;
			Signature componentSignature = 0;
			componentSignature |= (1 << _currentComponentType);

			_currentComponentType++;

			return componentSignature;
		}

		template<typename T>
		Signature getSignature() const
		{
			Signature componentSignature = 0;
			if (_typeMap.contains(std::type_index(typeid(T))))
			{
				componentSignature |= (1 << _typeMap.at(std::type_index(typeid(T))));
			}

			return componentSignature;
		}

		template<typename T>
		ComponentType getComponentType()
		{
			return _typeMap.at(std::type_index(typeid(T)));
		}

		std::vector<ComponentType> getRegisteredComponentTypes() const
		{
			std::vector<ComponentType> types;
			for (const auto& [index, cmpType] : _typeMap)
			{
				types.push_back(cmpType);
			}
			return types;
		}

		template<typename T>
		T& getComponent(Entity entity)
		{
			ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
			auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

			return componentPool->getComponent(entity);
		}

		template<typename T>
		const std::vector<Entity> getEntitiesWithComponent()
		{
			ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
			auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

			return componentPool->getEntitiesWithComponent();
		}

		template<typename T>
		bool hasComponent(Entity entity)
		{
			ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
			auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

			return componentPool->hasComponent(entity);
		}

		bool hasComponent(Entity entity, ComponentType componentType)
		{
			return _componentPools.at(componentType)->hasComponent(entity);
		}

		template<typename T>
		T& addComponent(Entity entity, const T& component)
		{
			ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
			auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

			return componentPool->addComponent(entity, component);
		}

		template<typename T>
		void removeComponent(Entity entity)
		{
			ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));

			auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());
			componentPool->removeComponent(entity);
		}

		void removeComponent(Entity entity, std::type_index typeIndex)
		{
			ComponentType componentTypeBit = _typeMap.at(typeIndex);

			auto componentPool = _componentPools[componentTypeBit].get();
			componentPool->removeComponent(entity);
		}

		void removeAllComponents(Entity entity)
		{
			entityDestroyed(entity);
		}

		void entityDestroyed(Entity entity)
		{
			for (uint32_t i = 0; i < _currentComponentType; i++)
			{
				_componentPools[i]->entityDestroyed(entity);
			}
		}

	private:
		std::vector<std::unique_ptr<ComponentPoolBase>> _componentPools = {};
		std::unordered_map<std::type_index, ComponentType> _typeMap = {};
		ComponentType _currentComponentType = 0;
	};
}