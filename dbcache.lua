local M = {}
M.memdbname = nil
M.session = nil

local c = require "dbcache.core"

function M.init(memdbname, ...)
	M.session = c.init(memdbname, ...)
	M.memdbname = memdbname
end

function M.begin()
	c.begin(M.session)
end

function M.commit()
	c.commit(M.session)
end

function M.rollback()
	c.rollback(M.session)
end

function M.close()
	if M.memdbname then
		c.close(M.memdbname, M.session)
	end
end

function M.test()
	c.test(M.session)
end

return setmetatable(M, { __gc = function(self)
	M.close()
end }
)
