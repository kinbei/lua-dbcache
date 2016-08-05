local dbcache = require "dbcache"
local config = require "config"

-- for test loadtable
local origin_record = {
	{ activity_id = 200, activity_name = 'test mysql sync' },
	{ activity_id = 819, activity_name = 'test activity 1' },
	{ activity_id = 824, activity_name = 'test activity 2' },
	{ activity_id = 832, activity_name = 'test activity 3' },
	{ activity_id = 834, activity_name = 'test activity 4' },
	{ activity_id = 887, activity_name = 'test activity 5' },
	{ activity_id = 888, activity_name = 'test activity 6' },
	{ activity_id = 889, activity_name = '中文测试' },
}

local function dbopt()
	-- insert testcase
	local tb_activity = dbcache.gettable("tb_activity")
	assert(tb_activity, "tb_activity is nil")

	-- test loadtable
	tb_activity.reset()
	tb_activity.prepare("");
	assert( tb_activity.find() == 8 )
	local idx = 1
	while tb_activity.next() do
		assert( origin_record[idx].activity_id == tb_activity.getactivity_id() )
		assert( origin_record[idx].activity_name == tb_activity.getactivity_name() )
		idx = idx + 1
	end

	tb_activity.reset()
	tb_activity.setactivity_id(100)
	tb_activity.setactivity_name("test_act_name")
	tb_activity.insert()

	-- select testcase
	tb_activity.reset()
	tb_activity.prepare("activity_id = %activity_id and activity_name = %activity_name")
	tb_activity.findsetactivity_id(100)
	tb_activity.findsetactivity_name("test_act_name")
	local row = tb_activity.find()
	assert(row == 1, string.format("row = %s", row))
	while tb_activity.next() do
		assert( tb_activity.getactivity_id() == 100 )
		assert( tb_activity.getactivity_name() == "test_act_name" )

		tb_activity.setactivity_name("test_act_name_1")
		tb_activity.update()
	end

	-- update testcase
	tb_activity.reset()
	tb_activity.prepare("activity_id = %activity_id and activity_name = %activity_name")
	tb_activity.findsetactivity_id(100)
	tb_activity.findsetactivity_name("test_act_name_1")
	local row = tb_activity.find()
	assert(row == 1)
	while tb_activity.next() do
		assert( tb_activity.getactivity_id() == 100 )
		assert( tb_activity.getactivity_name() == "test_act_name_1" )
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
	assert( tb_activity.find() == 8 )

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
	tb_activity.insert()
	
	-- test key field
	tb_activity.reset()
	tb_activity.setactivity_id(100)
	tb_activity.setactivity_name("test_act_name")
	tb_activity.insert()

	local success, err = pcall(function()
		tb_activity.reset()
		tb_activity.setactivity_id(100)
		tb_activity.setactivity_name("test_act_name")
		tb_activity.insert()
	end)
	assert(success == false)
	assert(err == "test.lua:101: insert error[tb_activity primary key(100) already exists]");
	tb_activity.prepare("activity_id = 100")
	assert( tb_activity.find() == 1 )
	while tb_activity.next() do
		tb_activity.remove()
	end

	--
	dbcache.freestatement()
end

dbcache.opendb(config.memdb_name, config.mysql_host, config.mysql_user, config.mysql_pwd, config.mysql_dbname, config.mysql_port)
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