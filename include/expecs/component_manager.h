#pragma once
#include "component_pool.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace expecs
{
    using ComponentType = uint8_t;
    constexpr uint8_t MAX_COMPONENTS = 64;

    class ComponentManager
    {
    public:
        ComponentManager() = default;
        ~ComponentManager() = default;

        Signature registerComponentPool(std::type_index typeIndex, std::unique_ptr<ComponentPoolBase> pool)
        {
            _componentPools.push_back(std::move(pool));

            _typeMap[typeIndex] = _currentComponentType;
            Signature componentSignature = 0;
            componentSignature |= (Signature{1} << _currentComponentType);

            _currentComponentType++;

            return componentSignature;
        }

        template <typename T>
        Signature registerComponentType()
        {
            _componentPools.push_back(std::make_unique<ComponentPool<T>>());

            _typeMap[std::type_index(typeid(T))] = _currentComponentType;
            Signature componentSignature = 0;
            componentSignature |= (Signature{1} << _currentComponentType);

            _currentComponentType++;

            return componentSignature;
        }

        template <typename T>
        Signature getSignature() const
        {
            Signature componentSignature = 0;
            if (_typeMap.contains(std::type_index(typeid(T))))
            {
                componentSignature |= (Signature{1} << _typeMap.at(std::type_index(typeid(T))));
            }

            return componentSignature;
        }

        Signature getSignature(std::type_index typeIndex) const
        {
            Signature componentSignature = 0;
            if (_typeMap.contains(typeIndex))
            {
                componentSignature |= (Signature{1} << _typeMap.at(typeIndex));
            }
            return componentSignature;
        }

        template <typename T>
        ComponentType getComponentType() const
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

        template <typename T>
        T& getComponent(Entity entity)
        {
            ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
            auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

            return componentPool->getComponent(entity);
        }

        void* getComponentRaw(Entity entity, std::type_index typeIndex)
        {
            ComponentType componentTypeBit = _typeMap.at(typeIndex);
            return _componentPools[componentTypeBit]->getComponentRaw(entity);
        }

        template <typename T>
        std::vector<Entity> getEntitiesWithComponent() const
        {
            ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
            auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

            return componentPool->getEntitiesWithComponent();
        }

        template <typename T>
        bool hasComponent(Entity entity) const
        {
            if (!_typeMap.contains(std::type_index(typeid(T))))
                return false;
            ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
            auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

            return componentPool->hasComponent(entity);
        }

        bool hasComponent(Entity entity, ComponentType componentType) const
        {
            return _componentPools.at(componentType)->hasComponent(entity);
        }

        bool hasComponent(Entity entity, std::type_index typeIndex) const
        {
            if (!_typeMap.contains(typeIndex))
                return false;
            ComponentType componentTypeBit = _typeMap.at(typeIndex);
            return _componentPools[componentTypeBit]->hasComponent(entity);
        }

        template <typename T>
        T& addComponent(Entity entity, const T& component)
        {
            ComponentType componentTypeBit = _typeMap.at(std::type_index(typeid(T)));
            auto componentPool = dynamic_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

            return componentPool->addComponent(entity, component);
        }

        void* addComponentRaw(Entity entity, std::type_index typeIndex, const void* data)
        {
            ComponentType componentTypeBit = _typeMap.at(typeIndex);
            return _componentPools[componentTypeBit]->addComponentRaw(entity, data);
        }

        template <typename T>
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
} // namespace expecs