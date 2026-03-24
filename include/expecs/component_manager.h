#pragma once
#include "component_pool.h"

#include <cassert>
#include <limits>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace expecs
{
    using ComponentType = uint8_t;
    constexpr ComponentType INVALID_COMPONENT_TYPE = std::numeric_limits<ComponentType>::max();
    constexpr uint8_t MAX_COMPONENTS = 64;

    template <typename T>
    struct ComponentTypeID
    {
        static inline ComponentType id = INVALID_COMPONENT_TYPE;
    };

    class ComponentManager
    {
    public:
        ComponentManager() = default;
        ~ComponentManager() = default;

        Signature registerComponentPool(std::type_index typeIndex, std::unique_ptr<ComponentPoolBase> pool)
        {
            assert(_currentComponentType < MAX_COMPONENTS && "Exceeded maximum component types");
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
            assert(_currentComponentType < MAX_COMPONENTS && "Exceeded maximum component types");
            _componentPools.push_back(std::make_unique<ComponentPool<T>>());

            ComponentTypeID<T>::id = _currentComponentType;
            _typeMap[std::type_index(typeid(T))] = _currentComponentType;
            Signature componentSignature = 0;
            componentSignature |= (Signature{1} << _currentComponentType);

            _currentComponentType++;

            return componentSignature;
        }

        template <typename T>
        Signature getSignature() const
        {
            ComponentType componentType = ComponentTypeID<T>::id;
            if (componentType == INVALID_COMPONENT_TYPE)
                return 0;

            Signature componentSignature = 0;
            componentSignature |= (Signature{1} << componentType);

            return componentSignature;
        }

        Signature getSignature(std::type_index typeIndex) const
        {
            Signature componentSignature = 0;
            auto it = _typeMap.find(typeIndex);
            if (it != _typeMap.end())
            {
                componentSignature |= (Signature{1} << it->second);
            }
            return componentSignature;
        }

        template <typename T>
        ComponentType getComponentType() const
        {
            return ComponentTypeID<T>::id;
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
            ComponentType componentTypeBit = ComponentTypeID<T>::id;
            auto componentPool = static_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

            return componentPool->get(entity);
        }

        void* getComponent(Entity entity, std::type_index typeIndex)
        {
            ComponentType componentTypeBit = _typeMap.at(typeIndex);
            return _componentPools[componentTypeBit]->getComponent(entity);
        }

        template <typename T>
        bool hasComponent(Entity entity) const
        {
            ComponentType componentTypeBit = ComponentTypeID<T>::id;
            if (componentTypeBit == INVALID_COMPONENT_TYPE)
                return false;
            auto componentPool = static_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

            return componentPool->hasComponent(entity);
        }

        bool hasComponent(Entity entity, ComponentType componentType) const
        {
            return _componentPools.at(componentType)->hasComponent(entity);
        }

        bool hasComponent(Entity entity, std::type_index typeIndex) const
        {
            auto it = _typeMap.find(typeIndex);
            if (it == _typeMap.end())
                return false;
            return _componentPools[it->second]->hasComponent(entity);
        }

        template <typename T>
        T& addComponent(Entity entity, const T& component)
        {
            ComponentType componentTypeBit = ComponentTypeID<T>::id;
            auto componentPool = static_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());

            return componentPool->add(entity, component);
        }

        void* addComponent(Entity entity, std::type_index typeIndex, const void* data)
        {
            ComponentType componentTypeBit = _typeMap.at(typeIndex);
            return _componentPools[componentTypeBit]->addComponent(entity, data);
        }

        template <typename T>
        void removeComponent(Entity entity)
        {
            ComponentType componentTypeBit = ComponentTypeID<T>::id;

            auto componentPool = static_cast<ComponentPool<T>*>(_componentPools[componentTypeBit].get());
            componentPool->removeComponent(entity);
        }

        void removeComponent(Entity entity, std::type_index typeIndex)
        {
            ComponentType componentTypeBit = _typeMap.at(typeIndex);

            auto componentPool = _componentPools[componentTypeBit].get();
            componentPool->removeComponent(entity);
        }

        std::vector<Entity> getSmallestPoolEntities(Signature signature) const
        {
            ComponentPoolBase* smallest = nullptr;
            size_t smallestSize = SIZE_MAX;

            for (uint8_t bit = 0; bit < _currentComponentType; ++bit)
            {
                if (signature & (Signature{1} << bit))
                {
                    size_t poolSize = _componentPools[bit]->size();
                    if (poolSize < smallestSize)
                    {
                        smallestSize = poolSize;
                        smallest = _componentPools[bit].get();
                    }
                }
            }

            if (!smallest)
                return {};

            return smallest->getEntities();
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