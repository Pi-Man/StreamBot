POS_Z = 0
POS_X = 1
NEG_Z = 2
NEG_X = 3

local fluids = { "minecraft:water", "minecraft:lava" }

function contains(list, item)
    for _, i in ipairs(list) do
        if item == i then
            return true
        end
    end
    return false
end

local check_flag = true
function check_inv()
    local flag, err
    if check_flag and turtle.getItemCount(16) > 0 then
        check_flag = false
        local x = turtle.x
        local y = turtle.y
        local z = turtle.z
        flag, err = go_to(0, 0, 1)
        if not flag then
            return false, err
        end
        flag, err = go_to(0, 0, 0)
        if not flag then
            return false, err
        end
        for s = 1,16 do
            turtle.select(s)
            turtle.dumpUp()
        end
        turtle.select(1)
        flag, err = go_to(x, y, z)
        if not flag then
            return false, err
        end
        check_flag = true
    end
    return true, nil
end

function face(dir)
    local ang = dir - turtle.facing
    while ang >= 2 do ang = ang - 4 end
    while ang < -2 do ang = ang + 4 end
    while ang < 0 do
        turtle.turnRight()
        ang = ang + 1
    end
    while ang > 0 do
        turtle.turnLeft()
        ang = ang - 1
    end
    turtle.facing = dir
end

function move()
    local flag
    local err
    flag, err = check_inv()
    if not flag then
        return false, err
    end
    local block
    local info
    block, info = turtle.inspect()
    if not block or info.name == "minecraft:air" or contains(fluids, info.name) then
        flag, err = turtle.forward()
    else
        while turtle.inspect() do
            turtle.dig()
        end
        flag, err = turtle.forward()
    end
    if not flag then
        return false, err
    end
    if turtle.facing == POS_Z then
        turtle.z = turtle.z + 1
    elseif turtle.facing == POS_X then
        turtle.x = turtle.x + 1
    elseif turtle.facing == NEG_Z then
        turtle.z = turtle.z - 1
    elseif turtle.facing == NEG_X then
        turtle.x = turtle.x - 1
    end
    return true, nil
end

function moven(length)
    while length > 0 do
        local flag
        local err
        flag, err = move()
        if not flag then
            return false
        end
        length = length - 1
    end
    return true, nil
end

function moveup()
    local flag
    local err
    flag, err = check_inv()
    if not flag then
        return false, err
    end
    local block
    local info
    block, info = turtle.inspectUp()
    if not block or info.name == "minecraft:air" then
        flag, err = turtle.up()
    else
        flag, err = turtle.digUp()
        if flag then flag, err = turtle.up() end
    end
    if not flag then
        return false, err
    end
    turtle.y = turtle.y + 1
    return true, nil
end

function moveupn(length)
    while length > 0 do
        local flag
        local err
        flag, err = moveup()
        if not flag then
            return false
        end
        length = length - 1
    end
    return true, nil
end

function movedown()
    local flag
    local err
    flag, err = check_inv()
    if not flag then
        return false, err
    end
    local block
    local info
    block, info = turtle.inspectDown()
    if not block or info.name == "minecraft:air" then
        flag, err = turtle.down()
    else
        flag, err = turtle.digDown()
        if flag then flag, err = turtle.down() end
    end
    if not flag then
        return false, err
    end
    turtle.y = turtle.y - 1
    return true, nil
end

function movedownn(length)
    while length > 0 do
        local flag
        local err
        flag, err = movedown()
        if not flag then
            return false
        end
        length = length - 1
    end
    return true, nil
end

function travelX(length)
    if length < 0 then
        face(NEG_X)
    else
        face(POS_X)
    end
    length = math.abs(length)
    local flag
    local err
    flag, err = moven(length)
    if not flag then
        return false
    end
    return true, nil
end

function travelY(length)
    local flag
    local err
    if length < 0 then
        flag, err = movedownn(-length)
    else
        flag, err = moveupn(length)
    end
    if not flag then
        return false
    end
    return true, nil
end

function travelZ(length)
    if length < 0 then
        face(NEG_Z)
    else
        face(POS_Z)
    end
    length = math.abs(length)
    local flag
    local err
    flag, err = moven(length)
    if not flag then
        return false
    end
    return true, nil
end

function go_to(x, y, z)
    local flag
    local err
    flag, err = travelZ(z - turtle.z)
    if not flag then
        return false, err
    end
    flag, err = travelY(y - turtle.y)
    if not flag then
        return false, err
    end
    flag, err = travelX(x - turtle.x)
    if not flag then
        return false, err
    end
    return true, nil
end

local dir = 1

function dig_slice(width, height)

    local nx = -math.floor((width - 1) / 2)
    local px = math.floor(width / 2)
    local dx = width - 1
    local y = height - 1

    local flag
    local err

    flag, err = go_to(dir > 0 and nx or px, y, turtle.z)
    if not flag then
        return false, err
    end

    while turtle.y > 0 do
        flag, err = travelX(dx * dir)
        if not flag then
            return false, err
        end
        flag, err = travelY(-1)
        if not flag then
            return false, err
        end
        dir = dir * -1
    end

    flag, err = travelX(dx * dir)
    if not flag then
        return false, err
    end
    flag, err = travelY(y)
    if not flag then
        return false, err
    end

    dir = dir * -1

    return true, nil

end

function dig_room(width, height, depth)
    local flag
    local err
    for d = 0,depth-1 do
        flag, err = travelZ(1)
        if not flag then
            return false, err
        end
        flag, err = dig_slice(width, height)
        if not flag then
            return false, err
        end
    end
    return true, nil
end

turtle.x = 0
turtle.y = 0
turtle.z = 0
turtle.facing = POS_Z

local arg = {...}

if #arg ~= 3 then
    print("Expected 3 Lengths")
    return
end

local x = tonumber(arg[1])
local y = tonumber(arg[2])
local z = tonumber(arg[3])

local volume = x * y * z + z + y * z

local fuel = turtle.getFuelLevel()

if volume > fuel then
    print("Not enough fuel to complete operation, need " .. volume .. " total fuel")
    return
end

if x == nil or y == nil or z == nil then
    print("Dimensions must be numbers")
    return
end

local flag
local err
flag, err = dig_room(x, y, z)

if not flag then
    print(err)
end

travelZ(-turtle.z + 1)
travelY(-turtle.y)
travelX(-turtle.x)
travelZ(-1)

face(POS_Z)
