i = 0
while true do
    i = i + 1
    puts i
    sleep 1
    break if Blink.req_reload?
end