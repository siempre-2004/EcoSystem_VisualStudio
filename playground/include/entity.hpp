// entity.hpp

#pragma once

#include "common.hpp"
#include "world.hpp"
#include <memory>
#include "pathfinding.h"

namespace sim
{
    constexpr int SHEEP_MAX_HP = 100;
    constexpr int SHEEP_HEAL_AMOUNT = 20;
    constexpr float SHEEP_HUNGER_HP_LOSS = 119.0f;
    constexpr int WOLF_DAMAGE = 30;
    constexpr int WOLF_MAX_HP = 100;
    constexpr float WOLF_TIRED_HP_LOSS = 119.0f;
    constexpr int REPRODUCE_HP_COST = 30;
    constexpr int REPRODUCE_HP_THRESHOLD = 60;
    constexpr float REPRODUCTION_PAUSE_TIME = 1.5f;

    struct World;
    struct Ground {
        Ground() = default;

        bool is_walkable() const;
        void set_walkable(bool state);
        void set_tile_coord(const Point& coord);

        Point m_tile_coord;
        bool  m_walkable{};
    };

    struct Grass {
        Grass() = default;
        enum class GrassState { NONE, SEED, GERMINATION, GROWN, WILT, EATEN };
        void eatenBySheep();
        void update(float dt);
        bool is_alive() const;
        float get_age() const;
        void set_age(float age);
        void set_tile_coord(const Point& coord);

        bool m_hasFertilizer = false;
        float m_updateTimer = 0.0f;
        float m_regrowTimer = 0.0f;
        Point m_tile_coord;
        float m_age{};
        GrassState m_state{ GrassState::SEED };
    };

    struct Sheep : public std::enable_shared_from_this<Sheep> {
        int HP = SHEEP_MAX_HP;
        static constexpr float WALKING_SPEED = 70.0f;
        static constexpr float RUNNING_SPEED = 150.0f;
        constexpr static float REPRODUCTION_COOLDOWN_TIME = 2.0f;
        float m_reproductionCooldown = 2.0f;
        std::weak_ptr<Sheep> reproductionPartner;
        std::vector<Point> m_path;
        float m_updateTimer = 0.0f;
        float m_reproduceTimer = 0.0f;
        bool m_isFull = false;           
        float m_satietyTimer = 0.0f;    
        const float FULL_DURATION = 5.0f;

        World* m_world;
        Sheep(World& world) : m_world(&world), m_reproductionCooldown(REPRODUCTION_COOLDOWN_TIME), m_updateTimer(0.0f) {}
        enum class SheepState { WANDERING, SEEKING, EATING, ESCAPING, DEAD, REPRODUCE };

        void set_position(const Vector2& position);
        void set_direction(const Vector2& direction);
        void set_radius(float radius);
        void set_sprite_flip_x(bool state);
        void set_sprite_origin(const Vector2& origin);
        void set_sprite_source(const Rectangle& source);

        void update(float dt);
        void sense();
        void decide(float dt);
        void act(float dt);
        void render(const Texture& texture) const;
        void getEaten();
        SheepState getState() const { return m_state; }
        void recalculatePath();

        Vector2   m_position{};
        Vector2   m_direction{};
        float     m_radius{};
        bool      m_flip_x{};
        Vector2   m_origin{};
        Rectangle m_source{};
        SheepState m_state{ SheepState::WANDERING };
        bool manureExists{ false };

        float m_hunger{ 0.0f };
        float m_eatingTimer{ 0.0f };
        bool  foundGrass{ false };
        bool  wolfNearby{ false };
        Vector2 nearestWolfPosition{};
    };

    struct Wolf {
        int HP = WOLF_MAX_HP;
        static constexpr float WALKING_SPEED = 50.0f;
        static constexpr float RUNNING_SPEED = 100.0f;
        constexpr static float EATING_PAUSE_TIME = 1.0f;
        Vector2 m_randomDirection;
        float m_pauseTimer = 0.0f;
        float m_randomTimer = 0.0f;
        float m_updateTimer = 0.0f;
        Vector2 m_targetPos = { 0.0f, 0.0f };
        std::vector<Point> m_path;
        Wolf(World& world) : m_world(&world), m_randomDirection{ 0, 0 }, m_randomTimer(0), m_hunger(0), HP(WOLF_MAX_HP), m_state(WolfState::SEEKING), m_updateTimer(0.0f) {}

        World* m_world;
        enum class WolfState { SEEKING, CATCHING, EATING, SLEEPING, DEAD, ATTACKING, ESCAPING };

        void set_position(const Vector2& position);
        void set_direction(const Vector2& direction);
        void set_radius(float radius);
        void set_sprite_flip_x(bool state);
        void set_sprite_origin(const Vector2& origin);
        void set_sprite_source(const Rectangle& source);
        bool shouldWakeUp(float dt);

        void update(float dt);
        void sense();
        void decide(float dt);
        void act(float dt);
        void render(const Texture& texture) const;
        void recalculatePath();

        Vector2   m_position{};
        Vector2   m_direction{};
        float     m_radius{};
        bool      m_flip_x{};
        Vector2   m_origin{};
        Rectangle m_source{};
        WolfState m_state{ WolfState::SEEKING };

        //variables
        float m_hunger{ 0.0f };
        bool foundSheep{ false };
        bool sheepCaught{ false };
        Sheep* targetSheep{ nullptr };
    };

    struct Manure {
        Manure() = default;
        World* m_world;
        Manure(World* world) : m_world(world) {}
        void set_position(const Vector2& position);
        void set_sprite_source(const Rectangle& source);
        void set_duration(float duration);
        void set_quality(float quality);
        void update(float dt);
        void render(const Texture& texture) const;
        void spreadGrass();

        Vector2   m_position{};
        Rectangle m_source{};
        float     m_duration{};
        float     m_quality{};
        bool      m_isActive{ true };
        float     m_alpha{ 1.0f };
        bool m_hasSpread{ false };
    };

    struct Herder {
        Vector2 m_position;
        World* m_world;
        float m_speed = 170.0f;
        float m_hitTimer = 0.0f;
        std::vector<Point> m_path;
        Texture* m_texture = nullptr;
        bool m_flip_x; 
        Vector2 m_origin;     
        Rectangle m_source;
        explicit Herder(World& world, Texture* herderTexture): m_position{ 0, 0 }, m_world(&world), m_texture(herderTexture) {
            m_source = { 0, 0, 50, 30 };
            m_origin = { 0, 0 };
            m_flip_x = false;
        }

        void update(float dt);
        void render();
        void set_position(const Vector2& position);
        Vector2 get_position() const;
        void recalculatePath();
    };

    // Transfer SheepState to string
    inline const char* SheepStateToString(Sheep::SheepState state)
    {
        switch (state) {
        case Sheep::SheepState::WANDERING:  return "Wandering";
        case Sheep::SheepState::SEEKING:     return "Seeking";
        case Sheep::SheepState::EATING:      return "Eating";
        case Sheep::SheepState::ESCAPING:    return "Escaping";
        case Sheep::SheepState::DEAD:        return "Dead";
        case Sheep::SheepState::REPRODUCE:   return "Reproduce";
        default:                           return "Unknown";
        }
    }

    // Switch WolfState to string
    inline const char* WolfStateToString(Wolf::WolfState state)
    {
        switch (state) {
        case Wolf::WolfState::SEEKING:      return "Seeking";
        case Wolf::WolfState::CATCHING:     return "Catching";
        case Wolf::WolfState::EATING:       return "Eating";
        case Wolf::WolfState::SLEEPING:     return "Sleeping";
        case Wolf::WolfState::ATTACKING:    return "Attacking";
        case Wolf::WolfState::ESCAPING:     return "Escaping";
        case Wolf::WolfState::DEAD:         return "Dead";
        default:                          return "Unknown";
        }
    }
}
