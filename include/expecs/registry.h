#pragma once
#include "entity_manager.h"
#include "component_manager.h"
#include "system_manager.h"

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

        uint32_t getEntityCount() const
        {
            return _entityManager->getEntityCount();
        }

        Signature getEntitySignature(Entity entity) const
        {
            return _entityManager->getSignature(entity);
        }

        template<typename T>
        Signature registerComponent()
        {
            return _componentManager->registerComponentType<T>();
        }

        template<typename T>
        T& getComponent(Entity entity) const
        {
            return _componentManager->getComponent<T>(entity);
        }

        template<typename T>
        Signature getComponentSignature() const
        {
            return _componentManager->getSignature<T>();
        }

        template<typename T>
        const std::vector<Entity> getEntitiesWithComponent()
        {
            return _componentManager->getEntitiesWithComponent<T>();
        }

        template<typename T>
        bool hasComponent(Entity entity) const
        {
            return _componentManager->hasComponent<T>(entity);
        }

        bool hasComponent(Entity entity, ComponentType componentType) const
        {
            return _componentManager->hasComponent(entity, componentType);
        }

        template<typename T>
        T& addComponent(Entity entity, const T& component)
        {
            auto signature = _entityManager->getSignature(entity);
            signature |= (1 << _componentManager->getComponentType<T>());
            
            _entityManager->setSignature(entity, signature);

            auto& cmpRef = _componentManager->addComponent(entity, component);

            _systemManager->entitySignatureChanged(entity, signature);

            return cmpRef;
        }

        template<typename T>
        void removeComponent(Entity entity)
        {
            auto signature = _entityManager->getSignature(entity);
            signature &= ~(1 << _componentManager->getComponentType<T>());

            _systemManager->entitySignatureChanged(entity, signature);

            _componentManager->removeComponent<T>(entity);

            _entityManager->setSignature(entity, signature);
        }

        void removeAllComponents(Entity entity)
        {
            _systemManager->entitySignatureChanged(entity, 0);

            _componentManager->removeAllComponents(entity);

            _entityManager->setSignature(entity, 0);
        }

        template<DerivedFromSystem T, typename... Args>
        T* registerSystem(Signature signature, Args&&... args)
        {
            auto system = _systemManager->registerSystem<T>(signature, std::forward<Args>(args)...);
            system->_registry = this;
            return system;
        }
        
        template<DerivedFromSystem T>
        T* getSystem() const
        {
            return _systemManager->getSystem<T>();
        }

    private:
        std::unique_ptr<EntityManager> _entityManager = nullptr;
        std::unique_ptr<ComponentManager> _componentManager = nullptr;
        std::unique_ptr<SystemManager> _systemManager = nullptr;
    };
}