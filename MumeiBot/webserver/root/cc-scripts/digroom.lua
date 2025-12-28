POS_Z = 0
POS_X = 1
NEG_Z = 2
NEG_X = 3

function face(dir)
    print("turning")
    print(dir)
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
    turtle.facing = dir;
    print("turned")
end

function move()
    local block
    local info
    block, info = turtle.inspect()
    local flag
    local err
    if not block or info.name == "minecraft:air" then
        flag, err = turtle.forward()
    else
        flag, err = turtle.dig()
        if flag then flag, err = turtle.forward() end
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
    local block
    local info
    block, info = turtle.inspectUp()
    local flag
    local err
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
    local block
    local info
    block, info = turtle.inspectDown()
    local flag
    local err
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
    print("travelX")
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
    print("travelY")
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
    print("travelZ")
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

function dig_slice(width, height)
    local flag
    local err
    flag, err = travelX(-math.floor((width - 1) / 2))
    if not flag then
        return false, err
    end
    local x = math.floor(width / 2)
    if height > 1 then
        flag, err = travelY(1)
        if not flag then
            return false, err
        end
        local h = height - 2
        local dir = 1
        while turtle.x < x do
            flag, err = travelY(h * dir)
            if not flag then
                return false, err
            end
            flag, err = travelX(1)
            if not flag then
                return false, err
            end
            dir = dir * -1
        end
        flag, err = travleY(h * dir)
        if not flag then
            return false, err
        end
    end
    flag, err = travelY(-turtle.y)
    if not flag then
        return false, err
    end
    flag, err = travelX(-turtle.x)
    if not flag then
        return false, err
    end
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

