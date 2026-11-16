-- All FFI lives here; game scripts never require("ffi").
local ffi = require("ffi")

engine = engine or {}

engine.phases = {
    preUpdate = 0,
    update = 1,
    postUpdate = 2,
    renderSort = 3,
    renderSetup = 4,
    renderOpaque = 5,
    renderSkybox = 6,
    renderTransparent = 7,
    renderPost = 8,
    renderFinish = 9,
}

engine.events = {
    onComponentAdd = 0,
}

engine.declared = {}

local TYPE_ALIASES = {
    vec2 = "struct vec2",
    vec3 = "struct vec3",
    quat = "struct quat",
    mat4 = "union mat4",
}

function engine.declare(name, fields)
    local cdef = { "struct ", name, " {\n" }
    for _, field in ipairs(fields) do
        local base = TYPE_ALIASES[field[2]] or field[2]
        if field[3] ~= nil then
            cdef[#cdef + 1] = "    " .. base .. " " .. field[1] .. "[" .. field[3] .. "];\n"
        else
            cdef[#cdef + 1] = "    " .. base .. " " .. field[1] .. ";\n"
        end
    end
    cdef[#cdef + 1] = "};\n"
    cdef[#cdef + 1] = "typedef struct " .. name .. " " .. name .. ";\n"

    ffi.cdef(table.concat(cdef))
    engine.declared[name] = fields
    return engine.declare0(name, ffi.sizeof(name))
end

function engine.vec2(x, y) return ffi.new("struct vec2", x, y) end
function engine.vec3(x, y, z) return ffi.new("struct vec3", x, y, z) end
function engine.quat(x, y, z, w) return ffi.new("struct quat", x, y, z, w) end

ffi.cdef[[
float vectorLenSquared2D(struct vec2 v);
float vectorLenSquared(struct vec3 v);
struct vec2 vectorNormalize2D(struct vec2 v);
struct vec3 vectorNormalize(struct vec3 v);
struct vec2 vectorScale2D(float a, struct vec2 v);
struct vec3 vectorScale(float a, struct vec3 v);
struct vec3 vectorAdd(struct vec3 a, struct vec3 b);
struct vec3 vectorSubtract(struct vec3 a, struct vec3 b);
struct vec3 vectorRotate(struct vec3 v, struct quat q);
struct quat quatMult(struct quat a, struct quat b);
struct quat eulerToQuat(struct vec3 v);
struct quat quatFromAxisAngle(struct vec3 axis, float angle);
]]
engine.math = ffi.C

-- The engine owns the id; scripts cannot free it
function engine.rawEntity(entity)
    return ffi.cast("entityId", engine.entityIdPointer(entity))
end

function engine.log(message)
    return engine.log0(message)
end

function engine.getComponent(entity, name)
    local pointer = engine.getComponent0(entity, name)
    if pointer == nil then return nil end
    return ffi.cast("struct " .. name .. " *", pointer)
end

function engine.getSingleton(name)
    local pointer = engine.getSingleton0(name)
    if pointer == nil then return nil end
    return ffi.cast("struct " .. name .. " *", pointer)
end

-- Bounds come from the live C row count, never MAX_ENTITY_COUNT.
function engine.view(run, filterIdx, ctype)
    local pointer, rows = engine.getColumn(run, filterIdx, ctype)
    if pointer == nil then
        error(string.format("engine.view: no column %q for filter index %d", ctype, filterIdx))
    end

    local column = ffi.cast("struct " .. ctype .. " *", pointer)
    return setmetatable(
        { rows = rows },
        {
            __index = function(viewTable, row)
                local index = tonumber(row)
                if index == nil then error("engine.view: numeric row index expected") end
                if index < 0 or index >= viewTable.rows then
                    error(string.format("engine.view: row %d out of bounds 0..%d for %s", index, viewTable.rows - 1, ctype))
                end
                return column[index]
            end,
        }
    )
end

-- Called from C after all scripts load.
function engine.sweep()
    local registry = engine.registry()
    for _, entry in ipairs(registry) do
        local ok, size = pcall(ffi.sizeof, entry.name)
        if not ok then
            error(string.format("cdef drift: %q has no ffi declaration", entry.name), 0)
        end
        if size ~= entry.size then
            error(string.format("cdef drift: %q ffi.sizeof = %d, ecs registry = %d bytes", entry.name, size, entry.size), 0)
        end
    end
end

return engine
