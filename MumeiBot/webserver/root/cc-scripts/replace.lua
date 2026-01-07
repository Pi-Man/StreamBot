POS_Z = 0
POS_X = 1
NEG_Z = 2
NEG_X = 3

POS_Y = 4
NEG_Y = 5

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
    turtle.facing = dir;
end

function move()
    local flag
    local err
    flag, err = turtle.forward()
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
    flag, err = turtle.up()
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
    flag, err = turtle.down()
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

turtle.x = 0
turtle.y = 0
turtle.z = 0
turtle.facing = POS_Z

function replace(maj, min, majF, minF, repF)

    local dir = 1
    local dx = maj - 1
    local dy = min - 1

    local flag
    local err

    for _ = 1,dy do
        for _ = 1,dx do
            flag, err = repF()
            if not flag then
                return false, err
            end
            flag, err = majF(dir)
            if not flag then
                return false, err
            end
        end
        flag, err = repF()
        if not flag then
            return flag, err
        end
        flag, err = minF(1)
        if not flag then
            return false, err
        end
        dir = dir * -1
    end

    for _ = 1,dx do
        flag, err = repF()
        if not flag then
            return false, err
        end
        flag, err = majF(dir)
        if not flag then
            return false, err
        end
    end
    flag, err = repF()
    if not flag then
        return false, err
    end

    return true, nil

end

local active_slot = 9

function check_inv(movF, dir)
    if turtle.getItemCount(active_slot) == 0 then
        active_slot = active_slot + 1
    end
    local full = turtle.getItemCount(active_slot - 2) ~= 0
    if full or turtle.getItemCount(16) == 0 then
        local flag, err

        local x = turtle.x
        local y = turtle.y
        local z = turtle.z

        flag, err = go_to(0, 0, 0)
        if not flag then
            return false, err
        end

        local s = 1
        while s <= 16 and turtle.getItemCount(s) > 0 do
            turtle.select(s)
            if dir == POS_Y then
                turtle.dropUp()
            elseif dir == NEG_Y then
                turtle.dropDown()
            else
                face(dir)
                turtle.drop()
            end
            s = s + 1
        end

        movF(1)

        local flag2 = true
        local flag3 = false
        s = 16
        while s >= 9 and flag2 do
            turtle.select(s)
            if dir == POS_Y then
                flag2 = turtle.suckUp()
            elseif dir == NEG_Y then
                flag2 = turtle.suckDown()
            else
                face(dir)
                flag2 = turtle.suck()
            end
            flag3 = flag3 or flag2
            s = s - 1
        end

        if not flag3 then
            return false, "out of items"
        end

        flag, err = go_to(x, y, z)
        if not flag then
            return false, err
        end

        active_slot = 9
    end
    return true, nil
end

function replace_topF()
    local flag, err
    flag, err = check_inv(travelX, NEG_Y)
    if not flag then
        return false, err
    end
    turtle.select(1)
    while turtle.inspectUp() do
        flag, err = turtle.digUp()
        if not flag then
            return false, err
        end
    end
    turtle.select(active_slot)
    turtle.placeUp()
    return true, nil
end

function replace_bottomF()
    local flag, err
    flag, err = check_inv(travelX, POS_Y)
    if not flag then
        return false, err
    end
    turtle.select(1)
    if turtle.inspectDown() then
        flag, err = turtle.digDown()
        if not flag then
            return false, err
        end
    end
    turtle.select(active_slot)
    turtle.placeDown()
    return true, nil
end

function replace_wallF()
    local flag, err
    flag, err = check_inv(travelY, NEG_Z)
    if not flag then
        return false, err
    end
    face(POS_Z)
    turtle.select(1)
    while turtle.inspect() do
        flag, err = turtle.dig()
        if not flag then
            return false, err
        end
    end
    turtle.select(active_slot)
    turtle.place()
    return true, nil
end

function replace_top(width, height)
    local flag
    local err
    travelY(128)
    flag, err = replace(width, height, travelX, travelZ, replace_topF)
    return flag, err
end

function replace_bottom(width, height)
    local flag
    local err
    flag, err = replace(width, height, travelX, travelZ, replace_bottomF)
    return flag, err
end

function replace_wall(width, height)
    local flag
    local err
    flag, err = replace(height, width, travelY, travelX, replace_wallF)
    return flag, err
end

local args = {...}

if #args == 3 then
    local width, height
    width = tonumber(args[2])
    height = tonumber(args[3])
    if width and height then
        if args[1] == "ceiling" then
            replace_top(width, height)
            return
        elseif args[1] == "wall" then
            replace_wall(width, height)
            return
        elseif args[1] == "floor" then
            replace_bottom(width, height)
            return
        end
    end
end

print(
"replace ceiling/wall/floor <width> <height>"
)
