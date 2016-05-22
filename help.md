# 皈依管理系统使用说明

> 此软件用于设置皈依信息，需要使用MYSQL支持，所以初次安装需要有MYSQL环境，并提供一个有所有权限的用户以及密码。下面简单介绍如何使用。

## 安装

##### 1. 首先安装MYSQL-SERVER这个软件

建议使用*GNU/Linux/UNIX* 环境，并且可以提供Rsync 服务，这样比较方便，当然使用*Windows* 也是可以的，只不过部署稍微麻烦一点。无非是安装个*MYSQL SERVER for Windows*然后再安装一个*Rsyncd*的*Windows*衍生版。如果是*GNU/Linux*那就很简单了。建议使用*Ubuntu*或者*Debian*系列的系统，使用比较简单。

```
apt-get install mysql-server # 安装数据库服务器
```
##### 2. 设置Rsyncd服务器
最好查阅下如何使用*rsync* 服务器

```
pid file = /var/run/rsyncd.pid
log file = /var/log/rsyncd.log


[citta]
path= /Users/quqinglei/RsyncShare
hosts allow = 192.168.1.0/24
uid = quqinglei
gid = admin
list = yes
read only = false
comment = citta images
```

```
rsync --daemon # 可以用来启动 RSYNCD 它需要root才能启动。可以让它默认启动。
```

##### 3. 然后就可以使用本软件初始化了
在初始化标签栏，输入数据库的IP地址，用户名，密码即可完成数据库的初始化操作，注意请用数据库的root用户。

## 使用介绍

##### 1. 

