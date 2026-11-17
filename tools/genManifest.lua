#!/usr/bin/env luajit

-- Maps the game at configure time: reads game.lua, classifies every other .lua
-- file as a component data file or a system script, and writes the manifest the
-- runtime loads. Exits nonzero on any validation failure.

local function die(message)
    print(message)
    os.exit(1)
end

local gameDir = arg[1]
local outFile = arg[2]
if gameDir == nil or outFile == nil then
    die("usage: genManifest.lua <gameDir> <outFile>")
end

local gameFile = gameDir .. "/game.lua"
local chunk = loadfile(gameFile)
if chunk == nil then
    die(string.format("Game definition %q does not exist or has a syntax error.", gameFile))
end
local game = chunk()
if type(game) ~= "table" or type(game.name) ~= "string" or game.name == "" then
    die(string.format("%s must return { name = \"...\" } (a non-empty name).", gameFile))
end

local scenes = game.scenes or {}
if type(scenes) ~= "table" or #scenes == 0 then
    die(string.format("%s must register at least one scene: scenes = { \"path\" }.", gameFile))
end

local sceneSet = {}
for index, path in ipairs(scenes) do
    if type(path) ~= "string" then
        die(string.format("%s: scenes[%d] must be a string path.", gameFile, index))
    end
    if io.open(gameDir .. "/" .. path) == nil then
        die(string.format("Registered scene %q does not exist.", path))
    end
    sceneSet[path] = true
end

-- List every .lua file under the game folder.
local onWindows = package.config:sub(1, 1) == "\\"
local command
if onWindows then
    command = "dir /s /b \"" .. gameDir .. "\""
else
    command = "find '" .. gameDir .. "' -name '*.lua'"
end
local pipe = io.popen(command)
if pipe == nil then
    die("Could not list files under " .. gameDir)
end
local listing = pipe:read("*a")
pipe:close()

local gameDirNormalized = gameDir:gsub("\\", "/")
local function toRelative(path)
    path = path:gsub("\\", "/"):gsub("\r", "")
    if path:sub(1, #gameDirNormalized + 1) == gameDirNormalized .. "/" then
        return path:sub(#gameDirNormalized + 2)
    end
    return path
end

local function skip(relativePath)
    if relativePath == "game.lua" or relativePath:match("/game%.lua$") then return true end
    if relativePath:sub(1, 1) == "." or relativePath:match("/%.") then return true end
    if relativePath:sub(1, 6) == "build/" or relativePath:match("/build/") then return true end
    if sceneSet[relativePath] then return true end
    return false
end

local candidates = {}
for path in listing:gmatch("[^\r\n]+") do
    local relativePath = toRelative(path)
    if relativePath:match("%.lua$") and not skip(relativePath) then
        candidates[#candidates + 1] = relativePath
    end
end
table.sort(candidates)

-- A no-op engine stub: any chain of fields stays callable, so system scripts run
-- harmlessly here. A file that returns a table with a components section is data.
local magic
magic = setmetatable({}, {
    __index = function() return magic end,
    __call = function() return magic end,
})

local sandbox = {
    engine = magic,
    math = math,
    string = string,
    table = table,
    tostring = tostring,
    tonumber = tonumber,
    ipairs = ipairs,
    pairs = pairs,
    next = next,
    select = select,
    type = type,
    unpack = unpack,
    error = error,
    assert = assert,
    setmetatable = setmetatable,
    getmetatable = getmetatable,
    print = function() end,
}

local components = {}
local systems = {}
for _, relativePath in ipairs(candidates) do
    local fileChunk = loadfile(gameDir .. "/" .. relativePath)
    if fileChunk == nil then
        die(string.format("Cannot load %q.", relativePath))
    end
    setfenv(fileChunk, sandbox)
    local ok, result = pcall(fileChunk)
    if not ok then
        die(string.format("%s failed under the manifest sandbox:\n%s", relativePath, tostring(result)))
    end

    if type(result) == "table" then
        local declarations = result.components
        if type(declarations) ~= "table" then
            die(string.format("%s returns a table without a components section; system scripts must not return a table.", relativePath))
        end
        for name, fields in pairs(declarations) do
            if components[name] ~= nil then
                die(string.format("Component %q is declared in two files.", name))
            end
            components[name] = fields
        end
    else
        systems[#systems + 1] = relativePath
    end
end

-- Serializes numbers, strings, booleans, and tables of those. Pure arrays stay
-- on one line; records expand one key per line.
local function serialize(value, indent)
    local valueType = type(value)
    if valueType == "number" then return string.format("%.14g", value) end
    if valueType == "boolean" then return tostring(value) end
    if valueType == "string" then return string.format("%q", value) end
    if valueType == "table" then
        local items = {}
        local arrayLength = #value
        for index = 1, arrayLength do
            items[#items + 1] = serialize(value[index], indent .. "    ")
        end
        local keys = {}
        for key in pairs(value) do
            if type(key) == "string" then keys[#keys + 1] = key end
        end
        table.sort(keys)
        for _, key in ipairs(keys) do
            items[#items + 1] = "[" .. string.format("%q", key) .. "] = " .. serialize(value[key], indent .. "    ")
        end
        if #items == 0 then return "{}" end
        if #items == arrayLength then
            return "{ " .. table.concat(items, ", ") .. " }"
        end
        local childIndent = indent .. "    "
        return "{\n" .. childIndent .. table.concat(items, ",\n" .. childIndent) .. "\n" .. indent .. "}"
    end
    die("Manifest value of type " .. valueType .. " cannot serialize.")
end

local output = io.open(outFile, "w")
if output == nil then
    die(string.format("Cannot write manifest to %q.", outFile))
end
output:write("-- Generated by goldfish/tools/genManifest.lua — do not edit. Self-contained Lua.\n")
output:write("return {\n")
output:write("    name = " .. string.format("%q", game.name) .. ",\n")
output:write("    components = " .. serialize(components, "    ") .. ",\n")
output:write("    systems = " .. serialize(systems, "    ") .. ",\n")
output:write("    scenes = " .. serialize(scenes, "    ") .. ",\n")
output:write("}\n")
output:close()

print("[genManifest] " .. #systems .. " system scripts, " ..
    (function()
        local count = 0
        for _ in pairs(components) do count = count + 1 end
        return count
    end)() .. " components, " .. #scenes .. " scenes")
