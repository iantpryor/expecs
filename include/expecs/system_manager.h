#pragma once
#include "entity_manager.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace expecs
{
    class Registry;
    class System
    {
    public:
        System()
        {
            _entityToIndexMap.resize(MAX_ENTITIES, INVALID_INDEX);
        }
        virtual ~System() = default;

        virtual void entityAdded(Entity /*entity*/) {}
        virtual void entityRemoved(Entity /*entity*/) {}

        const std::vector<Entity>& getEntities() const { return _entities; }
        bool contains(Entity entity) const { return entity < MAX_ENTITIES && _entityToIndexMap[entity] != INVALID_INDEX; }
        Registry* getRegistry() const { return _registry; }

    private:
        static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
        friend class SystemManager;
        friend class Registry;
        std::vector<Entity> _entities;
        std::vector<size_t> _entityToIndexMap;
        Registry* _registry = nullptr;

        void addEntity(Entity entity)
        {
            _entityToIndexMap[entity] = _entities.size();
            _entities.push_back(entity);
        }

        void removeEntity(Entity entity)
        {
            size_t indexToRemove = _entityToIndexMap[entity];
            size_t lastIndex = _entities.size() - 1;

            if (indexToRemove != lastIndex)
            {
                // Move last element to the position of removed element
                Entity lastEntity = _entities[lastIndex];
                _entities[indexToRemove] = lastEntity;
                _entityToIndexMap[lastEntity] = indexToRemove;
            }

            // Remove the last element
            _entities.pop_back();
            _entityToIndexMap[entity] = INVALID_INDEX;
        }
    };

    template <typename T>
    concept DerivedFromSystem = std::is_base_of<System, T>::value;

    class SystemManager
    {
    public:
        SystemManager() = default;
        ~SystemManager() = default;

        template <DerivedFromSystem T, typename... Args>
        T* registerSystem(Signature signature, Args&&... args)
        {
            const char* typeName = typeid(T).name();
            _systemsMap[typeName] = std::make_unique<T>(std::forward<Args>(args)...);
            _systemSignatures[typeName] = signature;
            return dynamic_cast<T*>(_systemsMap[typeName].get());
        }

        System* registerSystem(std::type_index typeIndex, Signature signature, std::unique_ptr<System> system)
        {
            const char* typeName = typeIndex.name();
            _systemsMap[typeName] = std::move(system);
            _systemSignatures[typeName] = signature;
            return _systemsMap[typeName].get();
        }

        template <DerivedFromSystem T>
        T* getSystem() const
        {
            const char* typeName = typeid(T).name();
            return dynamic_cast<T*>(_systemsMap.at(typeName).get());
        }

        System* getSystem(std::type_index typeIndex) const
        {
            const char* typeName = typeIndex.name();
            return _systemsMap.at(typeName).get();
        }

        void entityDestroyed(Entity entity)
        {
            for (auto const& [typeName, system] : _systemsMap)
            {
                if (system->contains(entity))
                {
                    system->entityRemoved(entity);
                    system->removeEntity(entity);
                }
            }
        }

        void entitySignatureChanged(Entity entity, Signature entitySignature)
        {
            for (auto const& [typeName, system] : _systemsMap)
            {
                auto const& systemSignature = _systemSignatures[typeName];

                if ((entitySignature & systemSignature) == systemSignature)
                {
                    if (!system->contains(entity))
                    {
                        system->addEntity(entity);
                        system->entityAdded(entity);
                    }
                }
                else
                {
                    if (system->contains(entity))
                    {
                        system->entityRemoved(entity);
                        system->removeEntity(entity);
                    }
                }
            }
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<System>> _systemsMap;
        std::unordered_map<std::string, Signature> _systemSignatures;
    };
} // namespace expecs