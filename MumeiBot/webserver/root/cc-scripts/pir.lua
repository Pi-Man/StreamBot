local args = {...}

local USAGE =                                        --
"download/upload [args]\n\n" ..

"download <file>\n" .. 
"       download the latest version of <file> to\n" .. 
"       /bin/\n\n" ..

"upload <file> <version> <username> <password>\n" .. 
"       upload /bin/<file> to repository\n" .. 
"       at <version>\n" .. 
"       If you do not have credentials, \n" ..
"       visit https://3.141592.dev/cc-scripts\n\n" ..

"account create <username> <password>\n" .. 
"       Create a new account for uploading scripts\n\n"

if not fs.exists("/bin/base64.lua") then
    shell.run("wget https://3.141592.dev/cc-scripts/base64.lua /bin/base64.lua")
end

local base64 = require("base64")

local hex = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" }

function percent_encode(string)
    local out
    for i = 1,string.len(string) do
        local byte = string.byte(string, i)
        if byte < 128 then
            out = out .. string.char(byte)
        else
            local lsn = byte % 16
            local msn = byte / 16
            out = out .. "%" .. hex[msn] .. hex[lsn]
        end
    end
    return out
end

if args[1] then
    if args[1] == "download" then
        if #args == 2 then
            fs.delete("/bin/" .. args[2])
            shell.run("wget https://3.141592.dev/cc-scripts/" .. args[2] .. " /bin/" .. args[2])
            return
        end
    elseif args[1] == "upload" then
        if #args == 5 then
            local file = fs.open("/bin/" .. args[2], "r")
            local script = file.readAll()
            file.close()
            local authPlain = args[4] .. ":" .. args[5]
            local authB64 = base64.enc(authPlain)
            local response = http.post("https://3.141592.dev/cc-scripts/" .. args[2] .. "?version=" .. args[3], script, { ["Authorization"] = "Basic " .. authB64})
            local code, res = response.getResponseCode()
            print("Code: " .. code .. "\nResponse: " .. res)
            return
        end
    elseif args[1] == "account" then
        if args[2] == "create" and #args == 4 then
            local body = "user=" .. percent_encode(args[3]) .. "&pass=" .. percent_encode(args[4])
            local response = http.post("https://3.141592.dev/cc-scripts/create_account", body)
            local code, res = response.getResponseCode()
            if code >= 200 and code < 300 then
                print("Account creation successful")
            else
                print("Username already taken")
            end
            return
        end
    end
end

print(USAGE)