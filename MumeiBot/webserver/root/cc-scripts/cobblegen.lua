local timer

function cobble_loop(continue)
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
"<run|stop|exit> [args]\n\n" ..
"run [n]    start the cobble generator to make 'n'\n" ..
"           cobble, or forever if not provided or\n" ..
"           less than 0\n" ..
"           does nothing if the generator is already" ..
"           running\n\n" ..
"stop       stop producing cobble early\n\n" ..
"exit       exit the program"

local input = ""
local count = 0
local running = true

function process_input()
    if string.find(input, "^run$") then
        count = -1
        cobble_start(true)
    elseif string.find(input, "^run %d+$") then
        local rest = string.sub(input, 5)
        local num = tonumber(rest)
        count = num
        cobble_start(count > 1)
    elseif string.find(input, "^stop$") then
        count = 1
    elseif string.find(input, "^exit$") then
        running = false
    else
        print(USAGE)
    end
    input = ""
end

function key_typed(key)
    if key == keys.enter then
        process_input()
        print("")
        if running then
            term.write(PROMPT)
        end
    elseif key == keys.backspace then
        input = string.sub(input, 1, string.leng(input) - 1)
    end
end

function char_typed(char)
    term.write(char)
    input = input .. char
end

function main()
    term.setCursorBlink(true)
    term.write(PROMPT)
    while running do
        local event, arg = os.pullEvent()
        if event == "key" then
            key_typed(arg)
        elseif event == "char" then
            char_typed(arg)
        elseif event == "timer" and timer == arg then
            if count > 0 then
                count = count - 1
                cobble_loop(count > 1)
            elseif count < 0 then
                cobble_loop(true)
            end
        end
    end
end

main()