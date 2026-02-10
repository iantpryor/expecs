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
            movementSignature |= (1 << 0); // Position component bit
            movementSignature |= (1 << 1); // Velocity component bit

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

    TEST_F(expecs_SystemManagerTest, getSystem_ReturnsCorrectSystem)
    {
        MovementSystem* retrieved = _systemManager->getSystem<MovementSystem>();
        EXPECT_EQ(retrieved, _movementSystem);
    }

    TEST_F(expecs_SystemManagerTest, entitySignatureChanged_AddsEntityToSystem)
    {
        Entity entity = 42;
        Signature entitySignature = 0;
        entitySignature |= (1 << 0); // Position
        entitySignature |= (1 << 1); // Velocity

        _systemManager->entitySignatureChanged(entity, entitySignature);

        EXPECT_EQ(_movementSystem->getEntities().size(), 1u);
        EXPECT_TRUE(_movementSystem->getEntities().contains(entity));
        EXPECT_TRUE(_movementSystem->entitiesAdded.contains(entity));
    }

    TEST_F(expecs_SystemManagerTest, entitySignatureChanged_RemovesEntityFromSystem)
    {
        Entity entity = 42;
        Signature fullSignature = 0b11; // Has both Position and Velocity
        Signature partialSignature = 0b01; // Only Position

        // Add entity to system
        _systemManager->entitySignatureChanged(entity, fullSignature);
        EXPECT_TRUE(_movementSystem->getEntities().contains(entity));

        // Remove velocity component (partial signature)
        _systemManager->entitySignatureChanged(entity, partialSignature);
        EXPECT_FALSE(_movementSystem->getEntities().contains(entity));
        EXPECT_TRUE(_movementSystem->entitiesRemoved.contains(entity));
    }

    TEST_F(expecs_SystemManagerTest, entityDestroyed_RemovesFromAllSystems)
    {
        Entity entity = 42;
        Signature entitySignature = 0b11; // Has both components

        _systemManager->entitySignatureChanged(entity, entitySignature);
        EXPECT_TRUE(_movementSystem->getEntities().contains(entity));

        _systemManager->entityDestroyed(entity);
        EXPECT_FALSE(_movementSystem->getEntities().contains(entity));
        EXPECT_TRUE(_movementSystem->entitiesRemoved.contains(entity));
    }
}