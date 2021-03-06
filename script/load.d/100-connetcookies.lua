--[[

  Protect against connect flood attacks with spoofed source ip.

]]--

spaghetti.addhook("hostcreate", function(info)
  info.skip = true
  engine.serverhost = engine.enet_host_create(info.address, engine.ENET_PROTOCOL_MAXIMUM_PEER_ID, 3, 0, cs.serveruprate);
  if not engine.serverhost then return end
  engine.serverhost.duplicatePeers = cs.maxdupclients ~= 0 and cs.maxdupclients or 4
  if engine.enet_host_connect_cookies(engine.serverhost, 500, 0) ~= 0 then return end
  if engine.enet_socket_set_option(engine.serverhost.socket, engine.ENET_SOCKOPT_RCVBUF, 5000000) == -1 then
    engine.writelog("Cannot set receive buffer size")
  end
  if engine.enet_socket_set_option(engine.serverhost.socket, engine.ENET_SOCKOPT_SNDBUF, 5000000) == -1 then
    engine.writelog("Cannot set send buffer size")
  end
  spaghetti.later(1000, function()
    local connrate = engine.serverhost.connectsInWindow * 2
    if connrate < 100 then return end
    engine.writelog("Connection flood " .. connrate .. "/s \t(window size " .. engine.serverhost.connectsWindow .. " " .. math.modf(engine.serverhost.connectsInWindow / engine.serverhost.connectsWindow * 100) .. "% used)")
  end, true)
end)

