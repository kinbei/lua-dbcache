## 关于lua-dbcache

lua-dbcache 是一个DB缓存的模块，使用FastDB作为数据的缓存，Mysql作为数据的存储  
执行insert()/update()/remove()操作后，会生成相应的SQL语句放到全局队列  
外部模块需要调用 dbcache.tickcount() 接口，使生成的SQL语句提交到Mysql数据库  
内存表结构主要在 dbfunc.h dbfunc.cpp 两个文件，可以很容易通过工具生成代码(之后会考虑提供 dbgen 工具生成 dbfunc.h dbfunc.cpp 两个文件的代码)  

## 如何编译

```
git clone https://github.com/kinbei/lua-dbcache.git
cd lua-dbcache
make
```

## 运行测试用例

注: 运行前先将 testdb.sql 导入 mysql, 并修改 `config.lua` 对应的参数

```
lua test.lua
```

## 如何使用

参考 test.lua 
