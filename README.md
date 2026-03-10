# Deckz

A C++17 card game engine for building arbitrary card games from JSON configuration.

## Overview

Deckz provides a layered engine that handles the common mechanics of card games — decks, hands, turns, phases, card effects, and win conditions — so you can focus on defining your game's rules rather than plumbing.

Games are defined in JSON and wired together through a high-level `Game` API.

## Features

- **JSON-driven** — define card types, cards, deck rules, and turn phases in a config file
- **Effect system** — built-in effects (`deal_damage`, `draw_cards`, `gain_resource`, `lose_resource`) with card attribute references (`attr:key`)
- **Action validation** — composable rule system for validating player actions
- **Win conditions** — plug in any lambda to detect game-over states
- **Turn engine** — automatic phase iteration, draw phases, multi-player rotation
- **Terminal UI** — playable out of the box in the console
- **80 unit tests** — full GoogleTest coverage at every layer

## Building

**Requirements:** CMake 3.20+, C++17 compiler, internet connection (fetches nlohmann/json and GoogleTest via CMake FetchContent)

```bash
cmake -S . -B build
cmake --build build
```

## Running

```bash
./build/terminal_game.exe
```

### Terminal Controls

| Command | Action |
|---|---|
| `play <n>` | Play card at index `n` from your hand |
| `end` | End the current phase |

## Testing

```bash
ctest --test-dir build --output-on-failure
```

## JSON Config

Games are defined in a single JSON file. See `examples/basic/config.json` for a full example.

```json
{
  "card_types": [
    {
      "name": "Spell",
      "attributes": {
        "cost":   { "type": "int", "default": 0 },
        "damage": { "type": "int", "default": 0 }
      },
      "effects": [
        {
          "trigger": "on_play",
          "type": "deal_damage",
          "params": { "target": "opponent", "amount": "attr:damage" }
        }
      ]
    }
  ],
  "cards": [
    { "id": "fireball", "type": "Spell", "attributes": { "cost": 3, "damage": 5 } }
  ],
  "deck_types": [
    { "name": "StandardDeck", "min_cards": 10, "max_cards": 60, "max_copies": 4 }
  ],
  "phases": [
    { "name": "Draw",   "type": "draw",   "draw_count": 1 },
    { "name": "Main",   "type": "action" },
    { "name": "End",    "type": "end" }
  ]
}
```

### Effect Triggers

| Trigger | When |
|---|---|
| `on_play` | Card is played from hand |
| `on_turn_start` | Start of the player's turn |
| `on_turn_end` | End of the player's turn |
| `on_draw` | Card is drawn |
| `custom` | Manually triggered |

### Built-in Effects

| Effect | Params |
|---|---|
| `deal_damage` | `target` (`self`/`opponent`), `amount` |
| `draw_cards` | `count` |
| `gain_resource` | `resource`, `amount` |
| `lose_resource` | `resource`, `amount` |

Param values can be a fixed number (`"3"`) or a card attribute reference (`"attr:damage"`).

## Architecture

```
CardAttribute → CardType → Card
                         → Deck (+ DeckType)
                                   → Hand
                                   → Player (+ resources)
Registry  (JSON loader + Card/Deck factory)

Effect + EffectRegistry + EffectContext
Phase → TurnEngine → GameState (+ win conditions)
Action + ActionValidator (+ Rules::)

Game      ← high-level facade
TerminalUI ← console frontend
```

## Project Structure

```
include/
  engine/   ← engine headers
  ui/       ← UI headers
src/
  ui/       ← UI implementation
examples/
  basic/    ← programmatic example + config
  terminal/ ← playable terminal game
tests/      ← GoogleTest unit tests
```
