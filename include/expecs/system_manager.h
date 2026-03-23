#pragma once
#include "entity_manager.h"

#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

namespace expecs
{
    class Registry;
    class System
    {
    public:
        System() = default;
        virtual ~System() = default;

        virtual void entityAdded(Entity /*entity*/) {}
        virtual void entityRemoved(Entity /*entity*/) {}

        const std::unordered_set<Entity>& getEntities() const { return _entities; }
        Registry* getRegistry() const { return _registry; }

    private:
        friend class SystemManager;
        friend class Registry;
        std::unordered_set<Entity> _entities;
        Registry* _registry = nullptr;
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
                if (system->_entities.contains(entity))
                {
                    system->entityRemoved(entity);
                    system->_entities.erase(entity);
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
                    if (!system->_entities.contains(entity))
                    {
                        system->_entities.insert(entity);
                        system->entityAdded(entity);
                    }
                }
                else
                {
                    if (system->_entities.contains(entity))
                    {
                        system->entityRemoved(entity);
                        system->_entities.erase(entity);
                    }
                }
            }
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<System>> _systemsMap;
        std::unordered_map<std::string, Signature> _systemSignatures;
    };
} // namespace expecs