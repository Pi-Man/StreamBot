local args = {...}

local USAGE =                                        --
"download/upload [args]\n\n" ..
"download <file>    download the latest version of\n" .. 
"                   <file> to /bin/\n\n" ..

"upload <file> <version> <username> <password>\n" .. 
"                   upload /bin/<file> to repository\n" .. 
"                   at <version>\n" .. 
"                   If you do not have credentials, \n" ..
"                   visit https://3.141592.dev/cc-scripts\n\n"

if not fs.exists("/bin/base64.lua") then
    shell.run("wget https://3.141592.dev/cc-scripts/base64.lua /bin/base64.lua")
end

local base64 = require("base64")

if args[1] then
    if args[1] == "download" then
        if #args == 2 then
            fs.delete("/bin/" .. args[2])
            shell.run("wget https://3.141592.dev/cc-scripts/" .. args[2] .. " /bin/" .. args[2])
            return
        else
            print(USAGE)
            return
        end
    elseif args[1] == "upload" then
        if #args == 5 then
            local file = fs.open("/bin/" .. args[2], "r")
            local script = file.readAll()
            file.close()
            local authPlain = args[4] .. ":" .. args[5]
            local aughB64 = base64.enc(authPlain)
            local response = http.post("https://3.141592.dev/cc-scripts/" .. args[2] .. "?version=" .. args[3], script, { ["Authorization"] = "Basic " .. authB64})
            local code, res = response.getResponseCode()
            print("Code: " .. code .. "\nResponse: " .. res)
            return
        else
            print(USAGE)
            return
        end
    else
        print(USAGE)
        return
    end
else
    print(USAGE)
    return
end