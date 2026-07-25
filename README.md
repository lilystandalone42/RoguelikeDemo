# RoguelikeDemo

A top-down, Hades-style roguelike combat demo built in **Unreal Engine 5.4** with **C++**.
It is a gameplay-programming showcase — mechanics and systems are the focus, so visuals
use engine primitive shapes (cubes, cylinders, spheres) as placeholders.

## Features

### Combat
- Melee attack with sphere-overlap + cone hit detection
- 3-hit combo chain with escalating damage, range, and knockback
- Directional knockback on hit (`LaunchCharacter`)
- Crit and lifesteal stats wired into the damage pipeline
- Dash with i-frames and cooldown

### Enemy AI
- State-machine AI controllers (Idle / Chase / Attack / Cooldown)
- `AIPerception` (sight) for target acquisition, with a re-acquire safety net
- Melee enemy: chases and attacks in range
- Ranged enemy: keeps distance, kites, and fires projectiles
- NavMesh pathing with a fallback to the nearest reachable point

### Roguelike Loop
- `RoomManager` drives an arena loop: spawn wave → clear → portal → next room
- Five rooms of escalating difficulty (mix of melee and ranged enemies)
- Off-arena "rescue" so knocked-out enemies never soft-lock a room

### Pickups / Buffs
- Extensible `APickupBase` class; each buff is a small subclass
- Health, Attack, Move Speed, Lifesteal, Crit, and Max Health
- One random pickup drops per cleared room

### UI / Flow
- Canvas HUD: health bar + live stat panel (ATK / CRIT / LIFESTEAL / SPEED)
- Player death handling with on-screen notice and level restart

## Project Structure

```
Source/RoguelikeDemo/
├── Character/    Base, Player, Enemy, RangedEnemy
├── Combat/       CombatComponent, projectiles
├── AI/           Melee + ranged AI controllers
├── Items/        PickupBase and buff pickups
├── Room/         RoomManager, ExitPortal
└── UI/           Canvas HUD
```

## Building

- Unreal Engine 5.4
- Generate project files and build the `RoguelikeDemoEditor` target, or open
  `RoguelikeDemo.uproject` and let the editor compile.

## Controls

Input uses **Enhanced Input**; bindings live in the `IMC_Player` mapping context.
The bound actions are **Move**, **Attack**, and **Dash**, with the character aiming
toward the mouse cursor.
