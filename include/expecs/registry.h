#pragma once
#include "component_manager.h"
#include "entity_manager.h"
#include "system_manager.h"

#include <cassert>
#include <memory>

namespace expecs
{
    class Registry
    {
    public:
        Registry()
        {
            _entityManager = std::make_unique<EntityManager>();
            _componentManager = std::make_unique<ComponentManager>();
            _systemManager = std::make_unique<SystemManager>();
        }
        ~Registry() = default;

        Entity createEntity()
        {
            return _entityManager->createEntity();
        }

        void destroyEntity(Entity entity)
        {
            _systemManager->entityDestroyed(entity);
            _componentManager->entityDestroyed(entity);
            _entityManager->destroyEntity(entity);
        }

        bool isAlive(Entity entity) const
        {
            return _entityManager->isAlive(entity);
        }

        uint32_t getEntityCount() const
        {
            return _entityManager->getEntityCount();
        }

        Signature getEntitySignature(Entity entity) const
        {
            return _entityManager->getSignature(entity);
        }

        Signature registerComponentPool(std::type_index typeIndex, std::unique_ptr<ComponentPoolBase> pool)
        {
            return _componentManager->registerComponentPool(typeIndex, std::move(pool));
        }

        template <typename T>
        Signature registerComponent()
        {
            return _componentManager->registerComponentType<T>();
        }

        template <typename T>
        T& getComponent(Entity entity) const
        {
            assert(_entityManager->isAlive(entity) && "Entity is not alive");
            return _componentManager->getComponent<T>(entity);
        }

        void* getComponent(Entity entity, std::type_index typeIndex)
        {
            assert(_entityManager->isAlive(entity) && "Entity is not alive");
            return _componentManager->getComponent(entity, typeIndex);
        }

        template <typename T>
        Signature getComponentSignature() const
        {
            return _componentManager->getSignature<T>();
        }

        Signature getComponentSignature(std::type_index typeIndex) const
        {
            return _componentManager->getSignature(typeIndex);
        }

        template <typename... Ts>
        std::vector<Entity> getEntitiesWithComponents() const
        {
            Signature query = (_componentManager->getSignature<Ts>() | ...);
            return getEntitiesWithComponents(query);
        }

        std::vector<Entity> getEntitiesWithComponents(Signature signature) const
        {
            auto candidates = _componentManager->getSmallestPoolEntities(signature);
            std::vector<Entity> result;
            for (Entity entity : candidates)
            {
                if ((_entityManager->getSignature(entity) & signature) == signature)
                    result.push_back(entity);
            }
            return result;
        }

        template <typename... Ts, typename Func>
        void each(Func&& func) const
        {
            Signature query = (_componentManager->getSignature<Ts>() | ...);
            each(query, std::forward<Func>(func));
        }

        template <typename Func>
        void each(Signature query, Func&& func) const
        {
            const auto& candidates = _componentManager->getSmallestPoolEntities(query);
            for (Entity entity : candidates)
            {
                if ((_entityManager->getSignature(entity) & query) == query)
                    func(entity);
            }
        }

        template <typename T>
        bool hasComponent(Entity entity) const
        {
            return _componentManager->hasComponent<T>(entity);
        }

        bool hasComponent(Entity entity, ComponentType componentType) const
        {
            return _componentManager->hasComponent(entity, componentType);
        }

        bool hasComponent(Entity entity, std::type_index typeIndex) const
        {
            return _componentManager->hasComponent(entity, typeIndex);
        }

        template <typename T>
        T& addComponent(Entity entity, const T& component)
        {
            auto signature = _entityManager->getSignature(entity);
            signature.set(_componentManager->getComponentType<T>());

            _entityManager->setSignature(entity, signature);

            auto& cmpRef = _componentManager->addComponent(entity, component);

            _systemManager->entitySignatureChanged(entity, signature);

            return cmpRef;
        }

        void* addComponent(Entity entity, std::type_index typeIndex, const void* data)
        {
            auto signature = _entityManager->getSignature(entity);
            signature |= _componentManager->getSignature(typeIndex);

            _entityManager->setSignature(entity, signature);

            void* ptr = _componentManager->addComponent(entity, typeIndex, data);

            _systemManager->entitySignatureChanged(entity, signature);

            return ptr;
        }

        template <typename T>
        void removeComponent(Entity entity)
        {
            auto signature = _entityManager->getSignature(entity);
            signature.reset(_componentManager->getComponentType<T>());

            _systemManager->entitySignatureChanged(entity, signature);

            _componentManager->removeComponent<T>(entity);

            _entityManager->setSignature(entity, signature);
        }

        void removeComponent(Entity entity, std::type_index typeIndex)
        {
            auto signature = _entityManager->getSignature(entity);
            signature.reset(_componentManager->getComponentType(typeIndex));

            _systemManager->entitySignatureChanged(entity, signature);

            _componentManager->removeComponent(entity, typeIndex);

            _entityManager->setSignature(entity, signature);
        }

        void removeAllComponents(Entity entity)
        {
            _systemManager->entitySignatureChanged(entity, Signature{});

            _componentManager->removeAllComponents(entity);

            _entityManager->setSignature(entity, Signature{});
        }

        template <DerivedFromSystem T, typename... Args>
        T* registerSystem(Signature signature, Args&&... args)
        {
            auto system = _systemManager->registerSystem<T>(signature, std::forward<Args>(args)...);
            system->_registry = this;
            return system;
        }

        System* registerSystem(std::type_index typeIndex, Signature signature, std::unique_ptr<System> system)
        {
            auto* ptr = _systemManager->registerSystem(typeIndex, signature, std::move(system));
            ptr->_registry = this;
            return ptr;
        }

        template <DerivedFromSystem T>
        T* getSystem() const
        {
            return _systemManager->getSystem<T>();
        }

        System* getSystem(std::type_index typeIndex) const
        {
            return _systemManager->getSystem(typeIndex);
        }

    private:
        std::unique_ptr<EntityManager> _entityManager = nullptr;
        std::unique_ptr<ComponentManager> _componentManager = nullptr;
        std::unique_ptr<SystemManager> _systemManager = nullptr;
    };
} // namespace expecs