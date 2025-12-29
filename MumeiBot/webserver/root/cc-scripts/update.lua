local arg = {...}

if #arg == 1 then
    local prog = arg[1]
    shell.run("rm " .. prog)
    shell.run("wget https://3.141592.dev/cc-scripts/" .. prog)
end
