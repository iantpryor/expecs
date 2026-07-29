#include "expecs/entity_manager.h"

#include "mock_objects.h"

#include <gtest/gtest.h>

namespace expecs
{
    class expecs_EntityManagerTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            _entityManager = std::make_unique<EntityManager>();
        }

        std::unique_ptr<EntityManager> _entityManager;
    };

    TEST_F(expecs_EntityManagerTest, createEntity)
    {
        Entity entity = _entityManager->createEntity();
        EXPECT_EQ(_entityManager->getEntityCount(), 1u);
    }

    TEST_F(expecs_EntityManagerTest, createMultipleEntities)
    {
        std::unordered_set<Entity> entities;

        for (int i = 0; i < 100; ++i)
        {
            Entity entity = _entityManager->createEntity();
            EXPECT_TRUE(entities.insert(entity).second) << "Entity " << entity.id << " was not unique";
        }

        EXPECT_EQ(_entityManager->getEntityCount(), 100u);
    }

    TEST_F(expecs_EntityManagerTest, destroyEntity_Count)
    {
        Entity entity1 = _entityManager->createEntity();
        Entity entity2 = _entityManager->createEntity();

        EXPECT_EQ(_entityManager->getEntityCount(), 2u);

        _entityManager->destroyEntity(entity1);
        EXPECT_EQ(_entityManager->getEntityCount(), 1u);

        _entityManager->destroyEntity(entity2);
        EXPECT_EQ(_entityManager->getEntityCount(), 0u);
    }

    TEST_F(expecs_EntityManagerTest, destroyEntity_Signature)
    {
        Entity entity = _entityManager->createEntity();
        _entityManager->setSignature(entity, 0xFF);

        _entityManager->destroyEntity(entity);

        Entity newEntity = _entityManager->createEntity();
        EXPECT_EQ(newEntity.index(), entity.index());
        EXPECT_EQ(newEntity.generation(), entity.generation() + 1);
        EXPECT_NE(newEntity, entity);
        EXPECT_EQ(_entityManager->getSignature(newEntity), 0u);
    }

#ifndef NDEBUG
    TEST_F(expecs_EntityManagerTest, entity_Asserts_WhenIndexOrGenerationOutOfRange)
    {
        EXPECT_DEATH(Entity(ENTITY_INDEX_MASK + 1), "out of range");
        EXPECT_DEATH(Entity(0, MAX_GENERATION + 1), "out of range");
    }
#endif

    TEST_F(expecs_EntityManagerTest, isAlive_TracksLifetime)
    {
        Entity entity = _entityManager->createEntity();
        EXPECT_TRUE(_entityManager->isAlive(entity));

        _entityManager->destroyEntity(entity);
        EXPECT_FALSE(_entityManager->isAlive(entity));
    }

    TEST_F(expecs_EntityManagerTest, isAlive_ReturnsFalse_ForStaleAndFabricatedHandles)
    {
        Entity entity = _entityManager->createEntity();
        _entityManager->destroyEntity(entity);

        Entity recycled = _entityManager->createEntity();
        EXPECT_EQ(recycled.index(), entity.index());

        EXPECT_TRUE(_entityManager->isAlive(recycled));
        EXPECT_FALSE(_entityManager->isAlive(entity));

        _entityManager->createEntity();
        Entity last = _entityManager->createEntity();
        EXPECT_FALSE(_entityManager->isAlive(Entity(last.index() + 1)));

        EXPECT_FALSE(_entityManager->isAlive(INVALID_ENTITY));
    }

    TEST_F(expecs_EntityManagerTest, createEntity_Throws_WhenIndexSpaceExhausted)
    {
        for (uint32_t i = 0; i < MAX_ENTITIES; ++i)
        {
            _entityManager->createEntity();
        }

        EXPECT_THROW(_entityManager->createEntity(), std::length_error);
    }

    TEST_F(expecs_EntityManagerTest, generationExhaustion_RetiresSlot)
    {
        Entity entity = _entityManager->createEntity();
        uint32_t index = entity.index();

        for (uint32_t generation = 0; generation < MAX_GENERATION; ++generation)
        {
            EXPECT_EQ(entity.index(), index);
            EXPECT_EQ(entity.generation(), generation);

            _entityManager->destroyEntity(entity);
            entity = _entityManager->createEntity();
        }

        EXPECT_EQ(entity.index(), index);
        EXPECT_EQ(entity.generation(), MAX_GENERATION);

        _entityManager->destroyEntity(entity);
        EXPECT_FALSE(_entityManager->isAlive(entity));

        Entity fresh = _entityManager->createEntity();
        EXPECT_NE(fresh.index(), index);
        EXPECT_EQ(fresh.generation(), 0u);
        EXPECT_TRUE(_entityManager->isAlive(fresh));
        EXPECT_FALSE(_entityManager->isAlive(entity));
    }

    TEST_F(expecs_EntityManagerTest, setgetSignature)
    {
        Entity entity = _entityManager->createEntity();

        _entityManager->setSignature(entity, 0b10101010);
        EXPECT_EQ(_entityManager->getSignature(entity), 0b10101010u);

        _entityManager->setSignature(entity, 0b01010101);
        EXPECT_EQ(_entityManager->getSignature(entity), 0b01010101u);
    }
} // namespace expecs