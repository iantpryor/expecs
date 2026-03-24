#include "expecs/system_manager.h"

#include "mock_objects.h"

#include <gtest/gtest.h>

namespace expecs
{
    class expecs_SystemManagerTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            _systemManager = std::make_unique<SystemManager>();

            // Movement system requires Position and Velocity
            Signature movementSignature = 0;
            movementSignature |= (Signature{1} << 0); // Position component bit
            movementSignature |= (Signature{1} << 1); // Velocity component bit

            _movementSystem = _systemManager->registerSystem<MovementSystem>(movementSignature);
        }

        std::unique_ptr<SystemManager> _systemManager;
        MovementSystem* _movementSystem;
    };

    TEST_F(expecs_SystemManagerTest, registerSystem_ReturnsValidSystem)
    {
        EXPECT_NE(_movementSystem, nullptr);
        EXPECT_EQ(_movementSystem->getEntities().size(), 0u);
    }

    TEST_F(expecs_SystemManagerTest, registerSystem_TypeErased_ReturnsValidSystem)
    {
        Signature systemSignature = (Signature{1} << 2);
        auto system = std::make_unique<RenderSystem>();
        auto* raw = system.get();

        auto* registered = _systemManager->registerSystem(std::type_index(typeid(RenderSystem)), systemSignature, std::move(system));

        EXPECT_EQ(registered, raw);
    }

    TEST_F(expecs_SystemManagerTest, getSystem_ReturnsCorrectSystem)
    {
        MovementSystem* retrieved = _systemManager->getSystem<MovementSystem>();
        EXPECT_EQ(retrieved, _movementSystem);
    }

    TEST_F(expecs_SystemManagerTest, getSystem_TypeErased_ReturnsCorrectSystem)
    {
        Signature systemSignature = (Signature{1} << 2);
        auto system = std::make_unique<RenderSystem>();

        auto* registered = _systemManager->registerSystem(std::type_index(typeid(RenderSystem)), systemSignature, std::move(system));

        auto* retrieved = _systemManager->getSystem(std::type_index(typeid(RenderSystem)));
        EXPECT_EQ(retrieved, registered);
    }

    TEST_F(expecs_SystemManagerTest, entitySignatureChanged_AddsEntityToSystem)
    {
        Entity entity = 42;
        Signature entitySignature = 0;
        entitySignature |= (Signature{1} << 0); // Position
        entitySignature |= (Signature{1} << 1); // Velocity

        _systemManager->entitySignatureChanged(entity, entitySignature);

        EXPECT_EQ(_movementSystem->getEntities().size(), 1u);
        EXPECT_TRUE(_movementSystem->contains(entity));
        EXPECT_TRUE(_movementSystem->entitiesAdded.contains(entity));
    }

    TEST_F(expecs_SystemManagerTest, registerSystem_TypeErased_AddsEntityToSystem)
    {
        Signature systemSignature = (Signature{1} << 2);
        auto system = std::make_unique<RenderSystem>();

        auto* registered = _systemManager->registerSystem(std::type_index(typeid(RenderSystem)), systemSignature, std::move(system));

        Entity entity = 10;
        _systemManager->entitySignatureChanged(entity, systemSignature);

        EXPECT_EQ(registered->getEntities().size(), 1u);
        EXPECT_TRUE(registered->contains(entity));
        EXPECT_TRUE(static_cast<RenderSystem*>(registered)->entitiesAdded.contains(entity));
    }

    TEST_F(expecs_SystemManagerTest, entitySignatureChanged_RemovesEntityFromSystem)
    {
        Entity entity = 42;
        Signature fullSignature = 0b11;    // Has both Position and Velocity
        Signature partialSignature = 0b01; // Only Position

        // Add entity to system
        _systemManager->entitySignatureChanged(entity, fullSignature);
        EXPECT_TRUE(_movementSystem->contains(entity));

        // Remove velocity component (partial signature)
        _systemManager->entitySignatureChanged(entity, partialSignature);
        EXPECT_FALSE(_movementSystem->contains(entity));
        EXPECT_TRUE(_movementSystem->entitiesRemoved.contains(entity));
    }

    TEST_F(expecs_SystemManagerTest, entityDestroyed_RemovesFromAllSystems)
    {
        Entity entity = 42;
        Signature entitySignature = 0b11; // Has both components

        _systemManager->entitySignatureChanged(entity, entitySignature);
        EXPECT_TRUE(_movementSystem->contains(entity));

        _systemManager->entityDestroyed(entity);
        EXPECT_FALSE(_movementSystem->contains(entity));
        EXPECT_TRUE(_movementSystem->entitiesRemoved.contains(entity));
    }
} // namespace expecs