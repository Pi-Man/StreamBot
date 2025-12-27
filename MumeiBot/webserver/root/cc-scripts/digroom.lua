POS_Z = 0
POS_X = 1
NEG_Z = 2
NEG_X = 3

function face(dir)
    local ang = turtle.facing - dir
    while ang >= 2 do ang = ang - 4 end
    while ang < -2 do ang = ang + 4 end
    while ang < 0 do
        turtle.rotateRight()
        ang = ang + 1
    end
    while ang > 0 do
        turtle.rotateLeft()
        ang = ang - 1
    end
end

function move()
    local info = turtle.inspect()
    local flag
    local err
    if info.name == "minecraft:air" then
        flag, err = turtle.forward()
    else
        flag, err = turtle.dig()
        if flag then flag, err = turtle.forward() end
    end
    if not flag then
        print(err)
        return false, err
    end
    return true
end

function moven(length)
    while length > 0 do
        local flag
        local err
        flag, err = move()
        if not flag then
            print(err)
            return false
        end
    end
    return true
end

function travelX(length)
    if length < 0 then
        face(NEG_X)
    else
        fase(POS_X)
    end
    length = math.abs(length)
    local flag
    local err
    flag, err = moven(length)
    if not flag then
        print(err)
        return false
    end
    return true
end

function travelY(length)
    if length < 0 then
        face(NEG_Y)
    else
        fase(POS_Y)
    end
    length = math.abs(length)
    local flag
    local err
    flag, err = moven(length)
    if not flag then
        print(err)
        return false
    end
    return true
end

function travelZ(length)
    if length < 0 then
        face(NEG_Z)
    else
        fase(POS_Z)
    end
    length = math.abs(length)
    local flag
    local err
    flag, err = moven(length)
    if not flag then
        print(err)
        return false
    end
    return true
end

function dig_slice(width, height)

end

function dig_room(width, height, depth)
    for d = 0,depth-1 do
        travelZ(1)
        if not dig_slice(width, height) then
            return false
        end
    end
    return true
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

dig_room(x, y, z)

