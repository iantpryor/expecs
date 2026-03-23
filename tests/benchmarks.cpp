#include "expecs/registry.h"

#include "mock_objects.h"

#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>

namespace expecs
{
    constexpr int NUM_ENTITIES = 100000;
    constexpr int NUM_ITERATIONS = 10;

    class expecs_PerformanceTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            _registry = std::make_unique<Registry>();
            _registry->registerComponent<Position>();
            _registry->registerComponent<Velocity>();
        }

        std::unique_ptr<Registry> _registry;
    };

    TEST_F(expecs_PerformanceTest, createManyEntities)
    {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        EXPECT_LT(duration.count(), 100000) << "Creating " << NUM_ENTITIES << " entities took " << duration.count() << " microseconds";
        EXPECT_EQ(_registry->getEntityCount(), NUM_ENTITIES);
    }

    TEST_F(expecs_PerformanceTest, addComponent_Templated)
    {
        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            _registry = std::make_unique<Registry>();
            _registry->registerComponent<Position>();
            _registry->registerComponent<Velocity>();

            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < NUM_ENTITIES; ++i)
            {
                Entity entity = _registry->createEntity();
                _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
            }
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[addComponent] Templated (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }

    TEST_F(expecs_PerformanceTest, addComponent_TypeErased)
    {
        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            _registry = std::make_unique<Registry>();
            _registry->registerComponent<Position>();
            _registry->registerComponent<Velocity>();

            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < NUM_ENTITIES; ++i)
            {
                Entity entity = _registry->createEntity();
                Position pos((float)i, (float)i, (float)i);
                _registry->addComponent(entity, std::type_index(typeid(Position)), &pos);
            }
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[addComponent] TypeErased (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }

    TEST_F(expecs_PerformanceTest, getComponent_Templated)
    {
        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
        }

        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (Entity e = 0; e < NUM_ENTITIES; ++e)
            {
                volatile auto& pos = _registry->getComponent<Position>(e);
                (void)pos;
            }
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[getComponent] Templated (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }

    TEST_F(expecs_PerformanceTest, getComponent_TypeErased)
    {
        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
        }

        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (Entity e = 0; e < NUM_ENTITIES; ++e)
            {
                volatile auto* pos = static_cast<Position*>(_registry->getComponent(e, std::type_index(typeid(Position))));
                (void)pos;
            }
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[getComponent] TypeErased (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }

    TEST_F(expecs_PerformanceTest, hasComponent_Templated)
    {
        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
        }

        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (Entity e = 0; e < NUM_ENTITIES; ++e)
            {
                volatile bool has = _registry->hasComponent<Position>(e);
                (void)has;
            }
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[hasComponent] Templated (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }

    TEST_F(expecs_PerformanceTest, hasComponent_TypeErased)
    {
        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
        }

        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (Entity e = 0; e < NUM_ENTITIES; ++e)
            {
                volatile bool has = _registry->hasComponent(e, std::type_index(typeid(Position)));
                (void)has;
            }
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[hasComponent] TypeErased (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }

    TEST_F(expecs_PerformanceTest, getEntitiesWithComponents_Templated)
    {
        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
            if (i % 2 == 0)
                _registry->addComponent(entity, Velocity((float)i, 0, 0));
        }

        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            auto start = std::chrono::high_resolution_clock::now();
            auto result = _registry->getEntitiesWithComponents<Position, Velocity>();
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

            if (iter == 0)
                EXPECT_EQ(result.size(), 50000u);
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[getEntitiesWithComponents] Templated (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }

    TEST_F(expecs_PerformanceTest, getEntitiesWithComponents_SignatureBased)
    {
        for (int i = 0; i < NUM_ENTITIES; ++i)
        {
            Entity entity = _registry->createEntity();
            _registry->addComponent(entity, Position((float)i, (float)i, (float)i));
            if (i % 2 == 0)
                _registry->addComponent(entity, Velocity((float)i, 0, 0));
        }

        Signature query = _registry->getComponentSignature(std::type_index(typeid(Position))) | _registry->getComponentSignature(std::type_index(typeid(Velocity)));

        std::vector<long long> durations;
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter)
        {
            auto start = std::chrono::high_resolution_clock::now();
            auto result = _registry->getEntitiesWithComponents(query);
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

            if (iter == 0)
                EXPECT_EQ(result.size(), 50000u);
        }

        std::sort(durations.begin(), durations.end());
        std::cout << "[getEntitiesWithComponents] SignatureBased (x" << NUM_ITERATIONS << "): "
                  << "min=" << durations.front() << "us, "
                  << "median=" << durations[NUM_ITERATIONS / 2] << "us, "
                  << "max=" << durations.back() << "us" << std::endl;
    }
} // namespace expecs