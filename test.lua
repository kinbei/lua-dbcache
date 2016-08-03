local dbcache = require "dbcache"

local function dbopt()
	-- insert testcase
	local tb_activity = dbcache.gettable("tb_activity")
	assert(tb_activity, "tb_activity is nil")

	tb_activity.reset()
	tb_activity.setactivity_id(100)
	tb_activity.setactivity_name("test_act_name")
	tb_activity.setstatus(1)
	tb_activity.insert()

	-- select testcase
	tb_activity.reset()
	tb_activity.prepare("activity_id = %activity_id and status = %status")
	tb_activity.findsetactivity_id(100)
	tb_activity.findsetstatus(1)
	local row = tb_activity.find()
	assert(row == 1, string.format("row = %s", row))
	while tb_activity.next() do
		assert( tb_activity.getactivity_id() == 100 )
		assert( tb_activity.getactivity_name() == "test_act_name" )
		assert( tb_activity.getstatus() == 1 )

		tb_activity.setactivity_name("test_act_name_1")
		tb_activity.update()
	end

	-- update testcase
	tb_activity.reset()
	tb_activity.prepare("activity_id = %activity_id and status = %status")
	tb_activity.findsetactivity_id(100)
	tb_activity.findsetstatus(1)
	local row = tb_activity.find()
	assert(row == 1)
	while tb_activity.next() do
		assert( tb_activity.getactivity_id() == 100 )
		assert( tb_activity.getactivity_name() == "test_act_name_1" )
		assert( tb_activity.getstatus() == 1 )
	end

	-- remove testcase
	tb_activity.reset()
	tb_activity.prepare("")
	tb_activity.find()
	while tb_activity.next() do
		if tb_activity.getactivity_id() == 100 then
			tb_activity.remove()
		end
	end
	tb_activity.reset()
	tb_activity.prepare("")
	assert( tb_activity.find() == 7 )

	-- test mysql sync
	tb_activity.reset()
	tb_activity.prepare("activity_id = %activity_id")
	tb_activity.findsetactivity_id(200)
	tb_activity.find()
	while tb_activity.next() do
		tb_activity.remove()
	end

	tb_activity.reset()
	tb_activity.setactivity_id(200)
	tb_activity.setactivity_name("test mysql sync")
	tb_activity.setstatus(1)
	tb_activity.insert()
end

dbcache.opendb("test", "192.168.8.196", "root", "hQK_DWbBuzl", "testdb", 4400)
dbcache.begin()
local ok, err = xpcall(dbopt, debug.traceback)
if not ok then
	print(err)
	dbcache.rollback()
else
	dbcache.commit()
end

dbcache.tickcount()
print("Testcase finish!")