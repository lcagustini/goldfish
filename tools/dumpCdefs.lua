local SCALAR_TYPEDEFS = {
    { alias = "entityId", cdef = "typedef char *entityId;" },
    { alias = "framebufferId", cdef = "typedef unsigned int framebufferId;" },
}

local function die(message)
    error(message, 0)
end

local function decodeJson(text)
    local pos = 1

    local function fail(message)
        error(string.format("JSON parse error at byte %d: %s", pos, message), 0)
    end

    local function skipSpace()
        local _, nextPos = text:find("[^ \t\r\n]", pos)
        pos = nextPos or (#text + 1)
    end

    local function utf8Encode(code)
        if code < 0x80 then
            return string.char(code)
        elseif code < 0x800 then
            return string.char(0xC0 + math.floor(code / 0x40), 0x80 + code % 0x40)
        elseif code < 0x10000 then
            return string.char(0xE0 + math.floor(code / 0x1000),
                               0x80 + math.floor(code / 0x40) % 0x40,
                               0x80 + code % 0x40)
        else
            return string.char(0xF0 + math.floor(code / 0x200000),
                               0x80 + math.floor(code / 0x10000) % 0x40,
                               0x80 + math.floor(code / 0x40) % 0x40,
                               0x80 + code % 0x40)
        end
    end

    local ESCAPES = {
        [34] = '"', [92] = "\\", [47] = "/", [98] = "\b",
        [102] = "\f", [110] = "\n", [114] = "\r", [116] = "\t",
    }

    local function parseString()
        pos = pos + 1 -- opening quote
        local start = pos
        local buffer = nil
        while true do
            local stop = text:find('["\\]', pos)
            if stop == nil then fail("unterminated string") end

            if text:byte(stop) == 34 then
                local chunk = text:sub(start, stop - 1)
                pos = stop + 1
                if buffer == nil then return chunk end
                buffer[#buffer + 1] = chunk
                return table.concat(buffer)
            end

            if buffer == nil then buffer = {} end
            buffer[#buffer + 1] = text:sub(start, stop - 1)
            pos = stop + 1
            local e = text:byte(pos)
            if e == nil then fail("unterminated escape") end

            local replacement = ESCAPES[e]
            if replacement ~= nil then
                buffer[#buffer + 1] = replacement
                pos = pos + 1
            elseif e == 117 then -- \uXXXX
                local code = tonumber(text:sub(pos + 1, pos + 4), 16)
                if code == nil then fail("bad \\u escape") end
                pos = pos + 5
                if code >= 0xD800 and code <= 0xDBFF
                   and text:byte(pos) == 92 and text:byte(pos + 1) == 117 then
                    local low = tonumber(text:sub(pos + 2, pos + 5), 16)
                    if low ~= nil and low >= 0xDC00 and low <= 0xDFFF then
                        code = 0x10000 + (code - 0xD800) * 0x400 + (low - 0xDC00)
                        pos = pos + 6
                    end
                end
                buffer[#buffer + 1] = utf8Encode(code)
            else
                fail("unknown escape")
            end
            start = pos
        end
    end

    local function parseNumber()
        local start = pos
        if text:byte(pos) == 45 then pos = pos + 1 end -- '-'
        while true do
            local b = text:byte(pos)
            if b == nil or b < 48 or b > 57 then break end
            pos = pos + 1
        end
        if text:byte(pos) == 46 then -- '.'
            pos = pos + 1
            while true do
                local b = text:byte(pos)
                if b == nil or b < 48 or b > 57 then break end
                pos = pos + 1
            end
        end
        if text:byte(pos) == 101 or text:byte(pos) == 69 then -- e/E
            pos = pos + 1
            local b = text:byte(pos)
            if b == 43 or b == 45 then pos = pos + 1 end -- +/-
            while true do
                local d = text:byte(pos)
                if d == nil or d < 48 or d > 57 then break end
                pos = pos + 1
            end
        end
        local number = tonumber(text:sub(start, pos - 1))
        if number == nil then fail("bad number") end
        return number
    end

    local parseValue

    local function parseObject()
        pos = pos + 1 -- '{'
        local object = {}
        skipSpace()
        if text:byte(pos) == 125 then pos = pos + 1; return object end -- '}'
        while true do
            skipSpace()
            if text:byte(pos) ~= 34 then fail("expected object key") end
            local key = parseString()
            skipSpace()
            if text:byte(pos) ~= 58 then fail("expected ':'") end
            pos = pos + 1
            object[key] = parseValue()
            skipSpace()
            local b = text:byte(pos)
            if b == 44 then -- ','
                pos = pos + 1
            elseif b == 125 then -- '}'
                pos = pos + 1
                return object
            else
                fail("expected ',' or '}'")
            end
        end
    end

    local function parseArray()
        pos = pos + 1 -- '['
        local array = {}
        skipSpace()
        if text:byte(pos) == 93 then pos = pos + 1; return array end -- ']'
        while true do
            array[#array + 1] = parseValue()
            skipSpace()
            local b = text:byte(pos)
            if b == 44 then -- ','
                pos = pos + 1
            elseif b == 93 then -- ']'
                pos = pos + 1
                return array
            else
                fail("expected ',' or ']'")
            end
        end
    end

    parseValue = function()
        skipSpace()
        local b = text:byte(pos)
        if b == 123 then return parseObject() end       -- {
        if b == 91 then return parseArray() end         -- [
        if b == 34 then return parseString() end        -- "
        if b == 116 then pos = pos + 4; return true end   -- true
        if b == 102 then pos = pos + 5; return false end  -- false
        if b == 110 then pos = pos + 4; return nil end    -- null
        if b == 45 or (b ~= nil and b >= 48 and b <= 57) then return parseNumber() end
        fail("unexpected character")
    end

    local value = parseValue()
    skipSpace()
    if pos <= #text then fail("trailing content") end
    return value
end

local function dumpAst(clang, header, includes, defines)
    local parts = { clang, "-Xclang", "-ast-dump=json", "-fsyntax-only", "'" .. header .. "'" }
    for _, include in ipairs(includes) do
        parts[#parts + 1] = "-I '" .. include .. "'"
    end
    for _, define in ipairs(defines) do
        parts[#parts + 1] = "-D '" .. define .. "'"
    end

    local stdoutFile = os.tmpname()
    local stderrFile = os.tmpname()
    local ok = os.execute(table.concat(parts, " ") .. " > '" .. stdoutFile .. "' 2> '" .. stderrFile .. "'")

    if not ok then
        local errorHandle = io.open(stderrFile)
        local stderrText = errorHandle and errorHandle:read("*a") or ""
        if errorHandle then errorHandle:close() end
        os.remove(stdoutFile)
        os.remove(stderrFile)
        die("[nativeCdefs] clang AST dump failed:\n" .. stderrText)
    end

    local outHandle = io.open(stdoutFile)
    local jsonText = outHandle:read("*a")
    outHandle:close()
    os.remove(stdoutFile)
    os.remove(stderrFile)

    return decodeJson(jsonText)
end

local function findRecords(root)
    local records = {}

    local function walk(node)
        if node.kind == "RecordDecl" and node.name
           and (node.tagUsed == "struct" or node.tagUsed == "union") then
            local existing = records[node.name]
            if existing == nil or (node.inner ~= nil and #node.inner > 0) then
                records[node.name] = node
            end
        end
        local inner = node.inner
        if inner ~= nil then
            for index = 1, #inner do
                walk(inner[index])
            end
        end
    end

    walk(root)
    return records
end

local function splitArray(qualType)
    local dims = ""
    local base = qualType
    while true do
        local dim = base:match("%[%d+%]$")
        if dim == nil then break end
        dims = dim .. dims
        base = base:sub(1, #base - #dim)
    end
    return base, dims
end

local function isUnnamedType(base)
    local tag
    if base:sub(1, 7) == "struct " then tag = "struct"
    elseif base:sub(1, 6) == "union " then tag = "union"
    else return nil end
    local rest = base:sub(#tag + 2)
    if rest:sub(1, 11) == "(unnamed at" and rest:sub(-1) == ")" then
        return tag
    end
    return nil
end

local function fieldValueText(qualType, recordName, fieldName)
    if qualType:find("%(%s*%*") then
        die("[nativeCdefs] function type in " .. recordName .. "." .. fieldName .. ": " .. qualType)
    end

    local base, dims = splitArray(qualType)

    if base:sub(1, 5) == "enum " then
        base = "int"
    end

    local body = base:match("^struct%s+(.+)$") or base:match("^union%s+(.+)$")
                 or base:match("^enum%s+(.+)$") or base
    if body:match("^GL%w+") then
        return "unsigned int", dims
    end

    return base, dims
end

local function emitFields(record, write, indent, emitted)
    indent = indent or ""
    local held = nil

    local function flushHeld()
        if held == nil then return end
        write(indent .. held.tagUsed .. " {\n")
        emitFields(held, write, indent .. "    ", emitted)
        write(indent .. "};\n")
        held = nil
    end

    local inner = record.inner or {}
    for _, member in ipairs(inner) do
        local kind = member.kind

        if kind == "EnumDecl" then
        elseif kind == "IndirectFieldDecl" then
        elseif kind == "RecordDecl" then
            flushHeld()
            if member.name == nil or member.name == "" then
                held = member
            end
        elseif kind == "FieldDecl" then
            local bitfield = member.isBitfield
            if bitfield == true or (type(bitfield) == "number" and bitfield ~= 0) then
                die("[nativeCdefs] bitfield member not supported in " .. record.name)
            end

            local fieldName = member.name
            if fieldName == nil or fieldName == "" then
                flushHeld() -- hidden member of an inline anonymous view
            else
                local base, dims = fieldValueText(member.type.qualType, record.name, fieldName)

                if isUnnamedType(base) then
                    if held == nil then
                        die("[nativeCdefs] " .. record.name .. ": unnamed type member "
                            .. fieldName .. " has no anon record")
                    end
                    write(indent .. held.tagUsed .. " {\n")
                    emitFields(held, write, indent .. "    ", emitted)
                    write(indent .. "} " .. fieldName .. dims .. ";\n")
                    held = nil
                else
                    flushHeld()
                    write(indent .. base .. " " .. fieldName .. dims .. ";\n")

                    local refName = base:match("^struct ([%w_]+)") or base:match("^union ([%w_]+)")
                    if refName ~= nil and base:find("*", 1, true) == nil
                       and emitted ~= nil and emitted[refName] == nil then
                        die("[nativeCdefs] " .. record.name .. " references " .. refName
                            .. " by value before its block; fix the exposure list order")
                    end
                end
            end
        else
            die("[nativeCdefs] unknown member kind in " .. record.name .. ": " .. kind)
        end
    end

    flushHeld()
end

local function generate(clang, header, typesPath, outPath, includes, defines)
    local typesHandle = io.open(typesPath)
    if typesHandle == nil then
        die("[nativeCdefs] cannot open exposure list: " .. typesPath)
    end
    local exposure = {}
    for line in typesHandle:lines() do
        local name = line:match("^%s*(.-)%s*$")
        if name ~= "" then exposure[#exposure + 1] = name end
    end
    typesHandle:close()

    local ast = dumpAst(clang, header, includes, defines)
    local records = findRecords(ast)

    local blocks = {}
    local refs = {}
    local emitted = {}
    for _, typeName in ipairs(exposure) do
        local record = records[typeName]
        if record == nil then
            die("[nativeCdefs] exposure type not found in " .. header .. ": " .. typeName)
        end

        local tag = record.tagUsed
        local fields = {}
        emitFields(record, function(text) fields[#fields + 1] = text end, "    ", emitted)
        local block = tag .. " " .. typeName .. " {\n" .. table.concat(fields) .. "};\n"
            .. "typedef " .. tag .. " " .. typeName .. " " .. typeName .. ";\n"
        blocks[#blocks + 1] = block
        refs[#refs + 1] = table.concat(fields)
        emitted[typeName] = true
    end

    local outputDir = outPath:match("^(.+)/") or "."
    os.execute("mkdir -p '" .. outputDir .. "'")

    local outHandle = io.open(outPath, "w")
    if outHandle == nil then
        die("[nativeCdefs] cannot open output file: " .. outPath)
    end
    outHandle:write("-- Generated by goldfish/tools/dumpCdefs.lua — do not edit. Self-contained Lua.\n\n")
    outHandle:write('local ffi = require("ffi")\n')
    outHandle:write("ffi.cdef[[\n")
    for _, pair in ipairs(SCALAR_TYPEDEFS) do
        for _, ref in ipairs(refs) do
            if ref:find(pair.alias, 1, true) then
                outHandle:write(pair.cdef .. "\n")
                break
            end
        end
    end
    for _, block in ipairs(blocks) do
        outHandle:write("\n" .. block)
    end
    outHandle:write("]]\n")
    outHandle:close()
end

local function main(argument)
    local clang, header, typesPath, outPath
    local includes, defines = {}, {}
    local runSelfTest = false

    local index = 1
    while index <= #argument do
        local item = argument[index]
        if item == "--selftest" then
            runSelfTest = true
        elseif item == "-I" or item == "-D" then
            index = index + 1
            if argument[index] == nil then die("missing value after " .. item) end
            if item == "-I" then includes[#includes + 1] = argument[index]
            else defines[#defines + 1] = argument[index] end
        elseif item:sub(1, 2) == "-I" and #item > 2 then
            includes[#includes + 1] = item:sub(3)
        elseif item:sub(1, 2) == "-D" and #item > 2 then
            defines[#defines + 1] = item:sub(3)
        elseif clang == nil then clang = item
        elseif header == nil then header = item
        elseif typesPath == nil then typesPath = item
        elseif outPath == nil then outPath = item
        else die("unexpected argument: " .. item) end
        index = index + 1
    end

    if runSelfTest then
        selfTest()
        return
    end

    if clang == nil or header == nil or typesPath == nil or outPath == nil then
        die("usage: dumpCdefs.lua <clang> <header> <nativeTypes.txt> <outputLua> [-I dir] [-D def] [--selftest]")
    end

    generate(clang, header, typesPath, outPath, includes, defines)
end

local ok, err = xpcall(main, function(value) return value end, arg)
if not ok then
    io.stderr:write(tostring(err) .. "\n")
    os.exit(1)
end
