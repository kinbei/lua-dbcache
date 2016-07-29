## 关于lua-dbcache

lua-dbcache 是一个DB缓存的模块，采用FastDB作为内存数据库，一般情况下的布署方案是，数据先通过 lua-dbcache 更新到fastdb再定时批量事务更新到mysql，内存表结构主要在 dbfunc.h dbfunc.cpp 两个文件，可以很容易通过工具生成代码

## 如何编译

```
git clone https://github.com/kinbei/lua-dbcache.git
cd lua-dbcache
git submodule update --init --recursive
make
```

## 运行测试用例

```
lua test.lua
```

## 如何使用

参考 test.lua 
