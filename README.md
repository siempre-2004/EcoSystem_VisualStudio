Overview:

This ecosystem simulation demonstrates a dynamic environment where sheep, wolves, grass, manure, and a player-controlled herder interact using finite state machines and AStar pathfinding.

Sheep can eat, reproduce, wander, escape from wolves, and generate manure that fertilizes grass. Wolves hunt sheep using AStar pathfinding and can also attack the herder or escape when nearby. Grass grows through various life stages and regrows after being eaten. Manure spreads grass growth over time. The herder, controlled by the player, can influence the ecosystem by moving around the terrain.When a wolf attacks the herder, the herder briefly flashes red as a visual indicator of being hit.

Each entity’s behavior changes dynamically based on their state, hunger, and surroundings. The simulation visualizes paths, states, and health in real time.

Instructions(How to use):

Key F1: Toggle debug mode (show tile grid, path lines) Key F2: Toggle between Editor Mode and View Mode Left Click: Set movement destination for the herder Right Click in View Mode: Select an entity to view its state and properties Right Click in Editor Mode: Modify tiles on the map (e.g., add grass or obstacles)
