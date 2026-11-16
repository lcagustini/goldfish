#!/usr/bin/env luajit

local function die(message)
    print(message)
    os.exit(1)
end

local gameFile = "./game.lua"
local flags = {}
for _, argument in ipairs(arg) do
    if argument:sub(1, 2) == "--" then
        flags[argument] = true
    else
        gameFile = argument
    end
end

local engineDir = arg[0]:match("^(.+)/[^/]+$") or "."
local gameDir = gameFile:match("^(.+)/[^/]+$") or "."

local onWindows = jit.os == "Windows"

local chunk = loadfile(gameFile)
if chunk == nil then
    die(string.format("Game definition %q does not exist or has a syntax error.", gameFile))
end
local game = chunk()
if type(game) ~= "table" or type(game.name) ~= "string" or game.name == "" then
    die(string.format("%s must return { name = \"...\" } (a non-empty CMake project name, no spaces).", gameFile))
end

-- LuaJIT's os.execute returns a raw POSIX wait status (nonzero = failed).
-- On Windows it returns a boolean. Normalize both to success.
local function run(command)
    local result = os.execute(command)
    if type(result) == "number" then
        return result == 0
    end
    return result == true
end

local function inGame(command)
    return "cd '" .. gameDir .. "' && " .. command
end

local target = "./build/" .. game.name .. (onWindows and ".exe" or "")

if flags["--build"] then
    local buildDir = gameDir .. "/build"
    local configure = string.format(
        "CC=clang CXX=clang++ cmake -S '%s' -B '%s' -DGOLDFISH_GAME_NAME='%s' -DGOLDFISH_GAME_DIR='%s' && cmake --build '%s'",
        engineDir, buildDir, game.name, gameDir, buildDir)
    if not run(configure) then
        die("Build failed")
    end
end

if not onWindows then
    print("[OS] Linux")
else
    print("[OS] Windows")
end

local targetHandle = io.open(gameDir .. "/" .. target)
if targetHandle == nil then
    die(string.format("Target %s does not exist. Run compile.lua with --build first.", target))
end
targetHandle:close()

if flags["--dll"] then
    if not onWindows then
        print("No DLL to copy")
    else
        print("Copying DLLs...")
        local pipe = io.popen(inGame("ldd '" .. target .. "' | grep ucrt64 | cut -d ' ' -f 3"))
        local dlls = pipe:read("*a")
        pipe:close()
        if dlls ~= "" then
            os.execute(inGame("cp " .. dlls .. " build/"))
        end
    end
end

if flags["--zip"] then
    if onWindows then
        local pipe = io.popen(inGame("ldd '" .. target .. "' | grep ucrt64 | cut -d ' ' -f 3"))
        local dlls = pipe:read("*a")
        pipe:close()
        os.execute(inGame("zip -j ./build/standalone.zip " .. dlls .. " " .. target))
    else
        os.execute(inGame("zip -j ./build/standalone.zip " .. target))
    end
    os.execute(inGame("zip -r ./build/standalone.zip assets/"))
end

if flags["--run"] then
    if not run(inGame("./" .. target)) then
        os.exit(1)
    end
elseif flags["--debug"] then
    os.execute(inGame("gdb ./" .. target))
end
