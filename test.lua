local dbcache = require "dbcache"
dbcache.init "test"
dbcache.begin()
dbcache.rollback()
dbcache.commit()

io.read()