#pragma once
#include "entity_manager.h"

#include <algorithm>
#include <limits>
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
        virtual const std::vector<Entity>& getEntities() const = 0;
    };

    template <typename T>
    class ComponentPool : public ComponentPoolBase
    {
    public:
        ComponentPool() = default;
        ~ComponentPool() = default;

        T& get(Entity entity)
        {
            return _componentData[_entityToIndexMap[entity.index()]];
        }

        void* getComponent(Entity entity) override
        {
            return &get(entity);
        }

        bool hasComponent(Entity entity) const override
        {
            if (entity.index() >= _entityToIndexMap.size())
                return false;

            size_t index = _entityToIndexMap[entity.index()];
            return index != INVALID_INDEX && _entities[index] == entity;
        }

        T& add(Entity entity, const T& component)
        {
            size_t newIndex = _componentData.size();
            _componentData.push_back(component);

            if (entity.index() >= _entityToIndexMap.size())
            {
                size_t newSize = std::max(_entityToIndexMap.size() * 2, static_cast<size_t>(entity.index()) + 1);
                _entityToIndexMap.resize(newSize, INVALID_INDEX);
            }

            _entityToIndexMap[entity.index()] = newIndex;
            _entities.push_back(entity);

            return _componentData[newIndex];
        }

        void* addComponent(Entity entity, const void* data) override
        {
            return &add(entity, *static_cast<const T*>(data));
        }

        void removeComponent(Entity entity) override
        {
            size_t indexToRemove = _entityToIndexMap[entity.index()];
            size_t lastIndex = _componentData.size() - 1;

            if (indexToRemove != lastIndex)
            {
                // Move last element to the position of removed element
                Entity lastEntity = _entities[lastIndex];

                _componentData[indexToRemove] = std::move(_componentData[lastIndex]);
                _entities[indexToRemove] = lastEntity;
                _entityToIndexMap[lastEntity.index()] = indexToRemove;
            }

            // Remove the last element
            _componentData.pop_back();
            _entityToIndexMap[entity.index()] = INVALID_INDEX;
            _entities.pop_back();
        }

        void entityDestroyed(Entity entity) override
        {
            if (hasComponent(entity))
            {
                removeComponent(entity);
            }
        }

        size_t size() const override
        {
            return _entities.size();
        }

        const std::vector<Entity>& getEntities() const override
        {
            return _entities;
        }

    private:
        static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
        std::vector<T> _componentData = {};
        std::vector<size_t> _entityToIndexMap = {};
        std::vector<Entity> _entities = {};
    };
} // namespace expecs
