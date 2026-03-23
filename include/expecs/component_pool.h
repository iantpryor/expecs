#pragma once
#include "entity_manager.h"

#include <cassert>
#include <queue>
#include <unordered_map>
#include <vector>

namespace expecs
{
    class ComponentPoolBase
    {
    public:
        ComponentPoolBase() = default;
        virtual ~ComponentPoolBase() = default;

        virtual void entityDestroyed(Entity entity) = 0;
        virtual void removeComponent(Entity entity) = 0;
        virtual bool hasComponent(Entity entity) const = 0;

        virtual void* addComponent(Entity entity, const void* data) = 0;
        virtual void* getComponent(Entity entity) = 0;

        virtual size_t size() const = 0;
        virtual std::vector<Entity> getEntities() const = 0;
    };

    template <typename T>
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

        T& get(Entity entity)
        {
            auto it = _entityToIndexMap.find(entity);
            return _componentData.at(it->second);
        }

        void* getComponent(Entity entity) override
        {
            return &get(entity);
        }

        bool hasComponent(Entity entity) const override
        {
            return _entityToIndexMap.contains(entity);
        }

        T& add(Entity entity, const T& component)
        {
            size_t newIndex = _componentData.size();
            _componentData.push_back(component);

            _entityToIndexMap[entity] = newIndex;
            _indexToEntityMap[newIndex] = entity;

            return _componentData[newIndex];
        }

        void* addComponent(Entity entity, const void* data) override
        {
            return &add(entity, *static_cast<const T*>(data));
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

        size_t size() const override
        {
            return _entityToIndexMap.size();
        }

        std::vector<Entity> getEntities() const override
        {
            std::vector<Entity> entities;
            entities.reserve(_entityToIndexMap.size());
            for (const auto& [entity, _] : _entityToIndexMap)
                entities.push_back(entity);
            return entities;
        }

    private:
        std::vector<T> _componentData = {};
        std::unordered_map<Entity, size_t> _entityToIndexMap = {};
        std::unordered_map<size_t, Entity> _indexToEntityMap = {};
    };
} // namespace expecs
