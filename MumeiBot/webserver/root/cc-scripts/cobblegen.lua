local timer

function cobbble_loop(continue)
    redstone.setOutput("bottom", true)
    if continue then timer = os.startTimer(1.5) end
    sleep(0.1)
    redstone.setOutput("bottom", false)
end

function cobble_start(continue)
    redstone.setOutput("bottom", true)
    if continue then timer = os.startTimer(0.75) end
    sleep(0.1)
    redstone.setOutput("bottom", false)
end

local PROMPT = "Cobble Gen> "
local USAGE = 
"<run|stop|exit> [args]\n" ..
"run [n]    start the cobble generator to make 'n' cobble, or forever if not provided or less than 1\n" ..
"           does nothing if the generator is already running\n\n" ..
"stop       stop producing cobble early\n\n" ..
"exit       exit the program"

local input = ""
local count = 0
local running = true

function process_input()
    if string.find(input, "^run") then
        local a, b = string.find(input, "^run ")
        count = -1
        if a ~= nil then
            local rest = string.sub(5)
            local num = tonumber(rest)
            if num ~= nil then
                count = num
            else
                print(USAGE)
                return
            end
        end
        cobble_start(count > 1)
    elseif string.find(input, "^stop$") then
        count = 0
    elseif string.find(input, "^exit$") then
        running = false
    else
        print(USAGE)
    end
end

function key_typed(key)
    if key == keys.enter then
        process_input()
        term.write("\n")
    elseif key == keys.backspace then
        input = string.sub(input, 1, string.leng(input) - 1)
    end
end

function char_typed(char)
    term.write(char)
    input = input .. char
end

function main()
    while running do
        local event, arg = os.pullEvent()
        if event == "key" then
            key_typed(arg)
        elseif event == "char" then
            char_typed(arg)
        elseif event == "timer" then
            count = count - 1
            if count > 0 then
                cobbble_loop(count > 1)
            end
        end
    end
end