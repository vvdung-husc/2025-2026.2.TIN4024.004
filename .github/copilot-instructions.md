<!--
Guidance for AI coding agents working in this repository.
Keep this file concise and focused on discoverable, actionable patterns.
-->
# Copilot instructions (repository-specific)

Purpose: Help an AI coding agent be immediately productive in this course projects repo. The repo contains many student/team PlatformIO/Arduino projects (ESP32 + DHT + OLED/Blynk) organized under `TEAM_*` folders.

Key places to read first
- `README.md` (repo root) — summary and course-level notes.
- `TEAM_*/*/ESP32_DHT_OLED/` or `TEAM_*/<student>/BLYNK_DHT/` — actual device projects.
- Example files:
  - `TEAM_08/TruongDucMaiLinh_22T1020199/BLYNK_DHT/platformio.ini` — typical PlatformIO environment for ESP32.
  - `TEAM_03/.../Blynk_DHT/lib/TM1637-master/README.md` — demonstrates third-party library layout under `lib/`.
  - `TEAM_11/PhanHuuTuanKiet-22T1020183/Traffic DHT Sensor/README.md` — another project README to see variations.

Big picture architecture (what you'll find)
- This is not a single product: it's a collection of small embedded projects. Each team folder contains a self-contained PlatformIO project (or Arduino sketch). Expect a `platformio.ini`, a `src/` or `.ino` entry, and a `lib/` folder for local libraries.
- Common runtime components: ESP32 core (PlatformIO environment), sensor code for DHTxx, display drivers (SSD1306/U8g2/TM1637), and integrations such as Blynk cloud APIs.
- Data flows are simple and local: sensor read → format → display + optional upload (Blynk). No monolithic server code here.

How to build, upload, and debug (explicit commands)
(Assumes Windows PowerShell; PlatformIO CLI is available either via VS Code PlatformIO extension or `pio` in PATH.)

- Build: `pio run -e <env>` (replace `<env>` with the env name from `platformio.ini`, e.g. `esp32dev`).
- Upload (flash): `pio run -t upload -e <env>` or use the PlatformIO VS Code upload action.
- Monitor serial output: `pio device monitor -e <env>` or `pio device monitor -b 115200 -e <env>` if `monitor_speed` isn't in `platformio.ini`.
- Quick check (build-only for default env): `pio run`.
- Lint/check: `pio check` (where projects include test/lint configs).

Project-specific conventions and patterns
- Each device project keeps code self-contained under its team folder. Typical layout: `platformio.ini`, `src/main.cpp` or `src/<project>.ino`, and `lib/` for vendored libs.
- Libraries are often copied under `lib/` (e.g., `lib/TM1637-master`) rather than pulled from the PlatformIO library registry. Prefer editing `lib/` local copies when making local fixes.
- Look for `platformio.ini` to find board env names and upload/monitor settings. Use those env names in CLI commands.
- Many projects use `Serial.begin(115200)` and expect `115200` monitor speed—check `platformio.ini` for `monitor_speed` or project code for the exact baud.

Integration points & external dependencies
- Expect the following external dependencies (look at `platformio.ini` and the `lib/` folder):
  - ESP32 Arduino core (PlatformIO environment)
  - DHT sensor libraries (DHT or DHTesp)
  - Display libraries (Adafruit SSD1306, U8g2, or TM1637)
  - Blynk client libraries for cloud interactions
- If a build fails due to a missing package, inspect `platformio.ini` and the `lib/` folder; these projects often vendor libraries locally.

Helpful examples to reference when editing or fixing
- When changing board or monitor settings, update `platformio.ini` at the project root (example: `TEAM_08/.../BLYNK_DHT/platformio.ini`).
- To add a third-party lib, prefer adding under the project's `lib/` folder so other students' builds remain unchanged.

Debugging tips
- Reproduce locally by building with `pio run` first. If build succeeds but device misbehaves, capture serial logs with `pio device monitor`.
- If an upload fails, check the board environment name and COM port; PlatformIO often auto-detects, but explicit `-e <env>` helps.

Assumptions and rules for edits
- Don't change other teams' source code unless working on a requested cross-team fix. If you must, explain the rationale in the PR description.
- Keep modifications minimal and well-scoped: small bugfix, updated library, or improved README for a single team.
- Preserve each project's `platformio.ini` env names and supplied libraries unless updating for a specific reason.

What to add to PR descriptions (recommended)
- Which team/project you changed (path). Example: `TEAM_08/TruongDucMaiLinh_22T1020199/BLYNK_DHT`.
- Build steps used locally and whether `pio run` passed.
- If device behavior is involved, include sample serial output and the board `env` used.

If something is unclear
- Start by opening the team's `platformio.ini` and `src/` files — they contain the actionable build and runtime settings. If you need more context, ask what team/project to prioritize and whether changes should be applied across multiple teams.

Please review these notes and tell me any missing areas or projects you want covered in more detail.
