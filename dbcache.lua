local M = {}
M.memdbname = nil
M.session = nil

local c = require "dbcache.core"

function M.opendb(memdbname, ...)
	M.session = c.opendb(memdbname, ...)
	M.memdbname = memdbname

	c.loadtable()
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

function M.closedb()
	if M.memdbname then
		c.closedb(M.memdbname, M.session)
		M.memdbname = nil
	end
end

function M.tickcount()
	c.tickcount()
end

function M.gettable(tablename)
	return c.gettable(tablename)
end

return setmetatable(M, { __gc = function(self)
	M.closedb()
end }
)
