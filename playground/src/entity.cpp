// entity.cpp

#include "entity.hpp"
#include <vector>
#include <iostream>

namespace sim
{
    bool Ground::is_walkable() const
    {
        return m_walkable;
    }

    void Ground::set_walkable(bool state)
    {
        m_walkable = state;
    }

    void Ground::set_tile_coord(const Point& coord)
    {
        m_tile_coord = coord;
    }

    bool Grass::is_alive() const
    {
        return !(m_state == GrassState::NONE || m_state == GrassState::EATEN);
    }

    float Grass::get_age() const
    {
        return m_age;
    }

    void Grass::set_age(float age)
    {
        m_age = age;
    }

    void Grass::set_tile_coord(const Point& coord)
    {
        m_tile_coord = coord;
    }

    void Grass::update(float dt)
    {//Controls the growth cycle of the grass, driving state changes by age and timer
        if (m_state == GrassState::EATEN) {
            m_regrowTimer += dt;
            if (m_regrowTimer > 40.0f) {
                m_state = GrassState::SEED;
                m_age = 0.0f;
                m_regrowTimer = 0.0f;
            }
            return;
        }
        switch (m_state) { //The lifecycle state machine of grass automatically changes according to the age control state
            case GrassState::SEED:
                if (m_age > 5.0f) m_state = GrassState::GERMINATION;
                break;
            case GrassState::GERMINATION:
                if (m_age > 10.0f) m_state = GrassState::GROWN;
                break;
            case GrassState::GROWN:
                if (m_age > 15.0f) m_state = GrassState::WILT;
                break;
            case GrassState::WILT:
                if (m_age > 20.0f) m_state = GrassState::SEED;
                m_age = 0.0f;
                m_regrowTimer = 0.0f;
                break;
            default:
                break;
        }
        m_age += dt * 2.0f;//Time is multiplied by a double speed to simulate rapid growth
    }

    void Grass::eatenBySheep()
    {
        m_state = GrassState::EATEN;
        m_age = -1;
    }

    void Sheep::set_position(const Vector2& position)
    {
        m_position = position;
    }

    void Sheep::set_direction(const Vector2& direction)
    {
        m_direction = direction;
    }

    void Sheep::set_radius(float radius)
    {
        m_radius = radius;
    }

    void Sheep::set_sprite_flip_x(bool state)
    {
        m_flip_x = state;
    }

    void Sheep::set_sprite_origin(const Vector2& origin)
    {
        m_origin = origin;
    }

    void Sheep::set_sprite_source(const Rectangle& source)
    {
        m_source = source;
    }


    void Sheep::update(float dt)
    {
        if (m_state == SheepState::DEAD) return; //Death is no longer updated

        if (m_reproductionCooldown > 0.0f) {
            m_reproductionCooldown -= dt;
        }

        //The update frequency is different in different states
        m_updateTimer += dt;
        float updateInterval = 0.02f;

        if (m_state == SheepState::WANDERING) updateInterval = 0.03f;
        else if (m_state == SheepState::SEEKING) updateInterval = 0.02f;

        if (m_updateTimer < updateInterval)
            return;

        m_updateTimer = 0.0f;
        //Perceive wolves or grass, and determine state changes
        sense();
        //Control the timer to influence whether or not to seek again
        if (m_isFull) {
            m_satietyTimer -= dt;
            if (m_satietyTimer <= 0.0f) {
                m_isFull = false;
            }
        }
        //If were in the grazing state, the eating behavior is completed first
        if (m_state != SheepState::EATING) {
            decide(dt);  
        }

        if (m_state == SheepState::EATING) {
            act(dt);
            return;
        }
        //If seeking for grass, follow the path
        if (m_state == SheepState::SEEKING && !m_path.empty()) {
            Vector2 nextPos = m_world->tile_coord_to_position(m_path.front());
            float dist = Vector2Distance(m_position, nextPos);

            if (dist < 5.0f) { 
                m_path.erase(m_path.begin());// Short-distance scenario
                for (auto& other : m_world->m_sheep) { // Sheep actively enter the reproduce state after encounter other sheep
                    if (other.get() == this || other->m_state == SheepState::DEAD) continue;

                    float d = Vector2Distance(m_position, other->m_position);
                    if (d < 10.0f &&
                        HP >= REPRODUCE_HP_THRESHOLD &&
                        other->HP >= REPRODUCE_HP_THRESHOLD &&
                        m_reproductionCooldown <= 0.0f &&
                        other->m_reproductionCooldown <= 0.0f)
                    {
                        m_state = SheepState::REPRODUCE;
                        m_reproduceTimer = REPRODUCTION_PAUSE_TIME;
                        reproductionPartner = other;
                        other->m_state = SheepState::REPRODUCE;
                        other->m_reproduceTimer = REPRODUCTION_PAUSE_TIME;
                        other->reproductionPartner = shared_from_this();
                        return;
                    }
                }
            }
            else {
                m_direction = Vector2Normalize(Vector2Subtract(nextPos, m_position));
                m_position = Vector2Add(m_position, Vector2Scale(m_direction, WALKING_SPEED * dt));
            }
        }
        else if (m_state != SheepState::SEEKING) {
            Vector2 velocity = Vector2Scale(m_direction, WALKING_SPEED * dt);
            m_position = Vector2Add(m_position, velocity);
        }

        m_flip_x = m_direction.x > 0.0f;
        act(updateInterval);
        //Hunger accumulates, and blood lost if too hungry
        m_hunger += dt;
        if (m_hunger > 10.0f && m_state != SheepState::REPRODUCE) {
            HP -= static_cast<int>(SHEEP_HUNGER_HP_LOSS * dt / 2.0f);
            if (HP <= 0) {
                HP = 0;
                m_state = SheepState::DEAD;
            }
        }
    }

    void Sheep::sense()
    {
        foundGrass = false;
        wolfNearby = false;
        nearestWolfPosition = { 0, 0 };

        Vector2 sheepPos = m_position;

        for (auto& grass : m_world->getGrass()) {
            if (grass.is_alive()) {
                Vector2 grassPos = m_world->tile_coord_to_position(grass.m_tile_coord);
                if (Vector2Distance(sheepPos, grassPos) < (m_world->m_tile_size.x / 2.0f)) {
                    foundGrass = true;
                    break;
                }
            }
        }

        float minDist = 9999.0f;// Initialize to a big num to ensure update it correctly
        for (const auto& wolf : m_world->m_wolf) {
            float dist = Vector2Distance(m_position, wolf.m_position);
            if (dist < 100.0f && dist < minDist) {
                minDist = dist;
                nearestWolfPosition = wolf.m_position;
                wolfNearby = true;
            }
        }
    }

    void Sheep::decide(float dt)
    {
        if (m_state == SheepState::REPRODUCE) return;
        // Sheep find potential reproducing partner actively, as long distance mating
        if (HP >= REPRODUCE_HP_THRESHOLD && m_reproductionCooldown <= 0.0f) {
            for (auto& other : m_world->m_sheep) {
                if (other.get() == this || other->m_state == SheepState::DEAD) continue;

                float dist = Vector2Distance(m_position, other->m_position);
                if (dist < 20.0f &&
                    other->HP >= REPRODUCE_HP_THRESHOLD &&
                    other->m_reproductionCooldown <= 0.0f) {
                    m_state = SheepState::REPRODUCE;
                    m_reproduceTimer = REPRODUCTION_PAUSE_TIME;
                    reproductionPartner = other;
                    other->m_state = SheepState::REPRODUCE;
                    other->m_reproduceTimer = REPRODUCTION_PAUSE_TIME;
                    other->reproductionPartner = shared_from_this();

                    return;
                }
            }
        }

        if (wolfNearby) {
            m_state = SheepState::ESCAPING;
            m_path.clear();
            return;
        }

        if (m_isFull) { 
            m_state = SheepState::WANDERING;
            m_path.clear();
            return;
        }

        for (size_t i = 0; i < m_world->m_sheep.size(); i++) {
            auto& other = m_world->m_sheep[i];
            if (other.get() == this || other->m_state == SheepState::DEAD) continue;
            float dist = Vector2Distance(m_position, other->m_position);
            if (dist < 10.0f && HP >= REPRODUCE_HP_THRESHOLD && other->HP >= REPRODUCE_HP_THRESHOLD &&
                m_reproductionCooldown <= 0.0f && other->m_reproductionCooldown <= 0.0f)
            {//Set the reproducing state and make references to each other's reproducing objects
                m_state = SheepState::REPRODUCE;
                m_reproduceTimer = REPRODUCTION_PAUSE_TIME;
                reproductionPartner = other;
                other->m_state = SheepState::REPRODUCE;
                other->m_reproduceTimer = REPRODUCTION_PAUSE_TIME;
                other->reproductionPartner = shared_from_this();
                return;
            }
        }

        Point currentTile = m_world->position_to_tile_coord(m_position);
        bool grassHere = m_world->has_grass_at(currentTile);
        if (grassHere) { 
            m_state = SheepState::EATING;
            m_eatingTimer = 0.0f;
            m_path.clear();

            Point tile = m_world->position_to_tile_coord(m_position);
            m_position = m_world->tile_coord_to_position(tile);  
            m_direction = { 0,0 };
            return;
        }

        if (m_hunger > 5.0f) {
            m_state = SheepState::SEEKING;
            if (m_path.empty()) {
                Point goal = m_world->findNearestGrass(currentTile);
                if (goal.x < 0 || goal.y < 0) {
                    m_state = SheepState::WANDERING;
                    return;
                }
                m_path = findPath(*m_world, currentTile, goal);
            }
            if (!m_path.empty()) {
                Vector2 nextPos = m_world->tile_coord_to_position(m_path.front());
                float dist = Vector2Distance(m_position, nextPos);
                if (dist < 8.0f) {
                    m_path.erase(m_path.begin());
                    if (m_path.empty()) {
                        Point newTile = m_world->position_to_tile_coord(m_position);
                        if (m_world->has_grass_at(newTile)) {
                            m_state = SheepState::EATING;
                            return;
                        }
                    }
                }
                else {
                    Vector2 dir = Vector2Normalize(Vector2Subtract(nextPos, m_position));
                    m_position = Vector2Add(m_position, Vector2Scale(dir, WALKING_SPEED * dt));
                }
            }
        }
        else {
            if (!m_isFull && m_reproductionCooldown <= 0.0f && HP >= REPRODUCE_HP_THRESHOLD) {
                Point start = m_world->position_to_tile_coord(m_position);
                Point partnerTile = start;
                float nearestDist = 9999.0f;
                std::shared_ptr<Sheep> potentialPartner = nullptr;

                for (const auto& other : m_world->m_sheep) {
                    if (other.get() == this || other->m_state == SheepState::DEAD) continue;
                    if (other->HP >= REPRODUCE_HP_THRESHOLD && other->m_reproductionCooldown <= 0.0f) {
                        float dist = Vector2Distance(m_position, other->m_position);
                        if (dist < 200.0f && dist < nearestDist) {
                            nearestDist = dist;
                            partnerTile = m_world->position_to_tile_coord(other->m_position);
                            potentialPartner = other;
                        }
                    }
                }

                if (potentialPartner && (partnerTile.x != start.x || partnerTile.y != start.y)) {
                    if (m_world->is_walkable(partnerTile)) {
                        m_path = findPath(*m_world, start, partnerTile);
                        if (!m_path.empty()) {
                            m_state = SheepState::SEEKING;
                            return;
                        }
                    }
                }
            }
            m_state = SheepState::WANDERING;
        }
    }

    void Sheep::act(float dt)
    {
        switch (m_state)
        {
            case SheepState::WANDERING:
            { //Sheep in wander state might follow other sheep, imitating the group behaviour
                constexpr float FOLLOW_RADIUS = 150.0f;
                constexpr int FOLLOW_CHANCE_PERCENT = 30; // 30% following potential other sheep

                bool followed = false;
                if (GetRandomValue(0, 100) < FOLLOW_CHANCE_PERCENT) { // find the nearest sheep to follow with
                    float nearestDist = 9999.0f;
                    Vector2 nearestSheepPos = m_position;

                    for (const auto& other : m_world->m_sheep) {
                        if (other.get() == this || other->m_state == SheepState::DEAD) continue;
                        float dist = Vector2Distance(m_position, other->m_position);
                        if (dist < FOLLOW_RADIUS && dist < nearestDist) {
                            nearestDist = dist;
                            nearestSheepPos = other->m_position;
                        }
                    }

                    if (nearestDist < FOLLOW_RADIUS) {
                        Vector2 dirToOther = Vector2Normalize(Vector2Subtract(nearestSheepPos, m_position));
                        m_position = Vector2Add(m_position, Vector2Scale(dirToOther, WALKING_SPEED * dt));
                        followed = true;
                    }
                }

                if (!followed) {
                    Vector2 randomDir = { (float)GetRandomValue(-100, 100) / 100.0f, (float)GetRandomValue(-100, 100) / 100.0f };
                    randomDir = Vector2Normalize(randomDir);
                    m_position = Vector2Add(m_position, Vector2Scale(randomDir, WALKING_SPEED * dt));
                }

                if (m_hunger > 10.0f) {
                    m_state = SheepState::SEEKING;
                }
                m_eatingTimer = 0.0f;
                break;
            }
            case SheepState::SEEKING:
            {
                if (foundGrass) {
                    m_state = SheepState::EATING;
                    m_eatingTimer = 0.0f; // Sheep starts eating
                }
                break;
            }
            case SheepState::EATING:
            {
                if (wolfNearby) {
                    m_state = SheepState::ESCAPING;
                    return;
                }

                m_eatingTimer += dt;
                if (m_eatingTimer >= 3.0f)
                {
                    HP = std::min(HP + SHEEP_HEAL_AMOUNT, SHEEP_MAX_HP);

                    Point tileCoord = m_world->position_to_tile_coord(m_position);

                    bool grassFoundAndEaten = false;
                    for (auto& grass : m_world->getGrass()) {
                        Vector2 grassPos = m_world->tile_coord_to_position(grass.m_tile_coord);
                        if (Vector2Distance(m_position, grassPos) < (m_world->m_tile_size.x / 2.0f) && grass.is_alive()) {
                            grass.eatenBySheep();
                            grassFoundAndEaten = true;
                            foundGrass = false;
                            break;
                        }
                    }

                    if (grassFoundAndEaten) {
                        bool localManureExists = false;
                        for (auto& m : m_world->m_manure) {
                            Point mTile = m_world->position_to_tile_coord(m.m_position);
                            if (mTile == tileCoord && m.m_isActive) {
                                localManureExists = true;
                                break;
                            }
                        }

                        if (!localManureExists) {
                            Manure newManure(m_world);
                            newManure.set_position(m_world->tile_coord_to_position(tileCoord));
                            newManure.set_duration(5.0f);
                            newManure.set_quality((float)GetRandomValue(1, 5));
                            m_world->m_manure.push_back(newManure);
                        }

                        foundGrass = false;
                    }

                    m_hunger = 0;
                    m_eatingTimer = 0.0f;

                    m_isFull = true;                  
                    m_satietyTimer = FULL_DURATION;

                    m_path.clear();
                    m_state = SheepState::WANDERING;

                    m_direction = Vector2Normalize({
            (float)GetRandomValue(-100, 100) / 100.0f,
            (float)GetRandomValue(-100, 100) / 100.0f
                        });
                }
                return;
            }
            case SheepState::ESCAPING:
            {
                if (wolfNearby) {
                    Vector2 fleeDir = Vector2Subtract(m_position, nearestWolfPosition);
                    fleeDir = Vector2Normalize(fleeDir);
                    m_position = Vector2Add(m_position, Vector2Scale(fleeDir, RUNNING_SPEED * dt));
                }
                else {
                    m_state = SheepState::WANDERING;
                }
                break;
            }

            case SheepState::REPRODUCE:
            {
                if (wolfNearby) {
                    m_state = SheepState::ESCAPING;
                    break;
                }
                auto partner = reproductionPartner.lock();

                if (!partner) {
                    m_state = SheepState::WANDERING;
                    m_reproduceTimer = 0.0f;
                    break;
                }
                m_reproduceTimer -= dt;
                m_position.x += (float)GetRandomValue(-2, 2);
                m_position.y += (float)GetRandomValue(-2, 2);//Small movement to reduce the frame movement results

                //Avoid stuck
                constexpr float REPRODUCTION_TIMEOUT = -2.0f;
                if (m_reproduceTimer < REPRODUCTION_TIMEOUT) {
                    m_state = SheepState::WANDERING;
                    m_reproduceTimer = 0.0f;
                    reproductionPartner.reset();
                    break;
                }

                if (m_reproduceTimer <= 0.0f) 
                {
                    if (std::less<Sheep*>()(this, partner.get()) && partner->m_state == SheepState::REPRODUCE)  
                    {
                        auto newSheep = std::make_shared<Sheep>(*m_world);
                        Vector2 newPos = Vector2Scale(Vector2Add(m_position, partner->m_position), 0.5f);
                        newSheep->set_position(newPos);
                        newSheep->set_direction({ 1, 0 });
                        newSheep->set_radius(m_radius);
                        newSheep->set_sprite_source(m_source);
                        newSheep->set_sprite_origin(m_origin);
                        newSheep->m_state = SheepState::WANDERING;
                        m_world->m_sheep.push_back(newSheep);

                        HP -= REPRODUCE_HP_COST;
                        partner->HP -= REPRODUCE_HP_COST;
                        m_reproductionCooldown = REPRODUCTION_COOLDOWN_TIME;
                        partner->m_reproductionCooldown = REPRODUCTION_COOLDOWN_TIME;
                        partner->reproductionPartner.reset();
                        partner->m_state = SheepState::WANDERING;
                        partner->m_reproduceTimer = 0.0f;
                    }
                    m_state = SheepState::WANDERING;
                    m_reproduceTimer = 0.0f;
                    reproductionPartner.reset();
                }
                return;
            }
            case SheepState::DEAD:
                 return;
        }
        m_hunger += dt;
    }

    void Sheep::getEaten() {
        m_state = SheepState::DEAD;
    }

    void Sheep::render(const Texture& texture) const
    {
        Rectangle src = m_source;
        float width = src.width;
        if (m_flip_x) {
            src.width = -src.width;
        }
        Color color = WHITE;
        switch (m_state) { //Each colors represent different states
        case SheepState::WANDERING: color = LIGHTGRAY; break;
        case SheepState::SEEKING: color = GREEN; break;
        case SheepState::EATING: color = BLUE; break;
        case SheepState::ESCAPING: color = RED; break;
        case SheepState::DEAD: color = BLACK; break;
        case SheepState::REPRODUCE: color = PINK; break;
        }
        Rectangle dest = { m_position.x, m_position.y, width, src.height };
        Vector2 origin = m_origin;
        DrawTexturePro(texture, src, dest, origin, 0.0f, color);
    }

    void Sheep::recalculatePath() {
        if (!m_world) return; 
        //The route recalculation function is used to call when the map is changed or the state is switched
        Point start = m_world->position_to_tile_coord(m_position);
        Point goal = m_world->findNearestGrass(start);

        if (goal.x >= 0 && goal.y >= 0 && m_world->is_walkable(goal)) {
            m_path = findPath(*m_world, start, goal);
        }
        else {
            m_path.clear(); 
        }
    }

    void Wolf::set_position(const Vector2& position)
    {
        m_position = position;
    }

    void Wolf::set_direction(const Vector2& direction)
    {
        m_direction = direction;
    }

    void Wolf::set_radius(float radius)
    {
        m_radius = radius;
    }

    void Wolf::set_sprite_flip_x(bool state)
    {
        m_flip_x = state;
    }

    void Wolf::set_sprite_origin(const Vector2& origin)
    {
        m_origin = origin;
    }

    void Wolf::set_sprite_source(const Rectangle& source)
    {
        m_source = source;
    }

    bool Wolf::shouldWakeUp(float dt) {
        m_pauseTimer -= dt;
        if (m_pauseTimer <= 0.0f) {
            m_pauseTimer = 0.0f;
            return true;
        }
        return false;
    }

    void Wolf::update(float dt)
    {
        m_updateTimer += dt;
        constexpr float SOME_DISTANCE = 150.0f;

        float updateInterval = 0.05f;
        if (m_state == WolfState::SEEKING)
            updateInterval = 0.01f;
        else if (m_state == WolfState::CATCHING)
            updateInterval = 0.02f;
        else if (m_state == WolfState::SLEEPING)
            updateInterval = 0.03f;

        if (m_updateTimer < updateInterval)
            return;
        //m_updateTimer -= updateInterval;
        m_updateTimer = 0.0f;

        if (m_state == WolfState::SLEEPING) {
            m_pauseTimer -= dt;
            if (shouldWakeUp(dt)) {
                m_state = WolfState::SEEKING;
            }
            return;
        }

        Vector2 velocity = Vector2Scale(m_direction, WALKING_SPEED * dt);
        m_position = Vector2Add(m_position, velocity);
        m_flip_x = m_direction.x > 0.0f ? true : false;
        sense();
        decide(dt);
        act(updateInterval);

        if (m_state != WolfState::EATING && m_state != WolfState::SLEEPING) {
            m_hunger += dt;
            if (m_hunger > 5.0f) {
                HP -= static_cast<int>(WOLF_TIRED_HP_LOSS * dt / 2.0f);
                if (HP <= 0) {
                    HP = 0;
                    m_state = WolfState::DEAD;
                }
            }
        }
    }

    void Wolf::sense()
    {
        foundSheep = false;
        targetSheep = nullptr;

        float distHerder = FLT_MAX;
        Vector2 herderPos = {};
        if (m_world->m_herder) {
            herderPos = m_world->m_herder->get_position();
            distHerder = Vector2Distance(m_position, herderPos);
        }

        const float HERDER_SAFE_DISTANCE = 150.0f;
        const float HERDER_ATTACK_DISTANCE = 100.0f;

        if (distHerder < HERDER_ATTACK_DISTANCE) {
            // Attack herder first, with shorter distance
            m_state = WolfState::ATTACKING;
            targetSheep = nullptr;
            foundSheep = false;
            m_path.clear();
            return;
        }
        else if (distHerder < HERDER_SAFE_DISTANCE) {
            // Escape afterwards
            m_state = WolfState::ESCAPING;
            targetSheep = nullptr;
            foundSheep = false;
            m_path.clear();
            m_direction = Vector2Normalize(Vector2Subtract(m_position, herderPos));
            return;
        }

        // Herder no around then check the sheep
        float minDist = FLT_MAX;
        for (auto& sheep : m_world->m_sheep) {
            if (sheep->m_state == Sheep::SheepState::DEAD) continue;
            float dist = Vector2Distance(m_position, sheep->m_position);
            if (dist < 200.0f && dist < minDist) {
                minDist = dist;
                targetSheep = sheep.get();
                foundSheep = true;
            }
        }

        if (foundSheep && m_state != WolfState::ATTACKING && m_state != WolfState::ESCAPING) {
            m_state = WolfState::CATCHING;
        }
        else if (!foundSheep && m_state != WolfState::ATTACKING && m_state != WolfState::ESCAPING) {
            m_state = WolfState::SEEKING;
        }
    }

    void Wolf::decide(float dt)
    {
        if (foundSheep) {
            m_state = WolfState::CATCHING;

            if (m_path.empty() || !targetSheep) {
                recalculatePath();
            }
            return;
        }

        if (m_hunger > 15.0f) {
            m_state = WolfState::SEEKING;
            return;
        }
        // Introduce chance to avoid const catching, ensuring balance
        if (m_state == WolfState::SEEKING && GetRandomValue(0, 100) < 1) {  
            m_state = WolfState::SLEEPING;
            m_pauseTimer = 1.5f;
            return;
        }
        //If the state is in catching, but the path is empty, the path is recalculated
        if (m_state == WolfState::CATCHING) {
            if (!m_path.empty()) {
                Vector2 nextPosition = m_world->tile_coord_to_position(m_path.front());
                if (!m_world) { return; }
                if (Vector2Distance(m_position, nextPosition) < 5.0f) {
                    m_path.erase(m_path.begin());
                } else {
                    Vector2 direction = Vector2Normalize(Vector2Subtract(nextPosition, m_position));
                    m_position = Vector2Add(m_position, Vector2Scale(direction, RUNNING_SPEED * dt));
                }
            } else {
                Point start = m_world->position_to_tile_coord(m_position);
                if (!m_world) { return;}
                Point goal = m_world->findNearestSheep(start);
                if (goal.x >= 0) {
                    m_path = findPath(*m_world, start, goal);
                }
                if (goal.x < 0 || goal.y < 0) { return;}
            }
        }

        if (m_state == WolfState::ESCAPING || m_state == WolfState::ATTACKING) {
            m_path.clear();
        }
    }

    void Wolf::act(float dt)
    {
        switch (m_state) {
        case WolfState::SEEKING:
            m_hunger += dt;
            m_randomTimer -= dt;
            if (m_randomTimer <= 0.0f) {// Random direction to avoid go for the same target
                float angle = GetRandomValue(0, 359) * (PI / 180.f);
                m_randomDirection = { cosf(angle), sinf(angle) };
                m_randomTimer = (float)GetRandomValue(1, 3); 
            }
            m_position = Vector2Add(m_position,
                Vector2Scale(m_randomDirection, WALKING_SPEED * dt));
            break;
        case WolfState::CATCHING://&& targetSheep->m_state != SheepState::DEAD
            if (targetSheep) { //Calculate the directional vector towards sheep to chase
                if (!m_path.empty()) {
                    Vector2 nextPosition = m_world->tile_coord_to_position(m_path.front());

                    if (Vector2Distance(m_position, nextPosition) < 5.0f) {
                        m_path.erase(m_path.begin());
                    }
                    else {
                        m_direction = Vector2Normalize(Vector2Subtract(nextPosition, m_position));
                        m_position = Vector2Add(m_position, Vector2Scale(m_direction, RUNNING_SPEED * dt));
                    }
                } else {
                    recalculatePath();  
                }

                if (targetSheep && Vector2Distance(m_position, targetSheep->m_position) < 50.0f) {
                    m_state = WolfState::EATING;
                    targetSheep->getEaten();
                    targetSheep = nullptr;
                    HP = WOLF_MAX_HP;
                    m_hunger = 0;
                    m_path.clear();

                    auto& sheepArray = m_world->m_sheep;
                    sheepArray.erase(
                        std::remove_if(sheepArray.begin(), sheepArray.end(),
                            [](const std::shared_ptr<sim::Sheep>& s) {
                                return s->getState() == sim::Sheep::SheepState::DEAD;
                            }),
                        sheepArray.end());
                }
            }
            break;
        case WolfState::EATING:
            m_hunger = 0;
            m_pauseTimer += dt;
            if (m_pauseTimer < EATING_PAUSE_TIME){
                return;
            }
            else {
                m_pauseTimer = 0.0f;
                m_state = WolfState::SLEEPING;
            }
            break;
        case WolfState::ATTACKING:
        {// Approach the herder to attack
            if (m_world->m_herder) {
                Vector2 herderPos = m_world->m_herder->get_position();  
                Vector2 attackDir = Vector2Normalize(Vector2Subtract(herderPos, m_position));

                m_direction = attackDir;
                m_position = Vector2Add(m_position, Vector2Scale(attackDir, RUNNING_SPEED * dt));

                if (Vector2Distance(m_position, herderPos) < 10.0f) { //The wolf attack the herder then move backwards
                    m_state = WolfState::SLEEPING;
                    m_pauseTimer = 1.0f;

                    m_world->m_herder->m_hitTimer = 1.0f;

                    Vector2 flee = Vector2Normalize(Vector2Subtract(m_position, herderPos));
                    Vector2 newHerderPos = Vector2Add(herderPos, Vector2Scale(flee, 70.0f));

                    // Restrict the herder within bound
                    Rectangle bounds = m_world->m_world_bounds;
                    newHerderPos.x = Clamp(newHerderPos.x, bounds.x + 10, bounds.x + bounds.width - 10);
                    newHerderPos.y = Clamp(newHerderPos.y, bounds.y + 10, bounds.y + bounds.height - 10);

                    m_world->m_herder->set_position(newHerderPos);
                    m_world->m_herder->m_path.clear();
                }
            }
            break;
        }
        case WolfState::ESCAPING: {//Meet the herder and escape from his location
            if (m_world->m_herder) {
                Vector2 herderPos = m_world->m_herder->get_position();
                Vector2 fleeDir = Vector2Normalize(Vector2Subtract(m_position, herderPos));
                m_position = Vector2Add(m_position, Vector2Scale(fleeDir, RUNNING_SPEED * dt));
            }
            break;
        }
        case WolfState::DEAD:
            return;
        }
        m_hunger += dt;
    }

    void Wolf::render(const Texture& texture) const
    {
        Rectangle src = m_source;
        float width = src.width;
        if (m_flip_x) {
            src.x += src.width;
            src.width = -src.width;
        }
        Color color = WHITE;
        switch (m_state) {
        case WolfState::SEEKING: color = ORANGE; break;
        case WolfState::CATCHING: color = RED; break;
        case WolfState::EATING: color = DARKGRAY; break;
        case WolfState::SLEEPING: color = BLUE; break;
        case WolfState::ATTACKING: color = MAGENTA; break;
        case WolfState::DEAD: color = BLACK; return;
        }
        Rectangle dest = { m_position.x, m_position.y, width, src.height };
        Vector2 origin = m_origin;
        DrawTexturePro(texture, src, dest, origin, 0.0f, color);

        DrawText(TextFormat("State: %d\nHP: %d\nHunger: %.1f", (int)m_state, HP, m_hunger),
            static_cast<int>(m_position.x), static_cast<int>(m_position.y) - 40, 10, WHITE);
    }

    void Wolf::recalculatePath() {
        if (!m_world || !targetSheep) {
            m_path.clear();
            return;
        }

        Point start = m_world->position_to_tile_coord(m_position);
        Point goal = m_world->position_to_tile_coord(targetSheep->m_position);

        if (goal.x >= 0 && goal.y >= 0 && m_world->is_walkable(goal)) {
            m_path = findPath(*m_world, start, goal);
        }
        else {
            m_path.clear(); 
        }
    }

    void Manure::set_position(const Vector2& position)
    {
        m_position = position;
    }

    void Manure::set_sprite_source(const Rectangle& source)
    {
        m_source = source;
    }

    void Manure::set_duration(float duration) 
    {
        m_duration = duration;
    }

    void Manure::set_quality(float quality)
    {
        m_quality = quality;
    }

    void Manure::update(float dt)
    {
        m_duration -= dt;
        float initialTime = 5.0f;
        m_alpha = m_duration / initialTime;
        if (m_alpha < 0.0f) m_alpha = 0.0f;
        if (!m_hasSpread && m_duration <= 1.0f) {
            spreadGrass();
            m_hasSpread = true;
        }
        if (m_duration <= 0.0f) {
            m_isActive = false;
        }
    }

    void Manure::render(const Texture& texture) const
    {
        Color c = BLACK;
        c.a = (unsigned char)(m_alpha * 255);
        DrawCircle((int)m_position.x, (int)m_position.y, 8, c);
    }

    void Manure::spreadGrass()
    {
        Point tile = m_world->position_to_tile_coord(m_position);
        //Iterate bordering tiles
        for (int dy = 0; dy <= 1; dy++) {
            for (int dx = -1; dx <= 0; dx++) {
                if (dx == 0 && dy == 0) continue;// Jump over own tile
                Point neighborTile = { tile.x + dx, tile.y + dy };//Calculate neighbour tiles
                if (!m_world->is_valid_coord(neighborTile)) continue;
                // Obtain corresponding grass
                Grass& g = m_world->m_grass[neighborTile.y * m_world->m_world_size.x + neighborTile.x];
                if (g.m_state == Grass::GrassState::NONE) {
                    g.m_state = Grass::GrassState::SEED;
                    g.set_age(0.0f);
                    g.m_regrowTimer = 0.0f;
                    g.m_hasFertilizer = true;
                }
            }
        }
    }

    void Herder::update(float dt) {//Left click to set the target tile
        constexpr float HERDER_WOLF_DETECTION_DISTANCE = 150.0f;

        if (m_hitTimer > 0.0f) {
            m_hitTimer -= dt;
            if (m_hitTimer < 0.0f) m_hitTimer = 0.0f;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            Point target = m_world->position_to_tile_coord(mousePos);
            Point start = m_world->position_to_tile_coord(m_position);
            if (m_world->is_walkable(target)) {
                m_path = findPath(*m_world, start, target);
            }
        }
        //Step by step movement along a path
        if (!m_path.empty()) {
            Vector2 nextPos = m_world->tile_coord_to_position(m_path.front());
            if (Vector2Distance(m_position, nextPos) < 5.0f) {
                m_path.erase(m_path.begin());
            }
            else {
                Vector2 direction = Vector2Normalize(Vector2Subtract(nextPos, m_position));
                m_position = Vector2Add(m_position, Vector2Scale(direction, m_speed * dt));
            }
        }
    }
    //Render character maps and path debug lines
    void Herder::render() {
        Rectangle src = m_source;
        float width = src.width;
        if (m_flip_x) {
            src.x += src.width;
            src.width = -src.width;
        }
        Rectangle dest = { m_position.x - m_origin.x, m_position.y - m_origin.y, src.width, src.height };
        //DrawTexturePro(*m_texture, src, dest, m_origin, 0.0f, WHITE);
        Color herderColor = (m_hitTimer > 0.0f) ? RED : WHITE;
        DrawTexturePro(*m_texture, src, dest, m_origin, 0.0f, herderColor);

        if (m_path.size() > 1) {//debugMode &&
            for (size_t i = 0; i + 1 < m_path.size(); ++i) {
                Vector2 pos1 = m_world->tile_coord_to_position(m_path[i]);
                Vector2 pos2 = m_world->tile_coord_to_position(m_path[i + 1]);
                DrawLineV(pos1, pos2, BLUE);
            }
            DrawText(TextFormat("Herder"), static_cast<int>(m_position.x), static_cast<int>(m_position.y) - 40, 10, WHITE);
        }
    }

    void Herder::set_position(const Vector2& position) {
        m_position = position;
    }

    Vector2 Herder::get_position() const {
        return m_position;
    }

    void Herder::recalculatePath() {
        if (m_path.empty()) return;

        Point currentPos = m_world->position_to_tile_coord(m_position);
        Point goal = m_path.back();

        if (m_world->is_walkable(goal)) {
            m_path = findPath(*m_world, currentPos, goal);
        } else {
            m_path.clear();
        }
    }
}
