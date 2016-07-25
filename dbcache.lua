local M = {}
M.memdbname = nil

local c = require "dbcache.core"

function M.init(memdbname, ...)
	c.init(memdbname, ...)
	M.memdbname = memdbname
end

return setmetatable(M, { __gc = function(self)
	if M.memdbname then
		c.cleanupsem(M.memdbname)
	end
end }
)
