local dbcache = require "dbcache"
dbcache.init "test"
dbcache.begin()
dbcache.test()
dbcache.rollback()
dbcache.commit()
