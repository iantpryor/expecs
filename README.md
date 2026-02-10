# ExplictECS (expecs)

ExplictECS is a lightweight C++ Entity Component System (ECS) designed mostly for clarity and educational value. It utilizes a push-based architecture to maintain system membership, ensuring high-performance iteration during logic updates.

---

## Architecture Overview

The framework is built around a central Registry that orchestrates three specialized managers:

* **EntityManager**: Handles entity creation, destruction, and signature tracking.
* **ComponentManager**: Manages component data storage in contiguous memory pools.
* **SystemManager**: Coordinates which entities belong to which systems based on bitmask signatures.

---

## Core Features

* **Push-Model Logic**: Systems maintain their own lists of entities. Membership is updated only when components are added or removed, removing the need for per-frame filtering.
* **Memory Efficiency**: Component data is stored in pools to maximize CPU cache hits.
* **Entity Recycling**: Dead entity IDs are placed in a queue for reuse, keeping the entity index space compact.

---

## Usage Guide

### 1. Define Components

Components should be simple data structures (PODs).

```cpp
struct Position {
    float x, y, z;
    Position(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Velocity {
    float dx, dy, dz;
    Velocity(float dx = 0, float dy = 0, float dz = 0) : dx(dx), dy(dy), dz(dz) {}
};

```

### 2. Implement a System

Systems inherit from the base System class and have access to the a Registry ptr as well as the Entities within that system.

```cpp
class MovementSystem : public expecs::System {
public:
    void update() {
        for (auto const& entity : getEntities()) {
            auto& pos = getRegistry()->getComponent<Position>(entity);
            auto& vel = getRegistry()->getComponent<Velocity>(entity);

            pos.x += vel.dx;
            pos.y += vel.dy;
        }
    }
};

```

### 3. Initialize and Run

The Registry manages the lifecycle of entities and the orchestration of systems.

```cpp
auto registry = std::make_unique<expecs::Registry>();

// Register components
registry->registerComponent<Position>();
registry->registerComponent<Velocity>();

// Define signature (Position + Velocity)
expecs::Signature moveSignature = 0;
moveSignature |= registry->getComponentSignature<Position>();
moveSignature |= registry->getComponentSignature<Velocity>();

// Register the system
auto movementSystem = registry->registerSystem<MovementSystem>(moveSignature);

// Create entity and add data
auto entity = registry->createEntity();
registry->addComponent(entity, Position(0, 0, 0));
registry->addComponent(entity, Velocity(1, 0, 0)); 

// Execution
movementSystem->update();

```


## Considerations

* **Signature Matching**: System membership is determined by a bitwise AND comparison between the entity's signature and the system's required signature.  The signature for a system needs to be explictly set during registration.
* **Reference Stability**: `getComponent<T>` returns a reference to the component data. Users should not store these references across multiple frames as component relocation may occur in the pool if components are removed and rearranged to keep tight packing.