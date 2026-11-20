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

-- declareComponents(map): declare every component from a { name = fields } table.
function engine.declareComponents(components)
    for name, fields in pairs(components or {}) do
        engine.declare(name, fields)
    end
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

-- Scene data conventions:
--   component value true      -> add with defaults
--   component value table     -> add, then set each field
--   field value number        -> set direct
--   field value {n, n}        -> vec2 / vec3 / quat by length 2 / 3 / 4
--   field rotationEuler       -> eulerToQuat into the rotation field
--   transform field parent    -> entity name string; resolved after all entities exist
--   entity model = "path"     -> loadModel; texture keys pass through, nil omitted
local function toVector(value)
    if type(value) ~= "table" then return value end
    local count = #value
    if count == 2 then return engine.vec2(value[1], value[2]) end
    if count == 3 then return engine.vec3(value[1], value[2], value[3]) end
    if count == 4 then return engine.quat(value[1], value[2], value[3], value[4]) end
    error("scene: cannot convert number array of length " .. count, 0)
end

local function setFields(record, fields, scene)
    for field, value in pairs(fields) do
        if field == "rotationEuler" then
            record.rotation = engine.math.eulerToQuat(toVector(value))
        elseif field == "parent" then
            -- entityId is a raw char*: a Lua string pointer would dangle, so the
            -- write goes through engine.rawEntity after all entities exist.
            if type(value) ~= "string" then error("scene: parent must be an entity name string", 0) end
            scene.deferredParents[#scene.deferredParents + 1] = { record = record, name = value }
        else
            record[field] = toVector(value)
        end
    end
end

function engine.loadScene(definition)
    local scene = { entities = {}, filters = {}, systems = {}, singletons = {}, deferredParents = {} }

    for name, components in pairs(definition.filters or {}) do
        engine.addFilter(name, components)
        scene.filters[#scene.filters + 1] = name
    end

    for _, system in ipairs(definition.systems or {}) do
        local phase = engine.phases[system.phase]
        if phase == nil then error("scene: unknown phase " .. tostring(system.phase), 0) end
        engine.addPhaseSystem(phase, system.fn, system.filters)
        scene.systems[#scene.systems + 1] = { phase = phase, name = system.fn }
    end

    for _, name in ipairs(definition.singleton or {}) do
        engine.addSingleton(name)
        scene.singletons[#scene.singletons + 1] = name
    end

    local byName = {}
    for _, entry in ipairs(definition.entities or {}) do
        local id
        if entry.model ~= nil then
            id = engine.loadModel(entry.model, entry.diffuse, entry.normal, entry.specular, entry.reflectance, entry.transparent)
        else
            id = engine.createEntity(entry.name or "Entity")
        end
        scene.entities[#scene.entities + 1] = id
        if entry.name ~= nil then byName[entry.name] = id end

        for component, fields in pairs(entry.components or {}) do
            if type(fields) ~= "table" and fields ~= true then
                error("scene: component '" .. component .. "' must be true or a field table", 0)
            end
            -- loadModel already added the transform; adding it again would move the entity to a duplicate table.
            if entry.model == nil or component ~= "transformComponent" then
                engine.addComponent(id, component)
            end
            if type(fields) == "table" then
                setFields(engine.getComponent(id, component), fields, scene)
            end
        end
    end

    for _, deferred in ipairs(scene.deferredParents) do
        local parent = byName[deferred.name]
        if parent == nil then error("scene: unknown parent entity '" .. deferred.name .. "'", 0) end
        deferred.record.parent = engine.rawEntity(parent)
    end

    if definition.skybox ~= nil then
        scene.entities[#scene.entities + 1] = engine.loadSkybox(definition.skybox)
    end

    engine.activeScene = scene
    return scene
end

-- Call between frames only: removing a running system corrupts the phase loop.
function engine.unloadScene()
    local scene = engine.activeScene
    if scene == nil then return end

    for _, system in ipairs(scene.systems) do engine.removePhaseSystem(system.phase, system.name) end
    for _, id in ipairs(scene.entities) do engine.deleteEntity(id) end
    for _, name in ipairs(scene.filters) do engine.removeFilter(name) end
    for _, name in ipairs(scene.singletons) do engine.removeSingleton(name) end

    engine.activeScene = nil
end

function engine.loadSceneFile(path)
    local chunk = loadfile(path)
    if chunk == nil then error("scene file not found: " .. tostring(path), 0) end
    local definition = chunk()
    if type(definition) ~= "table" then
        error("scene file must return a table: " .. tostring(path), 0)
    end
    engine.unloadScene()
    return engine.loadScene(definition)
end

-- uniformRenderData.type values; matching the anonymous enum in material.h.
engine.uniformTypes = {
    MATRIX_4 = 0,
    VECTOR_3 = 1,
    VECTOR_2 = 2,
    INT = 3,
    FLOAT = 4,
}

-- mesh must be a struct meshRenderData pointer (rendererData.opaqueMeshes + index).
function engine.uniformLocation(mesh, name)
    return engine.uniformLocation0(mesh, name)
end

-- value: number, {x, y[, z]}, or cdata vec2/vec3/mat4.
-- Must run after the rendererGet* systems; they rebuild the arrays.
function engine.setUniform(rendererData, meshIndex, name, value)
    -- Pointer arithmetic: indexing would copy, this reaches live storage.
    local mesh = rendererData.opaqueMeshes + meshIndex

    local capacity = ffi.sizeof(mesh.uniforms) / ffi.sizeof("struct uniformRenderData")
    if mesh.uniformsLength >= capacity then
        error("engine.setUniform: uniform array is full (" .. capacity .. ")", 0)
    end

    local entry = mesh.uniforms[mesh.uniformsLength]
    entry.location = engine.uniformLocation(mesh, name)
    entry.count = 1

    local valueType = type(value)
    if valueType == "number" then
        entry.type = engine.uniformTypes.FLOAT
        entry.data.single = value
    elseif valueType == "table" then
        if #value == 2 then
            entry.type = engine.uniformTypes.VECTOR_2
            entry.data.vector2 = engine.vec2(value[1], value[2])
        elseif #value == 3 then
            entry.type = engine.uniformTypes.VECTOR_3
            entry.data.vector3 = engine.vec3(value[1], value[2], value[3])
        else
            error("engine.setUniform: table value must have 2 or 3 numbers", 0)
        end
    elseif ffi.istype("union mat4", value) then
        entry.type = engine.uniformTypes.MATRIX_4
        entry.data.matrix = value
    elseif ffi.istype("struct vec3", value) then
        entry.type = engine.uniformTypes.VECTOR_3
        entry.data.vector3 = value
    elseif ffi.istype("struct vec2", value) then
        entry.type = engine.uniformTypes.VECTOR_2
        entry.data.vector2 = value
    else
        error("engine.setUniform: value must be a number, table, vec2, vec3 or mat4", 0)
    end

    mesh.uniformsLength = mesh.uniformsLength + 1
end

-- Bounds come from the live C row count, never MAX_ENTITY_COUNT.
--
-- ptr(row) returns a pointer into live storage; struct values passed to C
-- bindings arrive as boxed copies, so pass pointers instead.
function engine.view(run, filterIdx, ctype)
    local pointer, rows = engine.getColumn(run, filterIdx, ctype)
    if pointer == nil then
        error(string.format("engine.view: no column %q for filter index %d", ctype, filterIdx))
    end

    local column = ffi.cast("struct " .. ctype .. " *", pointer)
    return setmetatable(
        {
            rows = rows,
            ptr = function(row)
                if row < 0 or row >= rows then
                    error(string.format("engine.ptr: row %d out of bounds 0..%d for %s", row, rows - 1, ctype))
                end
                return column + row
            end,
        },
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
